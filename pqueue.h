#ifndef PQUEUE_H
#define PQUEUE_H

#include "common.h"
#include "dynarr.h"

#define GET_PARENT(i) (((i) - 1) / 2)
#define GET_LEFT_CHILD(i) ((2 * (i)) + 1)
#define GET_RIGHT_CHILD(i) ((2 * (i)) + 2)

typedef struct PriorityQueue {
	DynArr *heap;
} PriorityQueue;

typedef struct PriorityNode {
	void *data;
	f32 priority;
} PriorityNode;

PriorityQueue *pq_init() {
	PriorityQueue *pq = malloc(sizeof(PriorityQueue));
	pq->heap = da_init();

	return pq;
}

PriorityNode *pnode_init(void *data, f32 priority) {
	PriorityNode *pnode = (PriorityNode *)malloc(sizeof(PriorityNode));
	pnode->data = data;
	pnode->priority = priority;
	return pnode;
}

void sift_up_conn_heap(DynArr *heap, u64 idx) {
	if (idx != 0) {
		u64 parent_idx = GET_PARENT(idx);
		f32 parent_priority = ((PriorityNode *)heap->buffer[parent_idx])->priority;
		f32 idx_priority = ((PriorityNode *)heap->buffer[idx])->priority;
		if (parent_priority > idx_priority) {
			PriorityNode *tmp = (PriorityNode *)heap->buffer[parent_idx];
			heap->buffer[parent_idx] = heap->buffer[idx];
			heap->buffer[idx] = tmp;
			sift_up_conn_heap(heap, parent_idx);
		}
	}
}

void sift_down_conn_heap(DynArr *heap, u64 idx) {
	u64 left_child_idx = GET_LEFT_CHILD(idx);
	u64 right_child_idx = GET_RIGHT_CHILD(idx);
	u64 min_idx;

	if (right_child_idx >= heap->size) {
		if (left_child_idx >= heap->size) {
			return;
		} else {
			min_idx = left_child_idx;
		}
	} else {
		PriorityNode *left = heap->buffer[left_child_idx];
		PriorityNode *right = heap->buffer[left_child_idx];
		if (left->priority <= right->priority) {
			min_idx = left_child_idx;
		} else {
			min_idx = right_child_idx;
		}
	}

	PriorityNode *cur = heap->buffer[idx];
	PriorityNode *min = heap->buffer[min_idx];
	if (cur->priority > min->priority) {
		PriorityNode *tmp = min;
		heap->buffer[min_idx] = heap->buffer[idx];
		heap->buffer[idx] = tmp;
		sift_down_conn_heap(heap, min_idx);
	}
}

void pq_push(PriorityQueue *pq, void *data, f32 priority) {
	PriorityNode *pn = pnode_init(data, priority);
	da_insert(pq->heap, pn);
	u64 inserted_idx = pq->heap->size - 1;
	sift_up_conn_heap(pq->heap, inserted_idx);
}

void *pq_pop(PriorityQueue *pq) {
	if (pq->heap->size == 0) {
		printf("Heap is empty!\n");
		return NULL;
	} else {
		void *ret = ((PriorityNode *)pq->heap->buffer[0])->data;
		free(pq->heap->buffer[0]);
		pq->heap->buffer[0] = pq->heap->buffer[pq->heap->size - 1];
		pq->heap->size--;
		if (pq->heap->size > 0) {
			sift_down_conn_heap(pq->heap, 0);
		}
		return ret;
	}
}

void pq_free(PriorityQueue *pq) {
	da_free_data(pq->heap);
	free(pq);
}

#endif
