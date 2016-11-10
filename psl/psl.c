#include <stdlib.h> /* free() */
#include <libpsl.h>

#include <lua.h>
#include <lauxlib.h>

static const psl_ctx_t **luapsl_preppslctx(lua_State *L) {
	const psl_ctx_t **ud = lua_newuserdata(L, sizeof(psl_ctx_t*));
	*ud = NULL;
	luaL_setmetatable(L, "psl_ctx_t*");
	return ud;
}

static void luapsl_pushpslctx(lua_State *L, const psl_ctx_t *psl) {
	if (psl == NULL) {
		lua_pushnil(L);
	} else {
		const psl_ctx_t **ud = luapsl_preppslctx(L);
		*ud = psl;
	}
}

static const psl_ctx_t *luapsl_checkpslctx(lua_State *L, int idx) {
	const psl_ctx_t **ud = luaL_checkudata(L, idx, "psl_ctx_t*");
	return *ud;
}

static const psl_ctx_t *luapsl_checkpslctxnotnull(lua_State *L, int idx) {
	const psl_ctx_t *psl = luapsl_checkpslctx(L, idx);
	luaL_argcheck(L, psl != NULL, idx, "psl_ctx_t* is freed");
	return psl;
}

static int luapsl__tostring(lua_State *L) {
	void *ud = lua_touserdata(L, 1);
	const char* typename = "userdata";
	if (lua_getmetatable(L, 1) && lua_getfield(L, -1, "__name")) {
		typename = lua_tostring(L, -1);
	}
	lua_pushfstring(L, "%s: %p", typename, ud);
	return 1;
}

static int luapsl_load_file(lua_State *L) {
	const char *filename = luaL_checkstring(L, 1);
	const psl_ctx_t **ud = luapsl_preppslctx(L);
	*ud = psl_load_file(filename);
	if (*ud == NULL) {
		lua_pushnil(L);
	}
	return 1;
}

static int luapsl_load_fp(lua_State *L) {
	luaL_Stream *stream = luaL_checkudata(L, 1, LUA_FILEHANDLE);
	/* psl_load_fp doesn't hold onto the FILE*, so no need to e.g. stow away a reference */
	FILE *file = stream->f;
	const psl_ctx_t **ud = luapsl_preppslctx(L);
	*ud = psl_load_fp(file);
	if (*ud == NULL) {
		lua_pushnil(L);
	}
	return 1;
}

/* builtin is cached in upvalue */
static int luapsl_builtin(lua_State *L) {
	lua_pushvalue(L, lua_upvalueindex(1));
	return 1;
}

static int luapsl_free(lua_State *L) {
	psl_ctx_t **ud = luaL_checkudata(L, 1, "psl_ctx_t*");
	if (*ud != NULL && *ud != psl_builtin()) {
		psl_free(*ud);
		*ud = NULL;
	}
	return 0;
}

static int luapsl_is_public_suffix(lua_State *L) {
	const psl_ctx_t *psl = luapsl_checkpslctxnotnull(L, 1);
	const char *domain = luaL_checkstring(L, 2);
	int type = luaL_optinteger(L, 3, PSL_TYPE_ANY);
	lua_pushboolean(L, psl_is_public_suffix2(psl, domain, type));
	return 1;
}

static int luapsl_unregistrable_domain(lua_State *L) {
	const psl_ctx_t *psl = luapsl_checkpslctxnotnull(L, 1);
	const char *domain = luaL_checkstring(L, 2);
	const char *res = psl_unregistrable_domain(psl, domain);
	lua_pushstring(L, res);
	return 1;
}

static int luapsl_registrable_domain(lua_State *L) {
	const psl_ctx_t *psl = luapsl_checkpslctxnotnull(L, 1);
	const char *domain = luaL_checkstring(L, 2);
	const char *res = psl_registrable_domain(psl, domain);
	lua_pushstring(L, res);
	return 1;
}

static int luapsl_suffix_count(lua_State *L) {
	const psl_ctx_t *psl = luapsl_checkpslctxnotnull(L, 1);
	int	count = psl_suffix_count(psl);
	lua_pushinteger(L, count);
	return 1;
}

static int luapsl_suffix_exception_count(lua_State *L) {
	const psl_ctx_t *psl = luapsl_checkpslctxnotnull(L, 1);
	int	count = psl_suffix_exception_count(psl);
	lua_pushinteger(L, count);
	return 1;
}

static int luapsl_suffix_wildcard_count(lua_State *L) {
	const psl_ctx_t *psl = luapsl_checkpslctxnotnull(L, 1);
	int	count = psl_suffix_wildcard_count(psl);
	lua_pushinteger(L, count);
	return 1;
}

static int luapsl_builtin_file_time(lua_State *L) {
	time_t t = psl_builtin_file_time();
	if (t == 0) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, t);
	}
	return 1;
}

static int luapsl_builtin_sha1sum(lua_State *L) {
	lua_pushstring(L, psl_builtin_sha1sum());
	return 1;
}

static int luapsl_builtin_filename(lua_State *L) {
	lua_pushstring(L, psl_builtin_filename());
	return 1;
}

static int luapsl_builtin_outdated(lua_State *L) {
	lua_pushboolean(L, psl_builtin_outdated());
	return 1;
}

