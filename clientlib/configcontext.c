/**
 * @file
 * @brief Implements the @ref ConfigContext class.
 * @details This file provides a place to remember and pass configuration values around.
 *
 * @author &copy; 2012 - Alan Robertson <alanr@unix.sh>
 * @n
 * Licensed under the GNU Lesser General Public License (LGPL) version 3 or any later version at your option,
 * excluding the provision allowing for relicensing under the GPL at your option.
 */
#include <configcontext.h>
#include <memory.h>

typedef struct _ConfigValue ConfigValue;
struct _ConfigValue {
	enum ConfigValType	valtype;
	gint64			intvalue;	// Or boolean
	double			floatvalue;
	gpointer		objvalue;
};

FSTATIC void	_configcontext_ref(ConfigContext* self);
FSTATIC void	_configcontext_unref(ConfigContext* self);
FSTATIC void	_configcontext_finalize(AssimObj* self);
FSTATIC enum ConfigValType	_configcontext_gettype(ConfigContext*, const char *name);
FSTATIC gint	_configcontext_getint(ConfigContext*, const char *name);
FSTATIC void	_configcontext_setint(ConfigContext*, const char *name, gint value);
FSTATIC const char* _configcontext_getstring(ConfigContext*, const char *name);
FSTATIC void	_configcontext_setstring(ConfigContext*, const char *name, const char *value);
FSTATIC NetAddr*_configcontext_getaddr(ConfigContext*, const char *);
FSTATIC void	_configcontext_setaddr(ConfigContext*, const char *name, NetAddr*);
FSTATIC Frame*	_configcontext_getframe(ConfigContext*, const char *name);
FSTATIC void	_configcontext_setframe(ConfigContext*, const char *name, Frame*);
FSTATIC char *	_configcontext_toString(gconstpointer aself);
FSTATIC char *	JSONquotestring(char * s, gboolean ismalloced);
FSTATIC GScanner* _configcontext_JSON_GScanner_new(void);
FSTATIC ConfigContext* _configcontext_JSON_parse_object(GScanner* scan);
FSTATIC ConfigContext* _configcontext_JSON_parse_members(GScanner* scan, ConfigContext* cfg);
FSTATIC ConfigContext* _configcontext_JSON_parse_pair(GScanner* scan, ConfigContext* cfg);
FSTATIC ConfigValue* _configcontext_value_new(enum ConfigValType);
FSTATIC void _configcontext_value_finalize(gpointer vself);
/// @defgroup ConfigContext ConfigContext class
/// A base class for remembering configuration values of various types.
///@{
///@ingroup C_Classes

/// Construct a new ConfigContext object - with no values defaulted
ConfigContext*
configcontext_new(gsize objsize)	///< size of ConfigContext structure (or zero for min size)
{
	AssimObj * baseobj = NULL;
	ConfigContext * newcontext = NULL;

	if (objsize < sizeof(ConfigContext)) {
		objsize = sizeof(ConfigContext);
	}
	baseobj = assimobj_new(objsize);
	if (NULL == baseobj) {
		goto errout;
	}
	newcontext = NEWSUBCLASS(ConfigContext, baseobj);
	newcontext->setint	=	_configcontext_setint;
	newcontext->getint	=	_configcontext_getint;
	newcontext->setstring=		_configcontext_setstring;
	newcontext->getstring=		_configcontext_getstring;
	newcontext->getframe=		_configcontext_getframe;
	newcontext->setframe=		_configcontext_setframe;
	newcontext->getaddr=		_configcontext_getaddr;
	newcontext->setaddr=		_configcontext_setaddr;
	newcontext->gettype=		_configcontext_gettype;
	baseobj->_finalize	=	_configcontext_finalize;
	baseobj->toString	=	_configcontext_toString;
	newcontext->_values = g_hash_table_new_full(g_str_hash, g_str_equal, g_free
	,					    _configcontext_value_finalize);
	return newcontext;
errout:
	if (baseobj) {
		baseobj->_finalize(CASTTOCLASS(AssimObj,newcontext));
		newcontext = NULL;
		baseobj = NULL;
	}
	g_return_val_if_reached(NULL);
}

