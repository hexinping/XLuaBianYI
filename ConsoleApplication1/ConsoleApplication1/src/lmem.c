/*
** $Id: lmem.c,v 1.91.1.1 2017/04/19 17:20:42 roberto Exp $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"



/*
** About the realloc function:
** void * frealloc (void *ud, void *ptr, size_t osize, size_t nsize);
** ('osize' is the old size, 'nsize' is the new size)
**
** * frealloc(ud, NULL, x, s) creates a new block of size 's' (no
** matter 'x').
**
** * frealloc(ud, p, x, 0) frees the block 'p'
** (in this specific case, frealloc must return NULL);
** particularly, frealloc(ud, NULL, 0, 0) does nothing
** (which is equivalent to free(NULL) in ISO C)
**
** frealloc returns NULL if it cannot create or reallocate the area
** (any reallocation to an equal or smaller size cannot fail!)
*/



#define MINSIZEARRAY	4


/**
 * 内存扩容
 * 1. 扩容不能超过limit限制，如果超过了，则扩容到limit
 * 2. 一般扩容按照*2系数去扩
 */
void* luaM_growaux_(lua_State* L, void* block, int* size, size_t size_elems,
    int limit, const char* what) {
    void* newblock;
    int newsize;
    if (*size >= limit / 2) {  /* cannot double it? */
        if (*size >= limit)  /* cannot grow even a little? */
            luaG_runerror(L, "too many %s (limit is %d)", what, limit);
        newsize = limit;  /* still have at least one free place */
    }
    else {
        newsize = (*size) * 2;
        if (newsize < MINSIZEARRAY)
            newsize = MINSIZEARRAY;  /* minimum size */
    }
    newblock = luaM_reallocv(L, block, *size, newsize, size_elems); //最后调用的还是luaM_realloc_方法，然后调用l_alloc方法
    *size = newsize;  /* update only when everything else is OK */
    return newblock;
}


l_noret luaM_toobig(lua_State* L) {
    luaG_runerror(L, "memory allocation error: block too big");
}


/*
** generic allocation routine.
** 内存分配函数
** 内容块最终会调用*g->frealloc（*l_alloc）函数处理
**
** osize = 老的内存块大小  这参数没用上
**
*/
void* luaM_realloc_(lua_State* L, void* block, size_t osize, size_t nsize) {
    void* newblock;
    global_State* g = G(L);
    size_t realosize = (block) ? osize : 0;
    lua_assert((realosize == 0) == (block == NULL));
#if defined(HARDMEMTESTS)
    if (nsize > realosize && g->gcrunning)
        luaC_fullgc(L, 1);  /* force a GC whenever possible */
#endif
  //g->frealloc（*l_alloc）函数处理
    newblock = (*g->frealloc)(g->ud, block, osize, nsize); //  *g->frealloc 指的就是创建lua_newstate时候赋给的函数指针l_alloc 
    if (newblock == NULL && nsize > 0) {
        lua_assert(nsize > realosize);  /* cannot fail when shrinking a block */
        if (g->version) {  /* is state fully built? */
            luaC_fullgc(L, 1);  /* try to free some memory... */
            newblock = (*g->frealloc)(g->ud, block, osize, nsize);  /* try again */
        }
        if (newblock == NULL)
            luaD_throw(L, LUA_ERRMEM);
    }
    lua_assert((nsize == 0) == (newblock == NULL));
    g->GCdebt = (g->GCdebt + nsize) - realosize;
    return newblock;
}

