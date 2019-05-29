#ifndef _MAIN_H_
#define _MAIN_H_

#define AAL_NAME_STRING_MAX_LEN 		32
#define AAL_PATH_STRING_MAX_LEN 		256
#define AAL_ARG_STRING_MAX_LEN 			256
#define AAL_CONDITION_STRING_MAX_LEN 	256

#define AAL_BE_TYPE_MAX_NUM				32
#define AAL_PLUGIN_TYPE_MAX_NUM			32

#define AAL_BE_DEV_MAX_NUM				128
#define AAL_PLUGIN_DEV_MAX_NUM			128

#define AAL_BE_TO_PLUGIN_MAX_NUM		128
#define AAL_PLUGIN_TO_BE_MAX_NUM		128

#define AAL_BE_INIT_ARG_MAX_CNT			8
#define AAL_PLUGIN_INIT_ARG_MAX_CNT		8

#define AAL_BE_TO_PLUGIN_MAX_CNT		8
#define AAL_PLUGIN_TO_BE_MAX_CNT		8

#define AAL_CONFIG_XML_PATH				"./AAL2.conf"

enum AAL_RETURN_NUM
{
	AAL_RET_OK = 0,
	AAL_RET_PARAMETER_ERR = -1,
	
};

/*******
* aal config 
*
* be: backend
* p: plugin
* t: type
* d: device
* c: count
* i: index
********/

struct be_func
{
	void* handle;
	int (*init)(void**, void*);
	int (*read)(void*, void**);
	int (*write)(void*, void*, int);
	int (*clean)(void*);
};

struct be_type
{
	char name[AAL_NAME_STRING_MAX_LEN];
	struct be_func func;
};


struct plugin_func
{
	void* handle;
	int (*init)(void**, void*);
	int (*read)(void*, void**);
	int (*write)(void*, void*, int);
	int (*clean)(void*);
};

struct plugin_type
{
	char name[AAL_NAME_STRING_MAX_LEN];
	struct plugin_func func;
};

struct be_init_arg
{
	uint8_t num;
	char arg_name[AAL_BE_INIT_ARG_MAX_CNT][AAL_NAME_STRING_MAX_LEN];
	uint8_t arg[AAL_BE_INIT_ARG_MAX_CNT][AAL_ARG_STRING_MAX_LEN];
};

struct be_statistics
{
	// data number
	uint64_t read_num;
	uint64_t write_num;

	// data bytes
	uint64_t read_bytes;
	uint64_t write_bytes;

	// Others
};

struct be_dev
{
	uint8_t beti; // backend type index
	char name[AAL_NAME_STRING_MAX_LEN];
	struct be_init_arg beia; // backend init arg
	void* beird; // backend init return data
	struct be_statistics statistics;
	struct be_statistics last_second;
};

struct plugin_init_arg
{
	uint8_t num;
	char arg_name[AAL_PLUGIN_INIT_ARG_MAX_CNT][AAL_NAME_STRING_MAX_LEN];
	uint8_t arg[AAL_PLUGIN_INIT_ARG_MAX_CNT][AAL_ARG_STRING_MAX_LEN];
};

struct plugin_statistics
{
	// data number
	uint64_t read_num;
	uint64_t write_num;

	// data bytes
	uint64_t read_bytes;
	uint64_t write_bytes;

	// Others
};

struct plugin_dev
{
	uint8_t pti; // plugin type index
	char name[AAL_NAME_STRING_MAX_LEN];
	struct plugin_init_arg pia; // plugin init arg
	void* pird; // plugin init return data
	struct plugin_statistics statistics;
	struct plugin_statistics last_second;
};


// 1 backend send data to n plugin
struct be2plugin
{
	uint8_t bedi;   //backend dev index
	uint8_t pdic;   // plugin dev index count
	uint8_t dst;    // data send type   0.clone data to all plugin   1.conditional distribution
	uint8_t pdi[AAL_BE_TO_PLUGIN_MAX_CNT];  //plugin dev index
	uint8_t cdr[AAL_BE_TO_PLUGIN_MAX_CNT][AAL_CONDITION_STRING_MAX_LEN]; // dst==1, conditional distribution regular
};


// 1 plugin send data to n backend
struct plugin2be
{
	uint8_t pdi;   //plugin dev index
	uint8_t bedic; // backend dev index count
	uint8_t dst;   // data send type   0.clone data to all plugin   1.conditional distribution
	uint8_t bedi[AAL_PLUGIN_TO_BE_MAX_CNT];  //backend dev index
	uint8_t cdr[AAL_BE_TO_PLUGIN_MAX_CNT][AAL_CONDITION_STRING_MAX_LEN]; // dst==1, conditional distribution regular
};

struct aal_config
{	
	uint32_t betc;	                         // backend type count
	struct be_type bet[AAL_BE_TYPE_MAX_NUM]; // backend type 
	
	uint32_t ptc;                                   // plugin type count
	struct plugin_type pt[AAL_PLUGIN_TYPE_MAX_NUM]; // plugin type
	
	uint32_t bedc;                           // backend device count
	struct be_dev  bed[AAL_BE_DEV_MAX_NUM];  // backend device 
	
	uint32_t pdc;                                 // plugin device count
	struct plugin_dev pd[AAL_PLUGIN_DEV_MAX_NUM]; // plugin device
	
	uint32_t be2pc;                                // backend to plugin count
	struct be2plugin be2p[AAL_BE_TO_PLUGIN_MAX_NUM];  // backend to plugin 
	
	uint32_t p2bec;                                // plugin to backend count
	struct plugin2be p2be[AAL_PLUGIN_TO_BE_MAX_NUM];  // plugin to backend
};

struct aal_be2p_thread_arg
{
	uint32_t be2pi;   // backend to plugin index
	struct aal_config* conf;
};

struct aal_p2be_thread_arg
{
	uint32_t p2bei;   // plugin to backend  index
	struct aal_config* conf;
};

#endif

