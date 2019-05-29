#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#include "hash.h"

uint32_t hash_get_size(uint32_t size)
{	
	uint8_t max_index = 0;
	uint8_t count = 0;
	uint32_t s = size;
	while (s != 0)
	{
		if (s & 1)
			count++;

		s = s >> 1;
		max_index++;
	}

	if (max_index == 0)
		return 0;

	return count > 1 ? 1 << max_index : size;
}

uint32_t hash_default_get_hash(void* key, size_t key_size)
{
	uint32_t i;
	uint32_t num = 0;
	for (i = 0; i < key_size; i++)
		num += (((uint8_t*)key)[i] << ((i % 4) * 8)) + ((uint8_t*)key)[i];

	return num;
}

int32_t hash_init(hash_t* p, uint32_t size, uint32_t (*hash)(void*, size_t), uint32_t flag, uint32_t (*free)(void*))
{
	uint32_t i;

	p->size = hash_get_size(size);
	if (p->size == 0)
		return HASH_RET_ARG_ERR;


	p->lock = (hash_data_t**) malloc(sizeof(uint8_t) * p->size);
	if (p->lock == NULL)
		return HASH_RET_ALLOC_ERR;

	for (i = 0; i < p->size; i++)
		HASH_LOCK_INIT(p, i);
	
	p->data = (hash_data_t**) malloc(sizeof(hash_data_t*) * p->size);
	if (p->data == NULL)
	{
		free(p->lock);
		return HASH_RET_ALLOC_ERR;
	}
	
	memset(p->data, 0, sizeof(hash_data_t*) * p->size);
	
	//
	if (hash == NULL)
		p->hash = (get_hash) hash_default_get_hash;
	else
		p->hash = (get_hash) hash;

	//
	p->flag = flag;
	if (p->flag & HASH_FLAG_DATA_COPY == 0)
		p->free = (free_data) free;
	else
		p->free = NULL;

	return HASH_RET_OK;	
}

int32_t hash_add(hash_t* p, void* key, size_t key_size, void* data, size_t data_size)
{
	uint32_t ret = HASH_RET_OK;
	uint32_t hash_index = p->func(key, key_size) % p->size;

	if ((ret = HASH_LOCK(p, hash_index)) != QUE_RET_OK)
		goto lock_err;
	
	hash_data_t** head = &(p->data[hash_index]);

	hash_data_t* node = *head;
	while(node != NULL)
	{
		if (node->key_size == key_size && memcmp(node->key, key, key_size) == 0)
			break;

		node = node->next;
	}

	if (node != NULL)
	{
		ret = HASH_RET_DATA_EXIST;
		goto data_exist;
	}


	node = (hash_data_t*) malloc(sizeof(hash_data_t));
	if (node == NULL)
	{
		ret = HASH_RET_ALLOC_ERR;
		got alloc_err;
	}

	memset(node, 0, sizeof(hash_data_t));

	node->key = (void*) malloc(key_size);
	if (node->key == NULL)
	{
		ret = HASH_RET_ALLOC_ERR;
		got alloc_err;
	}

	memcpy(node->key, key, key_size);
	node->key_size = key_size;

	//
	if (p->flag & HASH_FLAG_DATA_COPY > 0)
	{
		node->data = (void*) malloc(data_size);
		if (node->data == NULL)
		{
			ret = HASH_RET_ALLOC_ERR;
			got alloc_err;
		}
		
		memcpy(node->data, data, data_size);
		node->data_size = data_size;
	}
	else
	{
		node->data = data;
		node->data_size = data_size;
	}


	node->next = *head;
	*head = node;

	HASH_UNLOCK(p, hash_index);

	return HASH_RET_OK;

alloc_err:

	if (node != NULL)
	{
		if (node->key != NULL)
			free(node->key);

		if (node->data != NULL)
			free(node->data)

		free(node);
	}
	

data_exist:

	HASH_UNLOCK(p, hash_index);

lock_err:

	return ret;
}

