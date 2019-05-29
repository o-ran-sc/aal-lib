#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "fpga.h"


int plugin_init(void** p, void* arg)
{
	uint8_t i;
	struct plugin_init_arg* initarg = (struct plugin_init_arg*) arg;

	struct plugin_init_data* init_data = (struct plugin_init_data*) malloc(sizeof(struct plugin_init_data));
	if (init_data == NULL)
		return -1;

	for (i = 0; i < initarg->num; i++)
	{
		if (memcmp(&initarg->arg_name[i], "name", strlen("name")) == 0)
		{
			init_data->name = (char*) initarg->arg[i];
		}
	}
	
	*p = (void*) init_data;
	return 0;
}

int plugin_read(void* p, void** data)
{
	struct plugin_init_data* init_data = (struct plugin_init_data*) p;
	if (*data != NULL)
	{
		// clean old data
	}

	usleep(1000000);

	uint8_t* readdata = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
	uint32_t readlen = strlen(readdata);

	*data = readdata;

	uint32_t j;
	uint32_t addr = 0;

	#if 1
	printf("Read from FGPA data: \n");
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


int plugin_write(void* p, void* data, size_t data_size)
{
	uint32_t j;
	uint32_t addr = 0;
	char* dp = (char*) data;
	struct plugin_init_data* init_data = (struct plugin_init_data*) p;

	#if 1
	printf("Write to '%s' data[%u]: \n", init_data->name, data_size);
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
	printf("Write to '%s' data[%u]: %.*s\n", init_data->name, data_size, data_size, dp);
	#endif

	return data_size;
}

int plugin_clean(void* p)
{
	return 0;
}