FSTATIC void
_configcontext_finalize(AssimObj* aself)
{
	ConfigContext*	self = CASTTOCLASS(ConfigContext, aself);
	
	if (self->_values) {
		g_hash_table_destroy(self->_values);
		self->_values = NULL;
	}
	FREECLASSOBJ(self);
}

FSTATIC enum ConfigValType
_configcontext_gettype(ConfigContext* self, const char *name)
{
	gpointer	ret = g_hash_table_lookup(self->_values, name);
	ConfigValue*	cfg;
	if (ret == NULL) {
		return CFG_EEXIST;
	}
	cfg = CASTTOCLASS(ConfigValue, ret);
	return cfg->valtype;
}

/// Get an integer value
FSTATIC gint
_configcontext_getint(ConfigContext* self	///<[in] ConfigContext object
	,	      const char *name)		///<[in] Name to get the associated int value of
{
	gpointer	ret = g_hash_table_lookup(self->_values, name);
	ConfigValue*	cfg;

	if (ret == NULL) {
		return -1;
	}
	cfg = CASTTOCLASS(ConfigValue, ret);
	if (cfg->valtype != CFG_INT64) {
		return -1;
	}

	return cfg->intvalue;
}

/// Set a name to an integer value
FSTATIC void
_configcontext_setint(ConfigContext* self	///<[in/out] ConfigContext Object
	,	      const char *name		///<[in] Name to set the associated int value of
	,	      gint value)		///<[in] Int value to set the 'name' to
{
	ConfigValue* val = _configcontext_value_new(CFG_INT64);
	char *	cpname = g_strdup(name);

	val->intvalue = value;
	g_hash_table_replace(self->_values, cpname, val);
}

/// Return the value of a string name
FSTATIC const char*
_configcontext_getstring(ConfigContext* self	///<[in] ConfigContext object
		,	 const char *name)	///<[in] Name to get the associated string value of
{
	gpointer	ret = g_hash_table_lookup(self->_values, name);
	ConfigValue*	cfg;

	if (ret == NULL) {
		return NULL;
	}
	cfg = CASTTOCLASS(ConfigValue, ret);
	if (cfg->valtype != CFG_STRING) {
		return NULL;
	}
	return (const char *)cfg->objvalue;
}

/// Set a name to a string value
FSTATIC void
_configcontext_setstring(ConfigContext* self	///<[in/out] ConfigContext object
			,const char *name	///<[in] Name to set the string value of (we copy it)
			,const char *value)	///<[in] Value to set 'name' to (we copy it)
{
	char *	cpname = g_strdup(name);
	char *	cpvalue = g_strdup(value);
	ConfigValue* val = _configcontext_value_new(CFG_STRING);

	val->objvalue = cpvalue;
	g_hash_table_replace(self->_values, cpname, val);
}


/// Return the NetAddr value of a name
FSTATIC  NetAddr*
_configcontext_getaddr(ConfigContext* self	///<[in] ConfigContext object
		,      const char *name)	///<[in] Name to get the NetAddr value of
{
	gpointer	ret = g_hash_table_lookup(self->_values, name);
	ConfigValue*	cfg;

	if (ret == NULL) {
		return NULL;
	}
	cfg = CASTTOCLASS(ConfigValue, ret);
	if (cfg->valtype != CFG_NETADDR) {
		return NULL;
	}
	return CASTTOCLASS(NetAddr, cfg->objvalue);
}

/// Set the NetAddr value of a name
FSTATIC void
_configcontext_setaddr(ConfigContext* self	///<[in/out] ConfigContext object
		,      const char * name	///<[in] Name to set to 'addr' (we copy it)
		,      NetAddr* addr)		///<[in/out] Address to set it to (we hold a ref to it)
{
	char *	cpname = g_strdup(name);
	ConfigValue* val = _configcontext_value_new(CFG_NETADDR);

	addr->baseclass.ref(addr);
	val->objvalue = addr;
	g_hash_table_replace(self->_values, cpname, val);
}

