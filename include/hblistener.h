/**
 * @file
 * @brief Defines Heartbeat Listener interfaces
 * @details This file defines interfaces for the Heartbeat Listener class.  It listens for
 * heartbeats from designated senders - allowing them to be added and dropped at run time.
 *
 *
 * @author &copy; 2011 - Alan Robertson <alanr@unix.sh>
 * @n
 * Licensed under the GNU Lesser General Public License (LGPL) version 3 or any later version at your option.
 * excluding the provision allowing for relicensing under the GPL at your option.
 */

#ifndef _HBLISTENER_H
#define _HBLISTENER_H
#include <projectcommon.h>
#include <netaddr.h>
#include <netgsource.h>
#include <listener.h>
typedef struct _HbListener HbListener;

typedef enum {
	HbPacketsBeingReceived = 1,
	HbPacketsTimedOut = 2,
}HbNodeStatus;

///@{
/// @ingroup HbListener

/// This is the base @ref HbListener object - which listens for heartbeats from a
/// particular sender.
struct _HbListener {
	Listener	baseclass;
	guint64		(*get_deadtime)(HbListener*);	///< Retrieve deadtime
	void		(*set_deadtime)(HbListener*, guint64);	///< Set deadtime
	guint64		(*get_warntime)(HbListener*);	///< Retrieve warntime
	void		(*set_warntime)(HbListener*, guint64);	///< Set warntime
	void		(*set_heartbeat_callback)(HbListener*, void (*)(HbListener* who));
	void		(*set_deadtime_callback)(HbListener*, void (*)(HbListener* who));
	void		(*set_warntime_callback)(HbListener*, void (*)(HbListener* who, guint64 howlate));
	void		(*set_comealive_callback)(HbListener*, void (*)(HbListener* who, guint64 howlate));
	guint64		_expected_interval;		///< How often to expect heartbeats
	guint64		_warn_interval;			///< When to warn about late heartbeats
	guint64		nexttime;			///< When next heartbeat is due
	guint64		warntime;			///< Warn heartbeat time
	int		_refcount;			///< Current reference count
	NetAddr*	listenaddr;			///< What address are we listening for?
	HbNodeStatus	status;				///< What status is this node in?
	void		(*_heartbeat_callback)(HbListener* who);
	void		(*_deadtime_callback)(HbListener* who);
	void		(*_warntime_callback)(HbListener* who, guint64 howlate);
	void		(*_comealive_callback)(HbListener* who, guint64 howlate);
};
#define	DEFAULT_DEADTIME	60 // seconds

WINEXPORT HbListener*	hblistener_new(NetAddr*, ConfigContext* config, gsize hblisten_objsize);
WINEXPORT void 		hblistener_unlisten(NetAddr* unlistenaddr);
WINEXPORT void		hblistener_set_martian_callback(void (*)(const NetAddr* who));
WINEXPORT HbListener*	hblistener_find_by_address(const NetAddr* which);

///@}

#endif /* _HBLISTENER_H */
