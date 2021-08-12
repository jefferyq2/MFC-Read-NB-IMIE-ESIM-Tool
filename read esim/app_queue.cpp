
#include "pch.h"
#include "app_queue.h"
#include "string.h"


void app_queue_init(void *p_q1, q_size_t q_max, q_size_t a_size)
{
	pos_t p;
	uint8_t len;
	q_size_t *pq;

	pq = (q_size_t *)p_q1;
	len = sizeof(pos_t);

	memcpy(&p, pq, len);
	p.q_len = q_max * a_size;
	p.size = a_size;
	memcpy(pq, &p, len);
}

void app_queue_clr(void *p_q1)
{
	pos_t p;
	uint8_t len;

	len = sizeof(pos_t);

	memcpy(&p, (q_size_t *)p_q1, len);
	p.end = 0;
	p.head = 0;
	memcpy((q_size_t *)p_q1, &p, len);
}

unsigned char app_queue_none(void *p_q1)
{
	pos_t p;

	memcpy(&p, (q_size_t *)p_q1, sizeof(pos_t));

	if (p.head == p.end)
	{
		return Q_NONE;
	}

	return Q_NON_NONE;
}

void app_enqueue(void *p_q1, void *p_src1)
{
	pos_t p;
	uint8_t len;
	q_size_t *q;
	uint8_t *qsrc;

	len = sizeof(pos_t);

	q = (q_size_t *)p_q1;
	qsrc = (uint8_t *)p_src1;

	memcpy(&p, q, len);
	if (p.head == (p.end + p.size) % p.q_len)
	{
		return;
	}
	memcpy((uint8_t *)(q + 4) + p.end, qsrc, p.size);
	p.end = (p.end + p.size) % p.q_len;
	memcpy(q, &p, len);
}

void app_dequeue(void *p_q1, void *p_des1)
{
	pos_t p;
	uint8_t len;
	q_size_t *q;
	uint8_t *qdes;

	len = sizeof(pos_t);

	q = (q_size_t *)p_q1;
	qdes = (uint8_t *)p_des1;

	memcpy(&p, q, len);

	if (p.head == p.end)
	{
		return;
	}

	memcpy(qdes, (uint8_t *)(q + 4) + p.head, p.size);
	p.head = (p.head + p.size) % p.q_len;
	memcpy(q, &p, len);
}

uint8_t app_queue_search(void *p_q1, void *p_src1 )
{
	pos_t p;
	uint8_t *ps, *pt, *pm;
	uint8_t i;
	q_size_t *q;
	uint8_t *qsrc;

	q = (q_size_t *)p_q1;
	qsrc = (uint8_t *)p_src1;

	memcpy(&p, q, sizeof(pos_t));

	ps = (uint8_t *)(q + 4);
	while (p.head != p.end)
	{
		pt = ps + p.head;
		pm = qsrc;

		for (i = 0; i < p.size; i++)
		{
			if (*pt++ != *pm++) break;
		}

		if (i == p.size)
		{
			return Q_S_OK;
		}
		else
		{
			p.head += p.size;
			if (p.head >= p.q_len)
			{
				p.head = 0;
			}
		}
	}

	return Q_S_NONE;
}

uint16_t app_queue_get_len(void *queue)
{
	pos_t p;
	q_size_t *q;
	uint16_t len;

	q = (q_size_t *)queue;
	memcpy(&p, q, sizeof(pos_t));

	if (p.end >= p.head)
	{
		len = (p.end - p.head) / p.size;
	}
	else
	{
		len = (p.q_len - p.head + p.end) / p.size;
	}

	return len;
}

uint16_t app_queue_get_remain_len(void *queue)
{
	pos_t p;
	q_size_t *q;
	uint16_t len;

	q = (q_size_t *)queue;
	memcpy(&p, q, sizeof(pos_t));

	len = p.q_len/p.size - app_queue_get_len(queue) - 1;

	return len;
}

Q_status_t app_queue_traverse(void *queue, void *p_des, q_size_t idx)
{
  pos_t pos;
	q_size_t *q;
	uint16_t len;
	q_size_t head;
	
	len = app_queue_get_len(queue);
	if (idx >= len)
	{
		return Q_S_ERR;
	}
	q = (q_size_t *)queue;
	memcpy(&pos, q, sizeof(pos_t));
	head = (pos.head + idx * pos.size) % pos.q_len;
	memcpy(p_des, (uint8_t *)(q+4) + head, pos.size);
	
	return Q_S_OK;
}