/// Return the @ref Frame value of a name
FSTATIC Frame*
_configcontext_getframe(ConfigContext* self	///<[in] ConfigContext object
		,       const char *name)	///<[in] Name to retrieve the @ref Frame value of
{
	gpointer	ret = g_hash_table_lookup(self->_values, name);
	ConfigValue*	cfg;

	if (ret == NULL) {
		return NULL;
	}
	cfg = CASTTOCLASS(ConfigValue, ret);
	if (cfg->valtype != CFG_FRAME) {
		return NULL;
	}
	return CASTTOCLASS(Frame, cfg->objvalue);
}

/// Set the signature frame to the given SignFrame
FSTATIC void
_configcontext_setframe(ConfigContext* self	///<[in/out] ConfigContext object
		,	const char * name	///<[in] name to set value of (we copy it)
		,	Frame* frame)		///<[in/out] @ref Frame value to set 'name' to
						/// (we hold a ref to it)
{
	char *	cpname = g_strdup(name);
	ConfigValue* val = _configcontext_value_new(CFG_FRAME);

	frame->baseclass.ref(frame);
	val->objvalue = frame;
	g_hash_table_replace(self->_values, cpname, val);
}


#define	JSONQUOTES	"\\\""
FSTATIC char *
JSONquotestring(char * s, gboolean ismalloced)
{
	GString*	ret;
	char *		str;
	if (strpbrk(s, JSONQUOTES) == NULL)  {
		return s;
	}
	ret = g_string_sized_new(strlen(s)+3);
	
	for (str=s;*str ; ++str ) {
		if (strchr(JSONQUOTES, *str )) {
			g_string_append_c(ret, '\\');
		}
		g_string_append_c(ret, *str);
	}
	if (ismalloced) {
		g_free(s);
	}
	return g_string_free(ret, FALSE);
}


/// Convert a ConfigContext to a printable string (in JSON notation)
FSTATIC char *
_configcontext_toString(gconstpointer aself)
{
	const ConfigContext*	self = CASTTOCONSTCLASS(ConfigContext, aself);

	GString*	gsret = g_string_new("{");
	GHashTableIter	iter;
	const char *	comma = "";
	gpointer	gkey;
	gpointer	gvalue;
	
	if (!self->_values) {
		return NULL;
	}
	/// @todo - return this string with keys in canonical (sorted) order
	/// - at least for tests - unsure if this will be needed.
	g_hash_table_iter_init(&iter, self->_values);
	while (g_hash_table_iter_next(&iter, &gkey, &gvalue)) {
		ConfigValue*	val = CASTTOCLASS(ConfigValue, gvalue);

		switch(val->valtype) {
			case CFG_EEXIST:
			case CFG_NULL:
				g_string_append_printf(gsret, "%s\"%s\":null"
				,	comma, (const char *)gkey);
				break;

			case CFG_BOOL:
				g_string_append_printf(gsret, "%s\"%s\":%s"
				,	comma, (const char *)gkey
				,	(val->intvalue? "true" : "false"));

			case CFG_INT64:
				g_string_append_printf(gsret, "%s\"%s\":"FMT_64BIT"d"
				,	comma, (const char *)gkey, val->intvalue);
				break;

			case CFG_FLOAT:
				g_string_append_printf(gsret, "%s\"%s\":%g"
				,	comma, (const char *)gkey
				,	val->floatvalue);
				break;

			case CFG_STRING: {
				char *	quotedstr = JSONquotestring((char*)val->objvalue
						    ,		    FALSE);
				g_string_append_printf(gsret, "%s\"%s\":\"%s\""
				,	comma, (const char *)gkey,  quotedstr);
				if (quotedstr != (char*) val->objvalue) {
					g_free(quotedstr); quotedstr = NULL;
				}
				break;
			}

			case CFG_ARRAY:
			case CFG_CFGCTX:
			case CFG_NETADDR:
			case CFG_FRAME: {
				AssimObj*	obj = CASTTOCLASS(AssimObj, val->objvalue);
				char *	str = obj->toString(obj);
				str = JSONquotestring(str, TRUE);
				g_string_append_printf(gsret, "%s\"%s\":\"%s\""
				,	comma, (const char *)gkey,  str);
				g_free(str); str = NULL;
				break;
			}
		}
		
		comma=",";
	}
	g_string_append(gsret, "}");
	return g_string_free(gsret, FALSE);
}

