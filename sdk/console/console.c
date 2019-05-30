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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "queue.h"
#include "hash.h"
#include "console.h"

struct cons_mth_data g_cons_mth_data = {.fp = NULL};


void* cons_write_thread(void* indata)
{
	struct cons_mth_data* p = &g_cons_mth_data;

	queue_data_t* ring = &(g_cons_mth_data.tx_ring);
	struct cons_send_data* data;

	while(1)
	{
		if (queue_pull(ring, (void**) &data) != QUE_RET_OK)
			continue;

		if (fwrite(data->data, sizeof(uint8_t), data->len, p->fp) != data->len)
			continue;

		
	}

	
}

void* cons_read_thread(void* indata)
{
	struct cons_mth_data* p = &g_cons_mth_data;

	uint32_t rl;
	uint32_t data_len;
	uint8_t* data;

	while(1)
	{
		rl = fread(&data_len, sizeof(uint32_t), 1, p->fp);
		if (rl != sizeof(uint32_t))
			continue;

		if (data_len <= sizeof(uint32_t) && data_len > (CONSOLE_RESULT_MAX_SIZE + sizeof(uint32_t)))
			continue;

		data = (uint8_t*) malloc(data_len);
		if (data == NULL)
			continue;

		rl = fread(data, sizeof(uint8_t), data_len, p->fp);
		if (rl != data_len)
		{
			free(data);
			continue;
		}

		uint32_t* dataid = (uint32_t*) data;
		if (hash_add(&p->rx_hash, dataid, sizeof(uint32_t), data, data_len) != HASH_RET_OK)
			continue;
		
	}

}



// multithreading api
int cons_init() // thread_num: 0~256, 0: default
{
	struct cons_mth_data* p = &g_cons_mth_data;

	uint32_t i;
	uint32_t ret = 0;	

	p->fp = fopen(CONSOLE_PATH, "a+");
	if (p->fp == NULL)
	{
		ret = -1;
		goto cons_open_err;
	}

	if (queue_init(&p->tx_ring, CONSOLE_RX_RING_SIZE) != QUE_RET_OK)
	{
		ret = -2;
		goto queue_init_err;
	}
	
	if (hash_init(&p->rx_hash, CONSOLE_RX_HASH_SIZE, NULL, 0, (free_data) free) != HASH_RET_OK)
	{
		ret = -3;
		goto hash_init_err;
	}

	
	pthread_t pid;
	pthread_create(&pid, NULL, cons_write_thread, NULL);
	pthread_create(&pid, NULL, cons_read_thread, NULL);
	
	return 0;

hash_init_err:

	queue_clean(&p->tx_ring);

queue_init_err:
	
	fclose(p->fp);

cons_open_err:

	return ret;
	
}


//
int cons_send(uint32_t id, uint8_t* data, uint32_t len)
{
	struct cons_mth_data* p = &g_cons_mth_data;
	
	queue_data_t* rx_ring = p->tx_ring;

	struct cons_send_data* ring_data = (struct cons_send_data*) malloc(sizeof(struct cons_send_data));
	if (ring_data == NULL)
	{
		return -1;
	}

	ring_data->len = sizeof(uint32_t) + sizeof(uint32_t) + len;

	ring_data->data = (uint8_t*) malloc(ring_data->len);
	if (ring_data->data == NULL)
	{
		free(ring_data);
		return -1;
	}

	uint32_t* data_len_p = (uint32_t*) ring_data->data;
	uint32_t* data_id_p = (uint32_t*) (data_len_p + 1);
	uint8_t*  user_data_p = (uint8_t*) (data_id_p + 1);

	
	*data_len_p = len + sizeof(uint32_t);
	*data_id_p = id;
	memcpy(user_data_p, data, len);
	
	return queue_push(rx_ring, ring_data) != QUE_RET_OK ? -2 : 0;

}



int cons_read(uint32_t id, uint8_t* data, uint32_t len)
{
	struct cons_mth_data* p = &g_cons_mth_data;

	uint8_t* hash_data;
	uint32_t hash_datalen;
	if ((hash_datalen = hash_search_del(p->rx_hash, &id, sizeof(uint32_t), &hash_data)) > 0)
	{
		return -1;
	}

	if (len < hash_datalen)
		return -2;

	memcpy(data, hash_data, hash_datalen);
	return hash_datalen;
}

cons_clean()
{
	struct cons_mth_data* p = &g_cons_mth_data;

	fclose(p->fp);

	queue_clean(p->tx_ring);
	hash_clean(p->rx_hash);
	
}

