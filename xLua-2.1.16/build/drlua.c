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

#define DL_ARRAY_NAME "dlarray"
#define DL_ARRAY_META "__dl_array"

#define uchar(c)	((unsigned char)(c))

static USER_DATA_ALLOC _callback = 0;

static int8_t* _lastest_ptr = 0;
static uint32_t* _lastest_len = 0;





/* translate a relative string position: negative means back from end */
static lua_Integer posrelat(lua_Integer pos, size_t len) {
    if (pos >= 0) return pos;
    else if (0u - (size_t)pos > len) return 0;
    else return (lua_Integer)len + pos + 1;


}


LUA_API int l_array_new(lua_State* L)
{
    int hsize = sizeof(((ByteArray*)0)->size);

    int n = luaL_checkinteger(L, 1);
    //nbytes actually enough for n elements
    size_t nbytes = sizeof(ByteArray) + n * sizeof(int8_t);
    ByteArray* ptr = (ByteArray*)lua_newuserdata(L, nbytes);
    ptr->size = n;

    if (_callback != 0)
    {
        //
        _callback((int8_t*)ptr, ptr->size, hsize);
    }

    _lastest_ptr = (int8_t*)ptr;
    _lastest_len = ptr->size;

    return 1; /* new userdatum is already on the stack */
}



LUA_API int l_array_set_byte(lua_State* L)
{
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    int index = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, ptr != NULL, 1, "`array' expected");
    luaL_argcheck(L, 1 <= index && index <= ptr->size, 2, "index out of range");
    ptr->values[TO_C_INDEX(index)] = uchar(value);
    return 0;
}

LUA_API int l_array_get_byte(lua_State* L)
{
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    int index = luaL_checkinteger(L, 2);

    luaL_argcheck(L, ptr != NULL, 1, "'array' expected");
    luaL_argcheck(L, 1 <= index && index <= ptr->size, 2, "index out of range");
    lua_pushinteger(L, uchar(ptr->values[TO_C_INDEX(index)]));

    //return count
    return 1;
}

LUA_API int l_array_set_bytes(lua_State* L)
{
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    int index = luaL_checkinteger(L, 2);
    int n = luaL_checkinteger(L, 3);
    int i;


    luaL_argcheck(L, ptr != NULL, 1, "'array' expected");
    luaL_argcheck(L, 1 <= index && index <= ptr->size, 2, "index out of range");
    luaL_argcheck(L, 1 <= index + n && index + n <= ptr->size, 3, "count out of range");

    for (i = 0; i < n; i++)
        ptr->values[TO_C_INDEX(index + i)] = uchar(luaL_checkinteger(L, 3 + i));

    //return count
    return 0;
}


LUA_API int l_array_get_bytes(lua_State* L)
{
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    int index = luaL_checkinteger(L, 2);
    int n = luaL_checkinteger(L, 3);
    int i;

    //char fmt[1000] = { 0 };
    //sprintf(fmt, "getting index out of ByteArray range %d -- %d", index + n, ptr->size);
    luaL_argcheck(L, index + n <= ptr->size, 3, "getting index out of ByteArray range");


    luaL_argcheck(L, ptr != NULL, 1, "'array' expected");
    luaL_argcheck(L, 1 <= index && index <= ptr->size, 2, "index out of range");
    luaL_argcheck(L, 1 <= index + n && index + n <= ptr->size, 3, "count out of range");

    luaL_checkstack(L, n, "byte array slice too long");
    for (i = 0; i < n; i++)
        lua_pushinteger(L, uchar(ptr->values[TO_C_INDEX(index + i)]));

    //return count
    return n;
}



LUA_API int l_array_get_str(lua_State* L)
{
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    lua_Integer count = posrelat(luaL_optinteger(L, 3, -1), l);
    luaL_argcheck(L, TO_C_INDEX(index) + count <= ptr->size, 3, "getting index out of ByteArray range");


    if (index < 1) index = 1;
    if (count > (lua_Integer)l) count = l;

    if (TO_C_INDEX(index) + count <= ptr->size)
        lua_pushlstring(L, ptr->values + TO_C_INDEX(index), count);
    else
        lua_pushliteral(L, "");
    return 1;
}

/**
 * dlarray.set(t, "abc", index)
 */
