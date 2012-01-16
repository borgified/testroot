/**
 * @file
 * @brief Implements NetIO GSource (NetGSource) object 
 * @details This file contains source code implementing NetGSource objects.
 * NetGSource objects construct GSource objects from NetIO objects, so that
 * input from NetIO objects can be incorporated into the Glib g_main_loop event-driven
 * programming paradigm as a GSource derived class. As a result, functions can
 * be dispatched when NetIO packets arrive.
 *
 *
 * @author &copy; 2011 - Alan Robertson <alanr@unix.sh>
 * @n
 * Licensed under the GNU Lesser General Public License (LGPL) version 3 or any later version at your option,
 * excluding the provision allowing for relicensing under the GPL at your option.
 */

#include <projectcommon.h>
#include <memory.h>
#include <glib.h>
#include <frameset.h>
#include <netgsource.h>

/// @defgroup NetGSource NetGSource class
///@{
/// @ingroup C_Classes
/// This is our basic NetGSource object.
/// It is used for reading from @ref NetIO objects in the context of the g_main_loop GSource paradigm.
/// It is a class from which we might eventually make subclasses,
/// and is managed by our @ref ProjectClass system.

FSTATIC gboolean _netgsource_prepare(GSource* source, gint* timeout);
FSTATIC gboolean _netgsource_check(GSource* source);
FSTATIC gboolean _netgsource_dispatch(GSource* source, GSourceFunc callback, gpointer user_data);
FSTATIC void     _netgsource_finalize(GSource* source);
FSTATIC void	_netgsource_addListener(NetGSource*, guint16, Listener*);
FSTATIC void	_netgsource_del_listener(gpointer);

static GSourceFuncs _netgsource_gsourcefuncs = {
	_netgsource_prepare,
	_netgsource_check,
	_netgsource_dispatch,
	_netgsource_finalize,
	NULL,
	NULL
};
FSTATIC void
_netgsource_del_listener(gpointer lptr)
{
	Listener*	lobj = CASTTOCLASS(Listener, lptr);
	lobj->unref(lobj);
}

/// Create a new (abstract) NetGSource object
NetGSource*
netgsource_new(NetIO* iosrc,			///<[in/out] Network I/O object
	       GDestroyNotify notify,		///<[in] Called when object destroyed
	       gint priority,			///<[in] g_main_loop
						///< <a href="http://library.gnome.org/devel/glib/unstable/glib-The-Main-Event-Loop.html#G-PRIORITY-HIGH:CAPS">dispatch priority</a>
	       gboolean can_recurse,		///<[in] TRUE if it can recurse
	       GMainContext* context,		///<[in] GMainContext or NULL
	       gsize objsize,			///<[in] number of bytes in NetGSource object - or zero
	       gpointer userdata		///<[in/out] Userdata to pass to dispatch function
	       )
{
	GSource*	gsret;
	NetGSource*	ret;
	GSourceFuncs*	gsf;

	if (objsize < sizeof(NetGSource)) {
		objsize = sizeof(NetGSource);
	}
	gsf  = MALLOCBASECLASS(GSourceFuncs);
	*gsf = _netgsource_gsourcefuncs;
	
	gsret = g_source_new(gsf, objsize);
	if (gsret == NULL) {
		FREECLASSOBJ(gsf);
		g_return_val_if_reached(NULL);
	}
	proj_class_register_object(gsret, "GSource");
	proj_class_register_subclassed(gsret, "NetGSource");
	ret = CASTTOCLASS(NetGSource, gsret);

	ret->_gsfuncs = gsf;
	ret->_userdata = userdata;
	ret->_netio = iosrc;
	ret->_socket = iosrc->getfd(iosrc);
	ret->_finalize = notify;
	ret->_gfd.fd = ret->_socket;
	ret->_gfd.events = G_IO_IN|G_IO_ERR|G_IO_HUP;
	ret->_gfd.revents = 0;
	ret->addListener = _netgsource_addListener;

	g_source_add_poll(gsret, &ret->_gfd);
	g_source_set_priority(gsret, priority);
	g_source_set_can_recurse(gsret, can_recurse);

	ret->_gsourceid = g_source_attach(gsret, context);

	if (ret->_gsourceid == 0) {
		FREECLASSOBJ(gsf);
		g_source_remove_poll(gsret, &ret->_gfd);
		memset(ret, 0, sizeof(*ret));
		g_source_unref(gsret);
		gsret = NULL;
		ret = NULL;
		g_return_val_if_reached(NULL);
	}
	ret->_dispatchers = g_hash_table_new_full(NULL, NULL, NULL, _netgsource_del_listener);
	return ret;
}

