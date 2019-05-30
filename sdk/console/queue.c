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
#include <unistd.h>

#include "queue.h"

/************************************
	head == tail             ==>    no data
	head == tail + n       ==>    n data
	head == tail - 1       ==>    full
************************************/

int queue_init(queue_data_t* p, uint32_t size)
{
	if (size == 0)
		return QUE_RET_ARG_ERR;
	
	p->size = size;
	p->queue = (void**) malloc(sizeof(void*)*size);
	if (p->queue == NULL)
	{
		return QUE_RET_ALLOC_ERR;
	}

	p->head = p->tail = 0;
	QUE_PUSH_LOCK_INIT(p);
	QUE_PULL_LOCK_INIT(p);

	return QUE_RET_OK;
}

int queue_push(queue_data_t* p, void* data)
{
	int ret = QUE_RET_OK;

	if (QUE_PUSH_LOCK(p) != QUE_RET_OK)
		return QUE_RET_LOCK_ERR;
	
	if (p->head == (p->size-1))
	{
		if (p->tail == 0)
		{
			ret = QUE_RET_FULL;
		}
		else
		{
			p->queue[p->head] = data;
			p->head = 0;
			p->push_cnt ++;
		}
	}
	else if ((p->head + 1) == p->tail)
	{
		ret = QUE_RET_FULL;
	}
	else
	{
		p->queue[p->head++] = data;
		p->push_cnt ++;
	}

	QUE_PUSH_UNLOCK(p);
	return ret;
}

int queue_pull(queue_data_t* p, void** data)
{
	int ret = QUE_RET_OK;

	if (QUE_PULL_LOCK(p) != QUE_RET_OK)
		return QUE_RET_LOCK_ERR;

	//
	if (p->head == p->tail)
	{
		// no data
		ret = QUE_RET_EMPTY;
	}
	else
	{
		// have data
		*data = p->queue[p->tail];
		p->pull_cnt ++;
		
		if (p->tail == (p->size-1))
		{
			p->tail = 0;
		}
		else
		{
			p->tail++;
		}
	}

	QUE_PULL_UNLOCK(p);
	return ret;
}

int queue_clean(queue_data_t* p)
{
	if (p->queue != NULL)
		free(p->queue);

	memset(p, 0, sizeof(queue_data_t));

	return QUE_RET_OK;
}

