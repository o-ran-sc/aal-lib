/*

2018/8/27 liusw4 create
*/

#ifndef _VIRTCONSOLEBE_H_
#define _VIRTCONSOLEBE_H_

#define PRINTLOG printf
#define ERROR -1
#define SUCESS 0
#include <unistd.h>  
#include <stdint.h>

#define SOCKETCONNRETRYTIMES 128
//if get error,sleep 100ms
#define ERRORSLEEPTIME 100000


#define AAL_BE_INIT_ARG_MAX_CNT		8
#define AAL_NAME_STRING_MAX_LEN 	32
#define AAL_ARG_STRING_MAX_LEN 	256

typedef struct be_init_arg
{
	uint8_t num;
	char arg_name[AAL_BE_INIT_ARG_MAX_CNT][AAL_NAME_STRING_MAX_LEN];
	uint8_t arg[AAL_BE_INIT_ARG_MAX_CNT][AAL_ARG_STRING_MAX_LEN];
}be_init_arg_t;


//socket info 
typedef struct socketInfo
{
   char socketPath[AAL_ARG_STRING_MAX_LEN];
   char socketFd;
   char *bufData;
}socketInfo_t;


//standard external interface
int be_init(void **Out, void *arg);
int be_read(void* p, void** data);
int be_write(void *p, void *data, size_t iLen);
int be_clean(void *InOut);




#endif