int32_t hash_del(hash_t* p, void* key, size_t key_size)
{
	uint32_t ret = HASH_RET_OK;
	uint32_t hash_index = p->func(key, key_size) % p->size;
	
	if (HASH_LOCK(p, hash_index) != QUE_RET_OK)
		goto lock_err;

	hash_data_t** head = &(p->data[hash_index]);
	if (*head == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	hash_data_t* last = NULL;
	hash_data_t* node = *head;
	while(node != NULL)
	{
		if (node->key_size == key_size && memcmp(node->key, key, key_size) == 0)
		{
			if (last == NULL)
				*head = node->next;
			else
				last->next = node->next;
			
			node->next = NULL;

			break;
		}

		last = node;
		node = node->next;
	}

	if (node == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	if (node->key != NULL)
	{
		free(node->key);
	}
	
	if (node->data != NULL)
	{
		if (p->flag & HASH_FLAG_DATA_COPY == 0)
			if (p->free != NULL)
				p->free(node->data);
		else
			free(node->data);
	}

	free(node);
	
	HASH_UNLOCK(p, hash_index);

	return HASH_RET_OK;
	
data_nonexist:

	HASH_UNLOCK(p, hash_index);

lock_err:

	return ret;
}

int32_t hash_search(hash_t* p, void* key, size_t key_size, void** data)
{
	uint32_t ret = HASH_RET_OK;
	uint32_t hash_index = p->func(key, key_size) % p->size;
	
	if (HASH_LOCK(p, hash_index) != QUE_RET_OK)
		goto lock_err;
	
	hash_data_t* node = p->data[hash_index];
	while(node != NULL)
	{
		if (node->key_size == key_size && memcmp(node->key, key, key_size) == 0)
			break;

		node = node->next;
	}

	if (node == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	*data = node->data;
	
	HASH_UNLOCK(p, hash_index);

	return node->data_size;

	
data_nonexist:

	HASH_UNLOCK(p, hash_index);

lock_err:

	return ret;
}

int32_t hash_search_copy(hash_t* p, void* key, size_t key_size, void* data, size_t data_size)
{
	uint32_t ret = HASH_RET_OK;
	uint32_t hash_index = p->func(key, key_size) % p->size;
	
	if (HASH_LOCK(p, hash_index) != QUE_RET_OK)
		return HASH_RET_LOCK_ERR;
	
	hash_data_t* node = p->data[hash_index];
	while(node != NULL)
	{
		if (node->key_size == key_size && memcmp(node->key, key, key_size) == 0)
			break;

		node = node->next;
	}

	if (node == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	if (node->data_size > data_size)
	{
		ret = HASH_RET_SPACE_NOT_ENOUGH;
		goto space_not_enough;
	}

	memcpy(data, node->data, node->data_size);
	
	HASH_UNLOCK(p, hash_index);

	return node->data_size;
	
space_not_enough:
	
data_nonexist:

	HASH_UNLOCK(p, hash_index);

lock_err:

	return ret;
}


int32_t hash_search_del(hash_t* p, void* key, size_t key_size, void** data)
{
	uint32_t ret = HASH_RET_OK;
	uint32_t hash_index = p->func(key, key_size) % p->size;
	
	if (HASH_LOCK(p, hash_index) != QUE_RET_OK)
		goto lock_err;

	hash_data_t** head = &(p->data[hash_index]);
	uint32_t data_len;
	if (*head == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	hash_data_t* last = NULL;
	hash_data_t* node = *head;
	while(node != NULL)
	{
		if (node->key_size == key_size && memcmp(node->key, key, key_size) == 0)
		{
			if (last == NULL)
				*head = node->next;
			else
				last->next = node->next;
			
			node->next = NULL;

			break;
		}

		last = node;
		node = node->next;
	}

	if (node == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	if (node->key != NULL)
	{
		free(node->key);
	}
	
	*data = node->data);
	data_len = node->data;

	free(node);
	
	HASH_UNLOCK(p, hash_index);

	return data_len;
	
data_nonexist:

	HASH_UNLOCK(p, hash_index);

lock_err:

	return ret;
}



int32_t hash_search_del_copy(hash_t* p, void* key, size_t key_size, void* data, size_t data_size)
{
	uint32_t ret = HASH_RET_OK;
	uint32_t hash_index = p->func(key, key_size) % p->size;
	
	if (HASH_LOCK(p, hash_index) != QUE_RET_OK)
		goto lock_err;

	hash_data_t** head = &(p->data[hash_index]);
	if (*head == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	hash_data_t* last = NULL;
	hash_data_t* node = *head;
	while(node != NULL)
	{
		if (node->key_size == key_size && memcmp(node->key, key, key_size) == 0)
		{
			if (last == NULL)
				*head = node->next;
			else
				last->next = node->next;
			
			node->next = NULL;

			break;
		}

		last = node;
		node = node->next;
	}

	if (node == NULL)
	{
		ret = HASH_RET_DATA_NONEXIST;
		goto data_nonexist;
	}

	if (node->key != NULL)
	{
		free(node->key);
	}
	
	if (node->data != NULL)
	{
		free(node->data);
	}

	free(node);
	
	HASH_UNLOCK(p, hash_index);

	return HASH_RET_OK;
	
data_nonexist:

	HASH_UNLOCK(p, hash_index);

lock_err:

	return ret;
}

int32_t hash_clean(hash_t* p)
{
	uint32_t i;

	for (i = 0; i < p->size; i++)
	{
		if (HASH_LOCK(p, i) != QUE_RET_OK)
			continue;
		
		hash_data_t* node = NULL;
		hash_data_t** head = &(p->data[i]);
		while(*head != NULL)
		{
			node = *head;
			
			*head = node->next;

			// clean node
			if (node->key != NULL)
			{
				free(node->key);
			}
			
			if (node->data != NULL)
			{
				if (p->flag & HASH_FLAG_DATA_COPY == 0)
					if (p->free != NULL)
						p->free(node->data);
				else
					free(node->data);
			}
			
			free(node);
			
		}	
		
		HASH_UNLOCK(p, i);
	}

	memset(p->data, 0, sizeof(hash_data_t*) * p->size);

	return HASH_RET_OK;
}


#if 0
int main()
{
	hash_t hash;
	hash_init(&hash, 3, NULL);
	printf("add 0 ret = %d\n", hash_add(&hash, (void*) "0", 1, (void*) "000", 3));
	printf("add 1 ret = %d\n", hash_add(&hash, (void*) "1", 1, (void*) "111", 3));
	printf("add 4 ret = %d\n", hash_add(&hash, (void*) "4", 1, (void*) "444", 3));
	printf("add 5 ret = %d\n", hash_add(&hash, (void*) "5", 1, (void*) "555", 3));
	printf("add 6 ret = %d\n", hash_add(&hash, (void*) "6", 1, (void*) "666", 3));
	printf("add 7 ret = %d\n", hash_add(&hash, (void*) "7", 1, (void*) "777", 3));
	printf("add 9 ret = %d\n", hash_add(&hash, (void*) "8", 1, (void*) "888", 3));

	char data[128*1024*4];
	int datasize = 128*1024*4;
	int datalen = 0;
	datalen = hash_search(&hash, (void*) "9", 1, data, datasize);
	printf("9 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "8", 1, data, datasize);
	printf("8 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "7", 1, data, datasize);
	printf("7 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "6", 1, data, datasize);
	printf("6 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "5", 1, data, datasize);
	printf("5 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "4", 1, data, datasize);
	printf("4 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "3", 1, data, datasize);
	printf("3 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "2", 1, data, datasize);
	printf("2 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "1", 1, data, datasize);
	printf("1 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "0", 1, data, datasize);
	printf("0 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	printf("\n");

	
	printf("del 8 ret = %d\n", hash_del(&hash, (void*) "8", 1));
	printf("del 6 ret = %d\n", hash_del(&hash, (void*) "6", 1));
	printf("del 5 ret = %d\n", hash_del(&hash, (void*) "5", 1));
	printf("del 1 ret = %d\n", hash_del(&hash, (void*) "1", 1));
	
	printf("add 2 ret = %d\n", hash_add(&hash, (void*) "2", 1, (void*) "222", 3));
	printf("add 3 ret = %d\n", hash_add(&hash, (void*) "3", 1, (void*) "333", 3));
	printf("add 9 ret = %d\n", hash_add(&hash, (void*) "9", 1, (void*) "999", 3));
	
	datalen = hash_search(&hash, (void*) "9", 1, data, datasize);
	printf("9 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "8", 1, data, datasize);
	printf("8 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "7", 1, data, datasize);
	printf("7 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "6", 1, data, datasize);
	printf("6 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "5", 1, data, datasize);
	printf("5 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "4", 1, data, datasize);
	printf("4 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "3", 1, data, datasize);
	printf("3 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "2", 1, data, datasize);
	printf("2 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "1", 1, data, datasize);
	printf("1 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "0", 1, data, datasize);
	printf("0 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	printf("\n");

	
	printf("add 5 ret = %d\n", hash_add(&hash, (void*) "5", 1, (void*) "555", 3));
	printf("add 6 ret = %d\n", hash_add(&hash, (void*) "6", 1, (void*) "666", 3));

	printf("del 2 ret = %d\n", hash_del(&hash, (void*) "2", 1));
	printf("del 4 ret = %d\n", hash_del(&hash, (void*) "4", 1));
	printf("del 3 ret = %d\n", hash_del(&hash, (void*) "3", 1));
	printf("del 7 ret = %d\n", hash_del(&hash, (void*) "7", 1));
	printf("del 9 ret = %d\n", hash_del(&hash, (void*) "9", 1));
	
	datalen = hash_search(&hash, (void*) "9", 1, data, datalen);
	printf("9 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "8", 1, data, datalen);
	printf("8 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "7", 1, data, datalen);
	printf("7 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "6", 1, data, datalen);
	printf("6 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "5", 1, data, datalen);
	printf("5 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "4", 1, data, datalen);
	printf("4 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "3", 1, data, datalen);
	printf("3 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "2", 1, data, datalen);
	printf("2 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "1", 1, data, datalen);
	printf("1 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	datalen = hash_search(&hash, (void*) "0", 1, data, datalen);
	printf("0 data[%d] = %.*s\n", datalen, datalen <= 0 ? 0 : datalen, data);
	printf("\n");

	hash_clean(&hash);
	
}
#endif
