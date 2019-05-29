#ifndef _AAL_SDK_CONSOLE_H_
#define _AAL_SDK_CONSOLE_H_

#define CONSOLE_PATH  "/console"

#define CONSOLE_RX_RING_SIZE		4*1024
#define CONSOLE_RX_HASH_SIZE		4*1024

#define CONSOLE_RESULT_MAX_SIZE		4*1024

struct cons_send_data
{
	int len;
	uint8_t* data;
};



struct cons_mth_data
{
	FILE* fp;

	queue_data_t tx_ring;
	hash_t  rx_hash;
	
};


#endif
