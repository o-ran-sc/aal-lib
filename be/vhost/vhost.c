#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "vhost.h"

int be_init(void** p, void* arg)
{
	uint8_t i;
	struct be_init_arg* initarg = (struct be_init_arg*) arg;

	struct be_init_data* init_data = (struct be_init_data*) malloc(sizeof(struct be_init_data));
	if (init_data == NULL)
		return -1;

	for (i = 0; i < initarg->num; i++)
	{
		if (memcmp(&initarg->arg_name[i], "name", strlen("name")) == 0)
		{
			init_data->name = (char*) initarg->arg[i];
		}
		else if (memcmp(&initarg->arg_name[i], "socket_path", strlen("socket_path")) == 0)
		{
			init_data->socket_path = (char*) initarg->arg[i];
		}
		else if (memcmp(&initarg->arg_name[i], "vm_id", strlen("vm_id")) == 0)
		{
			init_data->vm_id = (uint32_t) atoi((char*) initarg->arg[i]);
		}
	}
	
	*p = (void*) init_data;
	return 0;
}

int be_read(void* p, void** data)
{
	struct be_init_data* init_data = (struct be_init_data*) p;
	if (*data != NULL)
	{
		// clean old data
	}

	//usleep(1000000);

	uint8_t* readdata = "0123456789";
	uint32_t readlen = strlen(readdata);

	*data = readdata;

	uint32_t j;
	uint32_t addr = 0;

	#if 1
	printf("Read from vhost data: \n");
	for (j = 0; j < readlen; j++)
	{
		if ((j & 0xF) == 0)
		{
			if (j != 0)
			{
				printf("\n");
			}
			printf("0x%08x: ", addr);
			addr += 16;
		}
		
		printf("%02x ", readdata[j]);
	}
	printf("\n\n\n");
	#else
	//printf("\n\nRead from '%s' data: %.*s\n", init_data->name, readlen, readdata);
	#endif
	
	return readlen;
}


int be_write(void* p, void* data, size_t data_size)
{
	uint32_t j;
	uint32_t addr = 0;
	char* dp = (char*) data;
	struct be_init_data* init_data = (struct be_init_data*) p;

	#if 1
	printf("Write to vhost data: \n");
	for (j = 0; j < data_size; j++)
	{
		if ((j & 0xF) == 0)
		{
			if (j != 0)
			{
				printf("\n");
			}
			printf("0x%08x: ", addr);
			addr += 16;
		}
		
		printf("%02x ", dp[j]);
	}
	printf("\n\n\n");
	#else
	//printf("Write to '%s' data: %.*s\n", init_data->name, data_size, dp);
	#endif

	return data_size;
}

int be_clean(void* p)
{
	
	return 0;
}


