#ifndef _QUEUE_H_
#define _QUEUE_H_


enum que_return
{
	QUE_RET_OK = 0,
	QUE_RET_ARG_ERR,
	QUE_RET_ALLOC_ERR,
	QUE_RET_LOCK_ERR,
	QUE_RET_EMPTY,
	QUE_RET_FULL,
};

#define QUE_LOCK_LOOP_USLEEP   100
#define QUE_LOCK_LOOP_MAX      5

#define QUE_PUSH_LOCK_INIT(p)    ((p)->pushlock = 0)
#define QUE_PUSH_UNLOCK(p)   	((p)->pushlock = 0)
#define QUE_PUSH_LOCK(p) \
({ \
	int ret = QUE_RET_OK; \
	int defi; \
	for (defi = 0; defi < QUE_LOCK_LOOP_MAX; defi++) \
	{ \
		if (__sync_bool_compare_and_swap(&((p)->pushlock), 0, 1) != 0) \
		{ \
			break; \
		} \
		usleep(QUE_LOCK_LOOP_USLEEP); \
	} \
	if (defi == QUE_LOCK_LOOP_MAX) \
	{ \
		ret = QUE_RET_LOCK_ERR; \
	} \
	ret; \
})

#define QUE_PULL_LOCK_INIT(p)    ((p)->pulllock = 0)
#define QUE_PULL_UNLOCK(p)   	((p)->pulllock = 0)
#define QUE_PULL_LOCK(p) \
({ \
	int ret = QUE_RET_OK; \
	int defi; \
	for (defi = 0; defi < QUE_LOCK_LOOP_MAX; defi++) \
	{ \
		if (__sync_bool_compare_and_swap(&((p)->pulllock), 0, 1) != 0) \
		{ \
			break; \
		} \
		usleep(QUE_LOCK_LOOP_USLEEP); \
	} \
	if (defi == QUE_LOCK_LOOP_MAX) \
	{ \
		ret = QUE_RET_LOCK_ERR; \
	} \
	ret; \
})


typedef struct que_data
{
	uint8_t  pushlock;
	uint8_t  pulllock;
	uint32_t size;
	uint32_t head;
	uint32_t tail;
	void**   queue;

	// print info
	uint32_t push_cnt;
	uint32_t pull_cnt;
}queue_data_t;


int queue_init(queue_data_t* p, uint32_t size);
int queue_push(queue_data_t* p, void* data);
int queue_pull(queue_data_t* p, void** data);
int queue_clean(queue_data_t* p);

#endif