///
///	Create a GScanner object that is set up to scan JSON text.
///	See <a href="http://www.json.org/">JSON web site</a> for details
///	on JSON syntax.
FSTATIC GScanner*
_configcontext_JSON_GScanner_new(void)
{
	static GScannerConfig	config;
	GScanner*		retval;
	// Legal JSON keywords are true, false, and null
	// There are no 'identifiers' as such.
	static char		firstchars[] = "tfn";
	static char		subsequentchars[] = "aelrsu";
	static char		whitespace[] = " \t\n\r\f";
	static char		True[] = "true";
	static char		False[] = "false";
	static char		Null[] = "null";
	memset(&config, 0, sizeof(config));

	// For more info on what these settings do, see
	// http://developer.gnome.org/glib/2.32/glib-Lexical-Scanner.html

	config.cset_skip_characters = whitespace;
	config.cset_identifier_first = firstchars;
	config.cset_identifier_nth = subsequentchars;
	config.case_sensitive = TRUE;
	config.skip_comment_multi = FALSE;
	config.skip_comment_single = FALSE;
	config.scan_comment_multi = FALSE;
	config.scan_identifier = TRUE;
	config.scan_identifier_1char = FALSE;
	config.scan_identifier_NULL = FALSE;
	config.scan_symbols = TRUE; // ???
	config.scan_binary = FALSE;
	config.scan_octal = FALSE;
	config.scan_float = TRUE;
	config.scan_hex = FALSE;
	config.scan_hex_dollar = FALSE;
	config.scan_string_sq = FALSE;
	config.scan_string_dq = TRUE;
	config.numbers_2_int = TRUE;
	config.int_2_float = FALSE;
	config.identifier_2_string = FALSE;
	config.char_2_token = TRUE;
	config.symbol_2_token = TRUE; // ???
	config.scope_0_fallback = TRUE;
	config.store_int64 = TRUE;

	retval =  g_scanner_new(&config);
	if (retval) {
		g_scanner_scope_add_symbol(retval, 0, True, True);
		g_scanner_scope_add_symbol(retval, 0, False, False);
		g_scanner_scope_add_symbol(retval, 0, Null, Null);
	}
	return retval;
}

#define TOKEN_COLON	':'
#define	GULP	(void)g_scanner_get_next_token(scan)

FSTATIC ConfigContext*
_configcontext_JSON_parse_object(GScanner* scan)
{
	ConfigContext*	ret;
	ConfigContext*	membersret;
	if (g_scanner_peek_next_token(scan) != G_TOKEN_LEFT_CURLY) {
		// Syntax error
		return NULL;
	}
	GULP;	// Swallow '{'
	ret = configcontext_new(0);
	if (g_scanner_peek_next_token(scan) == G_TOKEN_RIGHT_CURLY) {
		// Empty 'object'
		GULP;
		return ret;
	}
	
	membersret = _configcontext_JSON_parse_members(scan, ret);
	if (membersret == NULL) {
		ret->baseclass.unref(ret); ret = NULL;
		return ret;
	}

	if (g_scanner_get_next_token(scan) != G_TOKEN_RIGHT_CURLY) {
		// Syntax error...
		ret->baseclass.unref(ret); ret = NULL;
		return ret;
	}
	return ret;
}
FSTATIC ConfigContext*
_configcontext_JSON_parse_members(GScanner* scan, ConfigContext* cfg)
{
	while (g_scanner_peek_next_token(scan) == G_TOKEN_STRING) {
		_configcontext_JSON_parse_pair(scan, cfg);
		if (g_scanner_peek_next_token(scan) == G_TOKEN_COMMA) {
			GULP;
		}else{
			break;
		}
	}
	return cfg;
}