LUA_API int l_array_set_str(lua_State* L)
{
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);

    size_t l;
    const char* s = luaL_checklstring(L, 2, &l);
    lua_Integer start = posrelat(luaL_checkinteger(L, 3), l);
    lua_Integer count = posrelat(luaL_optinteger(L, 4, -1), l);
    luaL_argcheck(L, start + count < ptr->size, 4, "insert str out of ByteArray size");


    if (start < 1) start = 1;
    if (count > (lua_Integer)l) count = l;

    if (start <= count)
    {
        for (int i = 0; i < l; i++)
        {
            ptr->values[start - 1 + i] = (uint32_t)(s[i]);
        }
    }
    return 0;
}

LUA_API int l_array_size(lua_State* L)
{
    ByteArray* a = (ByteArray*)luaL_checkudata(L, 1, DL_ARRAY_NAME);
    luaL_argcheck(L, a != NULL, 1, "'array' expected.");
    lua_pushinteger(L, a->size);
    return 1;
}

static int l_tostring(lua_State* L) {
    char str[256] = { "greating from c" };
    lua_pushstring(L, str);
    return 1;
}



//////////////////////////////////////////////////////////////////////////////
//////增加WriteBytes 和 ReadFloat, ReadInt等
///// Float和Double需要注重格式
///// Mp使用BigEndian

#define _BUFF_LEN 65

static const union {
    int dummy;
    char little;  /* true iff machine is little endian */
} nativeendian = { 1 };

typedef union DRL_Ftypes {
    float f;
    double d;
    lua_Number n;
    char buff[5 * sizeof(lua_Number)];  /* enough for any float type */
} DRL_F_types;


static void copywithendian(volatile char* dest, volatile const char* src,
    int size, int islittle) {
    if (islittle == nativeendian.little) {
        while (size-- != 0)
            *(dest++) = *(src++);
    }
    else {
        dest += size - 1;
        while (size-- != 0)
            *(dest--) = *(src++);
    }
}

#define NB	CHAR_BIT
#define SZINT	((int)sizeof(lua_Integer))
/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

void DR_ASSERT(int code, const char* fmt)
{
#if !defined(ANDROID) && defined(__WIN32)
    assert(0, "byte integer does not fit into Lua Integer");
#else

#endif
}


/**
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.

(opt == Kint)
*/

#define NB	CHAR_BIT
#define SZINT	((int)sizeof(lua_Integer))
/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

static lua_Integer __unpackint(const char* str,
    int islittle, int size, int issigned) {
    lua_Unsigned res = 0;
    int i;
    int limit = (size <= SZINT) ? size : SZINT;
    for (i = limit - 1; i >= 0; i--) {
        res <<= NB;
        res |= (lua_Unsigned)(unsigned char)str[islittle ? i : size - 1 - i];
    }
    if (size < SZINT) {  /* real size smaller than lua_Integer? */
        if (issigned) {  /* needs sign extension? */
            lua_Unsigned mask = (lua_Unsigned)1 << (size * NB - 1);
            res = ((res ^ mask) - mask);  /* do sign extension */
        }
    }
    else if (size > SZINT) {  /* must check unread bytes */
        int mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
        for (i = limit; i < size; i++) {
            if ((unsigned char)str[islittle ? i : size - 1 - i] != mask)
                DR_ASSERT(0, "byte integer does not fit into Lua Integer");
        }
    }
    return (lua_Integer)res;
}

int8_t __read_byte(ByteArray* ptr, int idx)
{
    return ptr->values[idx];
}

int __read_bytes(ByteArray* ptr, int8_t* dst, int idx, int count)
{
    int j = 0;
    for (j = 0;
        j < count && idx + count <= ptr->size;  j++)
    {
        dst[j] = ptr->values[idx + j];
    }
    return j;
}

int64_t __read_int64(ByteArray* ptr, int idx)
{
    int8_t buff[_BUFF_LEN] = { 0 };
    __read_bytes(ptr, buff, idx, 8);

    int64_t val = __unpackint(buff, 0, 8, 1);

    return val;
}

///Integers
LUA_API int drl_read_uint8(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 1 < ptr->size, 2, "getting index out of ByteArray range");

    uint8_t val = (uint8_t)__read_byte(ptr, TO_C_INDEX(index));
    lua_pushinteger(L, val);

    return 1;
}

LUA_API int drl_read_uint16(lua_State* L)
{
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 2 < ptr->size, 2, "getting index out of ByteArray range");

    uint8_t buff[_BUFF_LEN] = { 0 };
    __read_bytes(ptr, buff, TO_C_INDEX(index), 2);

    //uint16_t val = (uint16_t)(buff[0] << 8 | buff[1]);
    uint16_t val = (uint16_t)(__unpackint(buff, 0, 2, 0));
    lua_pushinteger(L, val);

    return 1;
}

