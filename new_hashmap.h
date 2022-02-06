#ifndef HASHMAP_H
#define HASHMAP_H

#include "common.h"

typedef struct HMNode {
	struct HMNode *next;
	char *key;
	void *data;
} HMNode;

typedef struct HMIter {
	struct HMNode *cur;
	u64 idx;
} HMIter;

typedef struct HashMap {
	HMNode **map;
	u64 *idx_map;
	u64 idx_map_size;
	u64 size;
	u64 capacity;
} HashMap;

HashMap *hm_sized_init(u64 capacity) {
	HashMap *hm = (HashMap *)calloc(1, sizeof(HashMap));
	hm->size = 0;
	hm->idx_map_size = 0;
	hm->capacity = capacity;
	hm->map = (HMNode **)calloc(hm->capacity, sizeof(HMNode *));
	hm->idx_map = (u64 *)calloc(hm->capacity, sizeof(u64));

	return hm;
}

HashMap *hm_init() {
	HashMap *hm = hm_sized_init(1);
	return hm;
}

void print_idx_map(HashMap *hm) {
	printf("[PRINT_IDX_MAP] idx_map size: %llu, map size: %llu\n", hm->idx_map_size, hm->size);
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		printf("[PRINT_IDX_MAP] %llu\n", hm->idx_map[i]);
	}
}

void print_hm(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		printf("[PRINT_MAP] key: %s\n", bucket->key);
		u64 sub_key = 0;
		while (bucket->next != NULL) {
			printf("[PRINT_MAP] key: %s\n", bucket->next->key);
			bucket = bucket->next;
			sub_key += 1;
		}

		printf("[PRINT_MAP] sub_key count: %llu\n", sub_key);
	}
}

char *hm_iter_key(HashMap *hm, HMIter *iter) {
	if (iter->idx >= hm->idx_map_size) {
		return NULL;
	}

	if (!iter->cur) {
		iter->cur = hm->map[hm->idx_map[iter->idx]];
		return iter->cur->key;
	}

	if (iter->cur->next) {
		iter->cur = iter->cur->next;
		return iter->cur->key;
	}

	iter->idx++;
	if (iter->idx == hm->idx_map_size) {
		return NULL;
	}

	iter->cur = hm->map[hm->idx_map[iter->idx]];
	return iter->cur->key;
}


u64 hm_hash(HashMap *hm, char *key) {
	u64 hash = 0;
	for (u32 i = 0; i < strlen(key); i++) {
		hash = (hash << 4) ^ (hash >> 28) ^ (u64)key[i];
	}
	hash = hash % hm->capacity;

	return hash;
}

HMNode *new_hmnode(char *key, void *value) {
	HMNode *tmp = (HMNode *)malloc(sizeof(HMNode));
	tmp->data = value;
	tmp->key = strdup(key);
	tmp->next = NULL;
	return tmp;
}

HashMap *hm_grow_capacity(HashMap *hm, u64 capacity);

bool hm_insert(HashMap **hm, char *key, void *value) {
    if ((*hm)->size > (((*hm)->capacity >> 2) + ((*hm)->capacity >> 1))) {
		*hm = hm_grow_capacity(*hm, (*hm)->capacity * 2);
	}

	u64 idx = hm_hash(*hm, key);
	if (!((*hm)->map[idx])) {
		(*hm)->map[idx] = new_hmnode(key, value);
		(*hm)->idx_map[(*hm)->idx_map_size] = idx;
		(*hm)->idx_map_size++;
		(*hm)->size++;
		return true;
	}

	if (!strcmp((*hm)->map[idx]->key, key)) {
		return false;
	}

	HMNode *tmp = (*hm)->map[idx];

	while (tmp->next) {
		if (!strcmp(tmp->key, key)) {
			return false;
		}
		tmp = tmp->next;
	}

	if (!strcmp(tmp->key, key)) {
		return false;
	}

	tmp->next = new_hmnode(key, value);
	(*hm)->size++;
	return true;
}

void hn_free(HMNode *bucket) {
	free(bucket->key);
	free(bucket);
}

void hn_free_data(HMNode *bucket) {
	free(bucket->data);
	hn_free(bucket);
}

HashMap *hm_grow_capacity(HashMap *hm, u64 capacity) {
	debug("[HM] growing capacity from %llu to %llu because size is %llu\n", hm->capacity, hm->capacity * 2, hm->size);
	HashMap *new_hm = hm_sized_init(capacity);

	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];

		HMNode *tmp = bucket;
		HMNode *prev = tmp;
		while (tmp->next) {
			hm_insert(&new_hm, tmp->key, tmp->data);
			prev = tmp;
			tmp = tmp->next;

			hn_free(prev);
		}
		hm_insert(&new_hm, tmp->key, tmp->data);
		hn_free(tmp);
	}

	free(hm->map);
	free(hm->idx_map);
	free(hm);
	return new_hm;
}

HMNode *_hm_get(HashMap *hm, char *key) {
	u64 idx = hm_hash(hm, key);

	HMNode *bucket = hm->map[idx];
	if (!bucket) {
		debug("key %s, hm->map[%llu] is NULL?\n", key, idx);
		return NULL;
	}

	if (!strcmp(bucket->key, key)) {
		return bucket;
	}

	HMNode *tmp = bucket;
	while (tmp->next) {
		tmp = tmp->next;
		if (!strcmp(tmp->key, key)) {
			return tmp;
		}
	}
	debug("key %s, hm->map[%llu] is NULL?\n", key, idx);
	return NULL;
}

void *hm_get(HashMap *hm, char *key) {
	HMNode *ret = _hm_get(hm, key);
	if (ret) {
		return ret->data;
	}

	return NULL;
}

bool idx_map_del(u64 *idx_map, u64 *idx_map_size, u64 idx) {
	u64 size = *idx_map_size;
    if (!size) {
		return false;
	}

	u64 idx_idx = 0;
	bool broke = false;
	for (; idx_idx < size; idx_idx++) {
		if (idx_map[idx_idx] == idx) {
			broke = true;
			break;
		}
	}

	if (idx_idx > size || broke != true || (idx_map[idx_idx] == (u64)-1)) {
		return false;
	}

	for (u64 i = idx_idx; i < size; i++) {
		idx_map[i] = idx_map[i + 1];
	}

	idx_map[size] = -1;
	*idx_map_size -= 1;

	return true;
}

bool hm_remove(HashMap **hm, char *key) {
	HMNode *node = _hm_get(*hm, key);
	if (!node) {
		return false;
	}

	HMNode *tmp = node->next;
	u64 idx = hm_hash(*hm, key);

	if (!tmp) {
		bool ret = idx_map_del((*hm)->idx_map, &(*hm)->idx_map_size, idx);

		if (!ret) {
			return false;
		}
	}

	hn_free(node);
	(*hm)->map[idx] = tmp;
	(*hm)->size--;

	return true;
}

void hm_free(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];

		HMNode *tmp = bucket;
		HMNode *prev = tmp;
		while (tmp->next) {
			prev = tmp;
			tmp = tmp->next;
			hn_free(prev);
		}
		hn_free(tmp);
	}
	free(hm->map);
	free(hm->idx_map);
	free(hm);
}

void hm_free_data(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];

		HMNode *tmp = bucket;
		HMNode *prev = tmp;
		while (tmp->next) {
			prev = tmp;
			tmp = tmp->next;
			hn_free_data(prev);
		}
		hn_free_data(tmp);
	}
	free(hm->map);
	free(hm->idx_map);
	free(hm);
}

#endif
