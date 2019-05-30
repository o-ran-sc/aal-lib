#ifndef _VHOST_H_
#define _VHOST_H_

#define AAL_BE_INIT_ARG_MAX_CNT			8
#define AAL_NAME_STRING_MAX_LEN 		32
#define AAL_ARG_STRING_MAX_LEN 			256

struct be_init_arg
{
	uint8_t num;
	char arg_name[AAL_BE_INIT_ARG_MAX_CNT][AAL_NAME_STRING_MAX_LEN];
	uint8_t arg[AAL_BE_INIT_ARG_MAX_CNT][AAL_ARG_STRING_MAX_LEN];
};

struct be_init_data
{
	uint32_t vm_id;
	char* name;
	char* socket_path;
};

#endif
