#define lmylib_c
#define LUA_LIB

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

#include "lprefix.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

//入参lua_State *L
//返回1 则函数执行成功
//通过luaL_checkinteger获取入参
static int mylib_number(lua_State* L) {
	int d = luaL_checkinteger(L, 1); //获取参数
	lua_pushinteger(L, d + 100);  /* push result */
	return 1;
}

//mylib_number函数名
//调用方式：mylib.mylib_number(99)
static const struct luaL_Reg mylib[] = {
	{"mylib_number", mylib_number},
	{NULL, NULL}
};

//mylib = 模块名称
// luaL_newlib 初始化模块
extern int luaopen_mylib(lua_State* L) {
	luaL_newlib(L, mylib);
	return 1;
}
