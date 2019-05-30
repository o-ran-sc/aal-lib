#ifndef _FPGA_PLUGIN_H_
#define _FPGA_PLUGIN_H_

#define AAL_PLUGIN_INIT_ARG_MAX_CNT		8
#define AAL_NAME_STRING_MAX_LEN 		32
#define AAL_ARG_STRING_MAX_LEN 			256

struct plugin_init_arg
{
	uint8_t num;
	char arg_name[AAL_PLUGIN_INIT_ARG_MAX_CNT][AAL_NAME_STRING_MAX_LEN];
	uint8_t arg[AAL_PLUGIN_INIT_ARG_MAX_CNT][AAL_ARG_STRING_MAX_LEN];
};


struct plugin_init_data
{
	char* name;
};

#endif
