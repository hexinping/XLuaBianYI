#ifndef DR_LUA_____H
#define DR_LUA_____H
#include "lua.h"
#include "stdint.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct ByteArray {
    int size; //first int size header
    int8_t values[1];
} ByteArray;

#define TO_C_INDEX(P) P - 1
int lua_on_alloc_mem(int size);
int lua_on_free_mem(int size);

typedef void (*USER_DATA_ALLOC)(uint8_t* buff, int length, int header);

typedef void (*LUA_MEM_CALLBACK)(void* ptr, int memType, int count);
LUA_MEM_CALLBACK lua_get_mem_callback();


typedef void (*ON_REUSE_CB)(uint8_t* arg1, int arg1cnt, uint8_t* arg2, int arg2cnt);

typedef void (*ON_NEW_CB)(uint16_t classIndex, uint16_t fieldIndex, uint8_t* str, int strSize);
//void drl_set_userdata_alloc(USER_DATA_ALLOC callback);

//void drl_userdata_copy(uint8_t* src, uint8_t* dst, int length);

LUA_API int luaopen_cmsgpack(lua_State* L);

LUA_API int luaopen_cmsgdl(lua_State* L);

void DR_ASSERT(int code, const char* fmt);

int json_encode_to_cb (lua_State* L, int opt, int Index, uint8_t* arg2, int arg2cnt, ON_REUSE_CB cb);
int json_encode_to_str(lua_State* L, int Index, uint8_t** buffer, int* bufferSize, int* realSize);

typedef enum  E_Statistics
{
    kSTCNewTableCount = 0,
    kSTCReuseTableCount = 1,
    kSTCReuseFailCount,
    kSTCSkipByteCount,
    kSTCEventCount,
    kSTCMax,
}StatisticType;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */ 


#endif
