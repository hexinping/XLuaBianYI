/*
@author duwenyuan

a dream land lua extension for byte array
*/
#pragma warning( push )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4244 )

#ifndef LUA_LIB
#define LUA_LIB 
#endif

#include "drlua.h"

#include <lua.h>
#include "lualib.h"
#include "lauxlib.h"

#include <string.h>
#include <stdint.h>
#include "i64lib.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <assert.h>
#include <limits.h>

#if USING_LUAJIT
#include "lj_obj.h"
#else
#include "lstate.h"
#endif

static LUA_MEM_CALLBACK _memCallback = 0;
LUA_MEM_CALLBACK lua_get_mem_callback()
{
    return _memCallback;
}

static int _totalAlloc = 0;
int lua_on_alloc_mem(int size)
{
    _totalAlloc += size;
    return 0;
}

static int _totalFree = 0;
int lua_on_free_mem(int size)
{
    _totalFree += size;
    return 0;
}

LUA_API int dr_lua_get_mem(int type)
{
    if (type == 0) return _totalAlloc;

    return _totalFree;
}

LUA_API int dr_lua_init_memcallback()
{
    _memCallback = 0;
    //set to zero.
    _memCallback = 0;
    return 0;
}


LUA_API int dr_lua_set_memcallback(LUA_MEM_CALLBACK callback)
{
    _memCallback = callback;
    return 0;
}






#pragma warning( pop )