/// GSource prepare routine for NetGSource - always returns TRUE
/// Called before going into the select/poll call - to get things ready for the poll call.
FSTATIC gboolean
_netgsource_prepare(GSource* source,	///<[unused] - GSource object
		    gint* timeout)	///<[unused] - timeout
{
	(void)source; (void)timeout;
	return FALSE;
}

/// GSource check routine for NetGSource.
/// Called after the select/poll call completes.
/// @return TRUE if if a packet is present, FALSE otherwise
FSTATIC gboolean
_netgsource_check(GSource* gself)	///<[in] NetGSource object being 'check'ed.
{
	NetGSource*	self = CASTTOCLASS(NetGSource, gself);
	// revents: received events...
	// @todo: probably should check for errors in revents
	return 0 != self->_gfd.revents;
}
/// GSource dispatch routine for NetGSource.
/// Called after our check function returns TRUE.
/// If a bunch of events are fired at once, then this call will be dispatched before the next prepare
/// call, but perhaps not quite right away - depending on what other events (with possibly higher
/// priority) get dispatched ahead of us, and how long they take to complete.
FSTATIC gboolean
_netgsource_dispatch(GSource* gself,			///<[in/out] NetGSource object being dispatched
		     GSourceFunc ignore_callback,	///<[ignore] callback not being used
		     gpointer ignore_userdata)		///<[ignore] userdata not being used
{
	NetGSource*	self = CASTTOCLASS(NetGSource, gself);
	GSList*		gsl;
	NetAddr*	srcaddr;
	(void)ignore_callback; (void)ignore_userdata;
	if ((self->_gfd.revents & (G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL|G_IO_PRI)) == 0) {
		g_debug("Dispatched due to UNKNOWN REASON: 0x%04x", self->_gfd.revents);
	}
	while(NULL != (gsl = self->_netio->recvframesets(self->_netio, &srcaddr))) {
		for (; NULL != gsl; gsl = gsl->next) {
			Listener*	disp = NULL;
			FrameSet*		fs = CASTTOCLASS(FrameSet, gsl->data);
			disp = g_hash_table_lookup(self->_dispatchers, GUINT_TO_POINTER((size_t)fs->fstype));
			if (NULL == disp) {
				disp = CASTTOCLASS(Listener, g_hash_table_lookup(self->_dispatchers, NULL));
			}
			if (NULL != disp) {
				disp->got_frameset(disp, fs, srcaddr);
			}else{ 
				g_warning("No dispatcher for FrameSet type %d", fs->fstype);
			}
		}
		srcaddr->unref(srcaddr); srcaddr = NULL;
	}
	return TRUE;
}

/// Finalize (free) the NetGSource object
FSTATIC void
_netgsource_finalize(GSource* gself)	///<[in/out] object being finalized
{
#ifndef __FUNCTION__
#	define __FUNCTION__ "_netgsource_finalize"
#endif
	NetGSource*	self = CASTTOCLASS(NetGSource, gself);
	g_message("***********IN %s()", __FUNCTION__);
	if (self->_finalize) {
		self->_finalize(self->_userdata);
	}else{
		if (self->_userdata) {
			/// If this next call crashes, you should have supplied your own
			/// finalize routine (and maybe you should anyway)
			FREECLASSOBJ(self->_userdata);
			self->_userdata = NULL;
		}
	}
	if (self->_gsfuncs) {
		g_message("***********FREEING GSFUNCS %s()", __FUNCTION__);
		FREECLASSOBJ(self->_gsfuncs);
		self->_gsfuncs = NULL;
	}
	g_hash_table_unref(self->_dispatchers);
	proj_class_dissociate(gself);// Avoid dangling reference in class system
}
FSTATIC void
_netgsource_addListener(NetGSource* self,	///<[in/out] Object being modified
			guint16 fstype,		///<[in] FrameSet fstype
			Listener* disp)		///<[in] dispatch function
{
	if (disp) {
		disp->ref(disp);
	}
	g_hash_table_replace(self->_dispatchers, GUINT_TO_POINTER((size_t)fstype), disp);
}
///@}
