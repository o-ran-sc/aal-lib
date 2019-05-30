/**
* Licensed to the Apache Software Foundation （ASF） under one or more
* contributor license agreements. See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* （the "License"）； you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/



#include <stdlib.h>  
#include <stdio.h>  
#include <stddef.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <errno.h>  
#include <string.h>  

#include "virtconsolebe.h"






static int socketCreateAndConnect(socketInfo_t *socketInfo, void **Out)
{
	struct  sockaddr_un cliun, serun;  
    int len;  
    char buf[2048] = {0};  
    int socketFd, n;  
	int iRetryTimes =0;
	char *socketPath = socketInfo->socketPath;
	char *client_path = "client.socket"; 
    int *pSocket = NULL;
    if ((socketFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{  
        PRINTLOG("client socket error");  
        return ERROR;  
    } 
	memset(&cliun, 0, sizeof(cliun));  
    cliun.sun_family = AF_UNIX;  
    strcpy(cliun.sun_path, client_path); 
#if 0
    len = offsetof(struct sockaddr_un, sun_path) + strlen(cliun.sun_path);  
    unlink(cliun.sun_path);  
    if (bind(socketFd, (struct sockaddr *)&cliun, len) < 0) {  
        perror("bind error");  
        exit(1);  
    }
#endif

	memset(&serun, 0, sizeof(serun));  
    serun.sun_family = AF_UNIX;  
    strcpy(serun.sun_path, socketPath);  
    len = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);  
	while (1)
	{
		//直至连接成功或达到最大重试次数
		PRINTLOG("unix socket connecting...\r\n");
	    if (connect(socketFd, (struct sockaddr *)&serun, len) < 0)
		{  
	        perror("client connect error,retrying\r\n"); 
			PRINTLOG("socket path:%s\r\n", socketPath);
	        iRetryTimes ++;
			//100ms
			usleep(ERRORSLEEPTIME);
			if (iRetryTimes > SOCKETCONNRETRYTIMES)
			{
				PRINTLOG("connect retry max times\r\n");
				return ERROR;
			}
			continue;
	    } 
		//keep socket fd
		socketInfo->socketFd = socketFd;
		PRINTLOG("unix socket cennectted\r\n");
		break;
		
	}
    pSocket = (int *)malloc(sizeof(int));
    if (NULL == pSocket)
    {
        PRINTLOG("no memory\r\n");
		return ERROR;
    }
    *pSocket = socketFd;
    *Out = pSocket;
    //store socket fd 
	return SUCESS;
	
}

//read date from socket 
static int socketRead(int socketFd, void **bufData, int iFlag)
{
	int iDateLen = 0;
	int iReadLen = 0;
	char *bufTmp = NULL;


	//check validity
	if (NULL != *bufData)
	{
		free(*bufData);
	}
	
	//数据格式
	//------------------------------------
	//| length 4B|	  data				 |
	//------------------------------------
	//get data len
	iReadLen = recv(socketFd, (char*)&iDateLen, 4, iFlag);
	if(iReadLen <= 0)
	{
		//连接出错，跳出，重新connect
		PRINTLOG("read socket error,reconnecting...\r\n");
		return ERROR;
	}
	//get data 
	iDateLen = ntohl(iDateLen);

	//超出入参buf大小，暂时先read出来丢弃
	bufTmp = malloc(iDateLen);
	if (NULL == bufTmp)
	{
		PRINTLOG("no memory\r\n");
		return ERROR;
	}
	
	iReadLen = recv(socketFd, bufTmp, iDateLen, iFlag);
	if(iReadLen <= 0)
	{
		//read error
		PRINTLOG("read socket error,reconnecting...\r\n");
		return ERROR;
	}

	*bufData = (void*)bufTmp;
	return iReadLen;

}


//write data to socket 
static int socketWrite(int socketFd, char *bufData, int iLen, int iFlag)
{
	int iWriteLen = 0;
	int iDateLen = htonl(iLen);

	//check validity
	if (NULL == bufData)
	{
		PRINTLOG("out buf is NULL\r\n");
		return ERROR;
	}

	//send data length
	//iWriteLen = send(socketFd, &iDateLen, 4, iFlag);
	if (iWriteLen < 0)
	{
		PRINTLOG("write error\r\n");
		return ERROR;
	}

	iWriteLen = send(socketFd, bufData, iLen, iFlag);
	if (iWriteLen < 0)
	{
		PRINTLOG("write error\r\n");
		return ERROR;
	}
	return iWriteLen;
}

static int socketClose(int *socketFd)
{

    if (NULL == socketFd)
    {
        PRINTLOG("socket fd is NULL\r\n");
        return ERROR;
    }
	close(*socketFd);
    free(socketFd);
    
}





//virtconsole init
int be_init(void **Out, void *arg)

{
	//TODO
	socketInfo_t socketInfo;
    be_init_arg_t *initArg = (be_init_arg_t*)arg;
    int i = 0;
    //get socket path form name
    for (i = 0; i < initArg->num; i++)
    {
        if (0 == strcmp(initArg->arg_name[i], "socketpath"))
        {
            strcpy(socketInfo.socketPath, initArg->arg[i]);
            break;
        }
               
    }
    if (i == initArg->num)
    {
        PRINTLOG("no socket path\r\n");
        return ERROR;
    }
	return socketCreateAndConnect(&socketInfo, Out);

}
int be_clean(void *In)

{
	//TODO
	socketInfo_t *socketInfo = (socketInfo_t*)In;
	return socketClose((int*)In);

}

int be_read(void* p, void** data)
{
	int socketFd = *((int*) p);
    return socketRead(socketFd, data, 0);
}

int be_write(void *p, void *data, size_t iLen)
{
    int socketFd = *((int*) p);
    return socketWrite(socketFd, (char*) data, iLen, 0);

}









