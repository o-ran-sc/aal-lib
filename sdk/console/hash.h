#ifndef _HASH_H_
#define _HASH_H_

#define HASH_FLAG_DATA_COPY   0x1

enum hash_ret_num
{
	HASH_RET_OK = 0,
	HASH_RET_ARG_ERR = -1,
	HASH_RET_ALLOC_ERR = -2,
	HASH_RET_DATA_EXIST = -3,
	HASH_RET_DATA_NONEXIST = -4,
	HASH_RET_SPACE_NOT_ENOUGH = -5,
	HASH_RET_LOCK_ERR = -6,
};

typedef uint32_t (*get_hash)(void*, uint32_t);
typedef uint32_t (*free_data)(void*);


typedef struct hash_data
{
	//uint8_t  wlock;
	//uint32_t rlock;

	void* key;
	void* data;
	size_t key_size;
	size_t data_size;

	struct hash_data* next;
}hash_data_t;

#define HASH_LOCK_LOOP_USLEEP   100
#define HASH_LOCK_LOOP_MAX      5


#define HASH_LOCK_INIT(p, index)    ((p)->lock[index] = 0)
#define HASH_UNLOCK(p, index)   	((p)->lock[index] = 0)
#define HASH_LOCK(p, index) \
({ \
	int ret = HASH_RET_OK; \
	int defi; \
	for (defi = 0; defi < HASH_LOCK_LOOP_MAX; defi++) \
	{ \
		if (__sync_bool_compare_and_swap(&((p)->lock[index]), 0, 1) != 0) \
		{ \
			break; \
		} \
		usleep(HASH_LOCK_LOOP_USLEEP); \
	} \
	if (defi == HASH_LOCK_LOOP_MAX) \
	{ \
		ret = HASH_RET_LOCK_ERR; \
	} \
	ret; \
})



typedef struct 
{
	uint32_t size;
	uint32_t flag;
	uint32_t (*hash)(void*, uint32_t);
	uint32_t (*free)(void*);

	uint8_t*  lock;
	hash_data_t** data;
}hash_t;

int32_t hash_init(hash_t* p, uint32_t size, uint32_t (*func)(void*, size_t));
int32_t hash_add(hash_t* p, void* key, size_t key_size, void* data, size_t data_size);
int32_t hash_del(hash_t* p, void* key, size_t key_size);
int32_t hash_search(hash_t* p, void* key, size_t key_size, void* data, size_t data_size);
int32_t hash_clean(hash_t* p);


#endif