// Parse a "name": value pair
FSTATIC ConfigContext*
_configcontext_JSON_parse_pair(GScanner* scan, ConfigContext* cfg)
{
	char *		name = NULL;
	// "name" : _value_	pairs
	//
	// Name is always a string -
	// Value can be any of:
	//	string
	//	number
	//	object
	//	array
	//	true (a symbol)
	//	false (a symbol)
	//	null (a symbol)
	if (g_scanner_peek_next_token(scan) != G_TOKEN_STRING) {
		return NULL;
	}
	GULP;
	if (g_scanner_peek_next_token(scan) != TOKEN_COLON) {
		// Syntax error
		return NULL;
	}
	// Get value of G_TOKEN_STRING
	name = g_strdup(scan->value.v_string);
	GULP;	// Swallow TOKEN_COLON
	// OK - we now have several next symbol possibilties:
	switch(g_scanner_peek_next_token(scan)) {
		case G_TOKEN_STRING:{		// String
			GULP;
			cfg->setstring(cfg, name, scan->value.v_string);
			break;
		}
		case G_TOKEN_INT: {		// Integer
			GULP;
			cfg->setint(cfg, name, scan->value.v_int64);
			break;
		}
		case G_TOKEN_SYMBOL: {		// true, false, null
			GULP;
			if (strcmp(scan->value.v_string, "true") == 0) {
				cfg->setint(cfg, name, 1);
			}else if (strcmp(scan->value.v_string, "false") == 0) {
				cfg->setint(cfg, name, 0);
			}else if (strcmp(scan->value.v_string, "null") == 0) {
				/// @todo fix the case of null value
				cfg->setint(cfg, name, 0);
			}else{
				// Syntax error
				g_free(name); name = NULL;
				return NULL;
			}
			break;
		}
	

		// Things we don't  support properly (yet)
		case G_TOKEN_LEFT_CURLY:	// Object
			/// @todo: add objects to the ConfigContext object
			(void)_configcontext_JSON_parse_object(scan);
			break;

		// Things we don't support (yet)

		/// @todo: add Floats to the ConfigContext object
		case G_TOKEN_FLOAT:		// Floating point number

		/// @todo: add Arrays to the ConfigContext object
		case G_TOKEN_LEFT_BRACE:	// Array
		default:
			// Syntax error
			if (name) {
				g_free(name); name = NULL;
			}
			return NULL;
	}
	if (name) {
		g_free(name); name = NULL;
	}
	return cfg;
	
}

FSTATIC ConfigValue*
_configcontext_value_new(enum ConfigValType t)
{
	ConfigValue*	ret;

	ret = MALLOCBASECLASS(ConfigValue);
	if (ret) {
		ret->valtype = t;
		ret->objvalue = NULL;
	}
	return ret;
}

FSTATIC void
_configcontext_value_finalize(gpointer vself)
{
	ConfigValue*	self;

	self = CASTTOCLASS(ConfigValue, vself);
	switch (self->valtype) {
		case CFG_STRING:
			g_free(self->objvalue); self->objvalue = NULL;
			break;
		case CFG_CFGCTX:
		case CFG_NETADDR:
		case CFG_FRAME: {
			AssimObj*	obj = CASTTOCLASS(AssimObj, self->objvalue);
			obj->unref(obj); obj = NULL; self->objvalue = NULL;
			break;
		}

		default: {
			// Do nothing
			break;
		}
	}
	FREECLASSOBJ(self);
	vself = NULL;
}

///@}