LUA_API int drl_read_uint32(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 4 < ptr->size, 2, "getting index out of ByteArray range");

    int8_t buff[_BUFF_LEN] = { 0 };
    __read_bytes(ptr, buff, TO_C_INDEX(index), 4);

    int val2 = (uint8_t)buff[0] << 24 | (uint8_t)buff[1] << 16 | (uint8_t)buff[2] << 8 | (uint8_t)buff[3];
    uint32_t val = (uint32_t)(__unpackint(buff, 0, 4, 0));
    lua_pushinteger(L, (uint32_t)val);

    return 1;
}

LUA_API int drl_read_uint64(lua_State* L)
{ //应该使用string
    DR_ASSERT(0, "read uint 64 really?");
    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 8 < ptr->size, 2, "getting index out of ByteArray range");


    int64_t val = __read_int64(ptr, TO_C_INDEX(index));
    lua_pushinteger(L, (uint64_t)val);

    return 1;
}


LUA_API int drl_read_int8(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 1 < ptr->size, 2, "getting index out of ByteArray range");
    lua_pushinteger(L, __read_byte(ptr, TO_C_INDEX(index)));

    return 1;
}

LUA_API int drl_read_int16(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 2 < ptr->size, 2, "getting index out of ByteArray range");

    int8_t buff[_BUFF_LEN] = { 0 };
    __read_bytes(ptr, buff, TO_C_INDEX(index), 2);

    //int val = buff[0] << 8 | buff[1];
    int16_t val = (int16_t)(__unpackint(buff, 0, 2, 0));
    lua_pushinteger(L, val);

    return 1;
}

LUA_API int drl_read_int32(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 4 < ptr->size, 2, "getting index out of ByteArray range");

    int8_t buff[_BUFF_LEN] = { 0 };
    __read_bytes(ptr, buff, TO_C_INDEX(index), 4);

    //int val = buff[0] << 24 | buff[1] << 16 | buff[2] << 8 | buff[3];
    int32_t val = (int32_t)(__unpackint(buff, 0, 4, 0));
    lua_pushinteger(L, (int32_t)val);


    return 1;
}

LUA_API int drl_read_int64(lua_State* L)
{
    DR_ASSERT(0, "read int 64 really?");

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 8 < ptr->size, 2, "getting index out of ByteArray range");

    lua_pushinteger(L, (int32_t)0);
    return 1;
}



///Single or Double

static FILE* fp = 0;
LUA_API int drl_read_float(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 4 < ptr->size, 2, "getting index out of ByteArray range");

    DRL_F_types f;

    char buff[_BUFF_LEN] = { 0 };
    __read_bytes(ptr, buff, TO_C_INDEX(index), 4);

    //int val = buff[0] << 24 | buff[1] << 16 | buff[2] << 8 | buff[3];
    copywithendian(f.buff, buff, 4, 0);

    float fval = f.f;
    lua_pushnumber(L, fval);

    return 1;
}

LUA_API int drl_read_double(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);
    luaL_argcheck(L, index + 8 < ptr->size, 2, "getting index out of ByteArray range");

    DRL_F_types f;

    char buff[_BUFF_LEN] = { 0 };
    __read_bytes(ptr, buff, TO_C_INDEX(index), 8);

    //int val = buff[0] << 24 | buff[1] << 16 | buff[2] << 8 | buff[3];
    copywithendian(f.buff, buff, 8, 0);

    double dval = f.d;
    lua_pushnumber(L, dval);

    return 1;
}


///String
LUA_API int drl_read_str(lua_State* L)
{

    ByteArray* ptr = (ByteArray*)lua_touserdata(L, 1);
    size_t l = ptr->size;

    lua_Integer index = posrelat(luaL_checkinteger(L, 2), l);

    lua_Integer length = luaL_checkinteger(L, 3);
    luaL_argcheck(L, index + length <= ptr->size, 3, "getting index out of ByteArray range");

    unsigned char* buff = (unsigned char*)malloc(length + 1);
    int j = __read_bytes(ptr, buff, TO_C_INDEX(index), length);
    if (j > 0)
    {
        for (int i = 0; i < j; i++)
        {
            buff[i] = uchar(buff[i]);
        }

        lua_pushlstring(L, buff, length);
        free(buff);
        return 1;
    }
    free(buff);
    return 0;
}