Q_status_t app_queue_modify_speci_element(void *queue, q_size_t idx, void *p_replace)
{
	pos_t pos;
	q_size_t *q;
	uint16_t len;
	q_size_t head;

	len = app_queue_get_len(queue);
	if (idx >= len)
	{
		return Q_S_ERR;
	}
	q = (q_size_t *)queue;
	memcpy(&pos, q, sizeof(pos_t));
	head = (pos.head + idx * pos.size) % pos.q_len;
	memcpy((uint8_t *)(q + 4) + head, p_replace, pos.size);

	return Q_S_OK;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
uint16_t app_batch_queue_remain_len(batch_q_t *bq)
{
	uint16_t len;

	if (bq->end >= bq->head)
	{
		len = bq->buf_size - bq->end + bq->head - 1;
	}
	else
	{
		len = bq->head - (bq->end + 1);
	}

	return len;
}

uint16_t app_batch_queue_get_data_len(batch_q_t *bq)
{
	uint16_t len;

	len = bq->buf_size - app_batch_queue_remain_len(bq) - 1;

	return len;
}

int32_t app_batch_data_enqueue(batch_q_t *bq, void *p, uint16_t len)
{
	uint16_t len_tp;
	uint16_t head, end;
	
	if (p == 0)
	{
		return 0;
	}

	head = bq->head;
	end = bq->end;

	if (((end + 1) &  bq->buf_pos) == head)
	{
		return -1;
	}

	if (end + len >= bq->buf_size)
	{
		len_tp = bq->buf_size - end;
		if (len >= len_tp + head)
		{
			if (head > 0)
			{
				len = len_tp + head - 1;
			}
			else
			{
				len = len_tp - 1;
			}
		}
	}

	len_tp = len;
	if (bq->size == 1)
	{
		uint8_t *bq8, *q8;

		bq8 = (uint8_t *)bq->data;
		q8 = (uint8_t *)p;

		while (len_tp--)
		{
			bq8[end++ & bq->buf_pos] = *q8++;
		}
	}
	else if (bq->size == 2)
	{
		uint16_t *bq16, *q16;

		bq16 = (uint16_t *)bq->data;
		q16 = (uint16_t *)p;
		while (len_tp--)
		{
			bq16[end++ & bq->buf_pos] = *q16++;
		}
	}
	else if (bq->size == 4)
	{
		uint32_t *bq32, *q32;

		bq32 = (uint32_t *)bq->data;
		q32 = (uint32_t *)p;
		while (len_tp--)
		{
			bq32[end++ & bq->buf_pos] = *q32++;
		}
	}

	bq->end = (bq->end + len) & bq->buf_pos;

	return len;
}

int32_t app_batch_data_dequeue(batch_q_t *bq, void *p, uint16_t len)
{
	uint16_t len_tp;
	uint16_t end, head;


	end = bq->end;
	head = bq->head;

	if (end == head)
	{
		return -1;
	}
	else if (head < end)
	{
		if ((head + len) > end)
		{
			len = end - head;
		}
	}
	else
	{
		len_tp = bq->buf_size - head;
		if (len > len_tp + end)
		{
			len = bq->buf_size - head + end;
		}
	}

	len_tp = len;
	if (bq->size == 1)
	{
		uint8_t *bq8, *q8;

		bq8 = (uint8_t *)bq->data;
		q8 = (uint8_t *)p;
		while (len_tp--)
		{
			*q8++ = bq8[head++ & bq->buf_pos];
		}
	}
	else if (bq->size == 2)
	{
		uint16_t *bq16, *q16;
		bq16 = (uint16_t *)bq->data;
		q16 = (uint16_t *)p;
		while (len_tp--)
		{
			*q16++ = bq16[head++ & bq->buf_pos];
		}
	}
	else if (bq->size == 4)
	{
		uint32_t *bq32, *q32;
		bq32 = (uint32_t *)bq->data;
		q32 = (uint32_t *)p;
		while (len_tp--)
		{
			*q32++ = bq32[head++ & bq->buf_pos];
		}
	}

	bq->head = (bq->head + len) & bq->buf_pos;

	return len;
}

int16_t app_batch_queue_none(batch_q_t *bq)
{
	if (bq->end == bq->head)
	{
		return Q_NONE;
	}

	return Q_NON_NONE;
}

void app_batch_queue_init(batch_q_t *bq, void *p,uint8_t size, uint16_t len)
{
	bq->head = 0;
	bq->end = 0;
	bq->size = size;
	bq->buf_size = len;
	bq->buf_pos = len - 1;
	bq->data = p;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
struct
{
	uint8_t *buf;
	uint16_t len;
	uint16_t pos;
	uint8_t size;
}samp_q;

void app_data_samp_init(void *p,uint16_t buf_len,uint8_t size)
{
	samp_q.buf = (uint8_t *)p;
	samp_q.len = buf_len;
	samp_q.pos = 0;
	samp_q.size = size;
}

void app_data_samp_enqueue(void *p)
{
	memcpy(samp_q.buf + samp_q.pos*samp_q.size, (uint8_t *)p, samp_q.size);
	if(++samp_q.pos >= samp_q.len)
	{
		samp_q.pos = 0;
	}
}

uint16_t app_data_samp_dequeue(void *pdes, uint16_t len)
{
	uint16_t pos_one, pos_two;
	uint16_t len_one, len_two = 0;
	uint8_t *p_temp;
	uint16_t q_len, q_pos;

	if (len == 0)
	{
		return 0;
	}
	
	q_len = samp_q.len;
	q_pos = samp_q.pos;
	if (len > q_len)
	{
		len = q_len;
	}

	p_temp = (uint8_t *)pdes;
	if (q_pos == 0)
	{
		pos_one = q_len - len;
		len_one = len;
		len_two = 0;
	}
	else if (q_pos > 0)
	{
		if (len <= q_pos)
		{
			pos_one = q_pos - len;
			len_one = len;
			len_two = 0;
		}
		else
		{
			pos_one = 0;
			len_one = q_pos;
			len_two = len - q_pos;
			pos_two = q_len - len_two;
		}
	}

	while (len_two--)
	{
		memcpy(p_temp, pos_two*samp_q.size + samp_q.buf, samp_q.size);
		p_temp += samp_q.size;
		pos_two++;
	}
	while (len_one--)
	{
		memcpy(p_temp, pos_one*samp_q.size + samp_q.buf, samp_q.size);
		p_temp += samp_q.size;
		pos_one++;
	}
	
	return len;
}