static int luapsl_is_cookie_domain_acceptable(lua_State *L) {
	const psl_ctx_t *psl = luapsl_checkpslctxnotnull(L, 1);
	const char *hostname = luaL_checkstring(L, 2);
	const char *cookie_domain = luaL_checkstring(L, 3);
	lua_pushboolean(L, psl_is_cookie_domain_acceptable(psl, hostname, cookie_domain));
	return 1;
}

static int luapsl_get_version(lua_State *L) {
	lua_pushstring(L, psl_get_version());
	return 1;
}

/* Returns the library version number if the given version number is at least
the version of the library, else return 0; If the argument is 0, the function
returns the library version number without performing a check. */
static int luapsl_check_version_number(lua_State *L) {
	int minversion = luaL_optinteger(L, 1, 0);
	int res = psl_check_version_number(minversion);
	if (res == 0) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushinteger(L, res);
	}
	return 1;
}

static int luapsl_str_to_utf8lower(lua_State *L) {
	const char *str = luaL_checkstring(L, 1);
	const char *encoding = luaL_optstring(L, 2, NULL);
	const char *locale = luaL_optstring(L, 3, NULL);
	char **lower = lua_newuserdata(L, sizeof(char*));
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_setmetatable(L, -2);
	psl_error_t res = psl_str_to_utf8lower(str, encoding, locale, lower);
	switch(res) {
	case PSL_SUCCESS:
		lua_pushstring(L, *lower);
		return 1;
	case PSL_ERR_INVALID_ARG:
		return luaL_argerror(L, 1, "string invalid");
	case PSL_ERR_CONVERTER: /* Failed to open the unicode converter with name encoding */
	case PSL_ERR_TO_UTF16: /* Failed to convert str to unicode */
	case PSL_ERR_TO_LOWER: /* Failed to convert unicode to lowercase */
	case PSL_ERR_TO_UTF8: /* Failed to convert unicode to UTF-8 */
	default:
		lua_pushnil(L);
		return 1;
	}
}

static int boxed_pointer__gc(lua_State *L) {
	void **ud = lua_touserdata(L, 1);
	free(*ud);
	*ud = NULL;
}

int luaopen_psl(lua_State *L) {
	static const luaL_Reg lib[] = {
		{"load_file", luapsl_load_file},
		{"load_fp", luapsl_load_fp},
		{"builtin", luapsl_builtin},
		{"free", luapsl_free},
		{"builtin_file_time", luapsl_builtin_file_time},
		{"builtin_sha1sum", luapsl_builtin_sha1sum},
		{"builtin_filename", luapsl_builtin_filename},
		{"builtin_outdated", luapsl_builtin_outdated},
		{"get_version", luapsl_get_version},
		{"check_version_number", luapsl_check_version_number},
		{"str_to_utf8lower", luapsl_str_to_utf8lower},
		{NULL, NULL}
	};

	static const luaL_Reg psl_methods[] = {
		{"is_public_suffix", luapsl_is_public_suffix},
		{"unregistrable_domain", luapsl_unregistrable_domain},
		{"registrable_domain", luapsl_registrable_domain},
		{"suffix_count", luapsl_suffix_count},
		{"suffix_exception_count", luapsl_suffix_exception_count},
		{"suffix_wildcard_count", luapsl_suffix_wildcard_count},
		{"is_cookie_domain_acceptable", luapsl_is_cookie_domain_acceptable},
		{NULL, NULL}
	};

	/* create psl metatable */
	luaL_newmetatable(L, "psl_ctx_t*");
	lua_pushcfunction(L, luapsl_free);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, luapsl__tostring);
	lua_setfield(L, -2, "__tostring");
	luaL_newlib(L, psl_methods);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_newlib(L, lib);

	lua_pushliteral(L, PSL_VERSION);
	lua_setfield(L, -2, "PSL_VERSION");
	lua_pushinteger(L, PSL_VERSION_MAJOR);
	lua_setfield(L, -2, "PSL_VERSION_MAJOR");
	lua_pushinteger(L, PSL_VERSION_MINOR);
	lua_setfield(L, -2, "PSL_VERSION_MINOR");
	lua_pushinteger(L, PSL_VERSION_NUMBER);
	lua_setfield(L, -2, "PSL_VERSION_NUMBER");
	lua_pushinteger(L, PSL_VERSION_PATCH);
	lua_setfield(L, -2, "PSL_VERSION_PATCH");

	/* constants for psl_is_public_suffix2 */
	lua_createtable(L, 0, 3);
	lua_pushinteger(L, PSL_TYPE_PRIVATE);
	lua_setfield(L, -2, "PRIVATE");
	lua_pushinteger(L, PSL_TYPE_ICANN);
	lua_setfield(L, -2, "ICANN");
	lua_pushinteger(L, PSL_TYPE_ANY);
	lua_setfield(L, -2, "ANY");
	lua_setfield(L, -2, "TYPE");

	/* cache builtin as upvalue so same pointer is returned every time */
	luapsl_pushpslctx(L, psl_builtin());
	lua_pushcclosure(L, luapsl_builtin, 1);
	lua_setfield(L, -2, "builtin");

	/* special metatable for boxed pointers that need to be freed */
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, boxed_pointer__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcclosure(L, luapsl_str_to_utf8lower, 1);
	lua_setfield(L, -2, "str_to_utf8lower");

	return 1;
}