static const luaL_Reg libs[] = {
    {"setb", l_array_set_byte},
    {"getb", l_array_get_byte},
    {"setbytes", l_array_set_bytes},
    {"getbytes", l_array_get_bytes},
    {"setstr", l_array_set_str},
    {"getstr", l_array_get_str},
    {"size", l_array_size},
    {"new", l_array_new},

    {"readu8", drl_read_uint8},
    {"readu16", drl_read_uint16},
    {"readu32", drl_read_uint32},

    {"readi8", drl_read_int8},
    {"readi16", drl_read_int16},
    {"readi32", drl_read_int32},

    {"readd32", drl_read_float},
    {"readd64", drl_read_double},


    { NULL, NULL }
};

static const luaL_Reg meta[] = {
    //{"__index", l_get_array},
    //{"__tostring", array2string},
    { NULL, NULL }
};

LUA_API int luaopen_bytearray_0(lua_State* L)
{
    luaL_newlib(L, libs);
    lua_setglobal(L, DL_ARRAY_NAME);

    return 1;
}


static void set_methods(lua_State* L,
    const char* metatablename,
    const struct luaL_Reg* methods) {
    luaL_getmetatable(L, metatablename);   // mt
    // No need for a __index table since everything is __*
    for (; methods->name; methods++) {
        lua_pushstring(L, methods->name);    // mt, "name"
        lua_pushcfunction(L, methods->func); // mt, "name", func
        lua_rawset(L, -3);                   // mt
    }
    lua_pop(L, 1);
}

LUA_API int luaopen_bytearray_1(lua_State* L)
{

    /*
        luaL_newlib(L, funcs);
    luaL_newmetatable(L, LONG_NUM_TYPE);
    lua_pop(L, 1);
    set_methods(L, LONG_NUM_TYPE, methods);
    lua_setglobal(L, "liblualongnumber");
    */

    luaL_newlib(L, libs);

    luaL_newmetatable(L, DL_ARRAY_META);
    lua_pop(L, 1);
    set_methods(L, DL_ARRAY_META, meta);

    lua_setglobal(L, DL_ARRAY_NAME);

    /*
    luaL_newmetatable(L, "drl.array");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3); //metatable.__index == metatable
    */

    return 1;
}


LUA_API int luaopen_bytearray_2(lua_State* L)
{
    if (luaL_newmetatable(L, DL_ARRAY_META)) {
        luaL_setfuncs(L, meta, 0);
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
    luaL_newlib(L, libs);
    lua_setglobal(L, DL_ARRAY_NAME);
    return 1;
}

LUA_API int drl_test_userdata()
{
    return 0;
}


LUA_API int drl_set_userdata_alloc(USER_DATA_ALLOC callback)
{
    _callback = callback;
    return 0;
}

LUA_API int drl_userdata_copy(uint8_t* src, uint8_t* dst, int length)
{
    ByteArray* sbuf = (ByteArray*)src;
    DR_ASSERT(sbuf->size >= length, "drl_userdata_copy src buff");

    ByteArray* dbuf = (ByteArray*)dst;
    DR_ASSERT(dbuf->size > sbuf->size, "drl_userdata_copy dst buff ");

    memcpy(dbuf->values, sbuf->values, length);
    return 0;
}

LUA_API int drl_userdata_write(uint8_t* src, int soffset, uint8_t* dst, int doffset, int length)
{
    ByteArray* sbuf = (ByteArray*)src;
    DR_ASSERT(sbuf->size >= length, "drl_userdata_copy src buff");

    ByteArray* dbuf = (ByteArray*)dst;
    DR_ASSERT(dbuf->size > sbuf->size, "drl_userdata_copy dst buff ");

    memcpy(dbuf->values, sbuf->values, length);
    return 0;
}

//#define TEST_CODE 1

#if TEST_CODE1
// comment by marco
void test_memview(int n)
{
    size_t nbytes = sizeof(ByteArray) + (n - 1) * sizeof(ByteArray);

    void* m = malloc(nbytes);
    memset(m, 0, nbytes);
    ByteArray* ptr = (ByteArray*)m;

    for (int i = 0; i < n; i++)
    {
        ptr->values[i] = ((i + 1) & 0x7f);
        ptr->size = i + 1;
        printf("--%d", (ptr->values[i]));
    }

    printf("--%d", ptr->size);
}

void GetError(lua_State* L, int status)
{
    if (status != LUA_OK) {
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // pop error message
    }
}

void testud(lua_State* L)
{
    int status, result, i;

    /* Load the file containing the script we are going to run */
    status = luaL_dostring(L, "print 'hello from lua~'"
        "local t = dlarray.new(100)"
        "dlarray.setb(t, 1, 65)"
        "dlarray.setstr(t, 'ABCEFFGHIJKL',  2)"
        "print 'hello again'"
        "print(dlarray.getb(t,2))"
        "print(dlarray.getstr(t, 2, 3))"

    );

    printf("Script returned: %d\n", status);
    GetError(L, status);

    status = luaL_loadfile(L, "D:\\temp\\MPUnpack2.lua");
    GetError(L, status);


    status = luaL_dostring(L, "t2 = dlarray.new(100)"
        "            local messageUDUnpack = require('MPUnpack2')"
        "udc =  messageUDUnpack.unpackCreateUDCursor(t2)"
        "udc.i = 1"
        "udc.j = 41"
    );
    GetError(L, status);

    printf("Script returned: %d\n", status);
    if (status != LUA_OK) {
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // pop error message
    }

    ByteArray* ptr = (ByteArray*)_lastest_ptr;

    int READ_LEN = 41;

    FILE* fp = fopen("D:\\temp\\_mp_dmup_15", "rb");
    char buff[60] = { 0 };
    fread(buff, 1, READ_LEN, fp);

    for (i = 1; i <= READ_LEN; i++)
        ptr->values[TO_C_INDEX(i)] = uchar(buff[i - 1]);

    status = luaL_dostring(L, "local messagePack = require('MessagePack')"
        "local inStr = dlarray.getstr(t2, 1, 41)"
        "local old = messagePack.unpackWithLen(inStr, 41)"
        "print('---C---'..tostring(old['C']))"
        "print('---D---'..tostring(old['D']))"
        "print('---AA---'..tostring(old['AA']))"
        "print('------'..tostring(old['E']))"

    );
    printf("Script returned: %d\n", status);
    GetError(L, status);

    status = luaL_dostring(L, "print('---------------mark2--------------')"
        "local messageUDUnpack = require('MPUnpack2')"
        "local data = messageUDUnpack.unpackWithUDCursor(udc)"
        "print('--->>>-'..tostring(data['D']))"
        "print('------'..tostring(data['C']))"
        "print('------'..tostring(data['D']))"
        "print('------'..tostring(data['AA']))"
        "print('------'..tostring(data['E']))"
    );
    printf("Script returned: %d\n", status);
    GetError(L, status);


}

void TestCMSGPACK(lua_State* L)
{
    int status, result, i;


    status = luaL_loadfile(L, "D:\\temp\\MPUnpack2.lua");
    GetError(L, status);


    status = luaL_dostring(L, "t2 = dlarray.new(2000)");
    GetError(L, status);

    printf("Script returned: %d\n", status);
    if (status != LUA_OK) {
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // pop error message
    }

    ByteArray* ptr = (ByteArray*)_lastest_ptr;

    int READ_LEN = 41;

    FILE* fp = fopen("D:\\_mp_temp_1", "rb");
    char buff[101] = { 0 };
    int BUF_SZ = 100;

    int tIdx = 0;
    while (1)
    {
        int readed = fread(buff, 1, BUF_SZ, fp);
        if (readed <= 0)
        {
            break;
        }

        for (i = 0; i < readed; i++)
            ptr->values[i + tIdx] = uchar(buff[i]);

        tIdx += readed;
    }
    ptr->size = tIdx;

    char strBuf[1024];
    memset(strBuf, 0, 1024);

    //"local cmsgpack = require \"cmsgpack\"";
    const char* luaStr = sprintf(strBuf,
        //" local packStr =  dlarray.getstr(t2, 1, %d) \n"
        " local nt = cmsgpack.unpack_ud(t2, 1, %d) \n"
        " for k, v in pairs(nt) do print (k, '--', v) end",
        tIdx);

    status = luaL_dostring(L, strBuf);
    printf("Script returned: %d\n", status);
    GetError(L, status);

}

int main(int argc, char** argv)
{
    //test_memview(2000);
    printf("hi, this is xlua\n");

    {
        int status, result, i;
        double sum;
        lua_State* L;

        L = luaL_newstate();

        luaL_openlibs(L); /* Load Lua libraries */

        luaopen_bytearray_0(L);

        luaopen_cmsgpack_0(L);

        TestCMSGPACK(L);
        //testud(L);



        lua_pop(L, 1);  /* Take the returned value out of the stack */
        lua_close(L);   /* Cya, Lua */


        getchar();
        return 0;

    }

    return 0;
}
#endif




#pragma warning( pop )