#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct HMNode {
	struct HMNode *next;
	char *key;
	void *data;
} HMNode;

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
	HashMap *hm = hm_sized_init(10);
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
		while (bucket->next != NULL) {
			printf("[PRINT_MAP] key: %s\n", bucket->next->key);
			bucket = bucket->next;
		}
	}
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
	if ((*hm)->map[idx] == NULL) {
		DEBUG_PRINT("inserting new HMNode %s @ %llu\n", key, idx);

		(*hm)->map[idx] = new_hmnode(key, value);
		(*hm)->idx_map[(*hm)->idx_map_size] = idx;
		(*hm)->idx_map_size++;
	} else if (strcmp((*hm)->map[idx]->key, key) != 0) {
		DEBUG_PRINT("hash collision for key %s, idx %llu\n", key, idx);
		HMNode *tmp = (*hm)->map[idx];
		while (tmp->next != NULL) {
			if (strcmp(tmp->key, key) == 0) {
				DEBUG_PRINT("key %s @ idx %llu already exists!\n", key, idx);
				return false;
			}
			tmp = tmp->next;
		}
		if (strcmp(tmp->key, key) == 0) {
			DEBUG_PRINT("key %s @ idx %llu already exists!\n", key, idx);
			return false;
		}

		DEBUG_PRINT("inserting new HMNode %s @ %llu\n", key, idx);
		tmp->next = new_hmnode(key, value);
	} else {
		DEBUG_PRINT("key %s @ idx %llu already exists!\n", key, idx);
		return false;
	}

	(*hm)->size++;
	return true;
}

HashMap *hm_grow_capacity(HashMap *hm, u64 capacity) {
	DEBUG_PRINT("[HM] growing capacity from %llu to %llu because size is %llu\n", hm->capacity, hm->capacity * 2, hm->size);
	HashMap *new_hm = hm_sized_init(capacity);

	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];

		HMNode *tmp = bucket;
		HMNode *prev = tmp;
		while (tmp->next != NULL) {
			hm_insert(&new_hm, tmp->key, tmp->data);
			prev = tmp;
			tmp = tmp->next;
			free(prev);
		}
		hm_insert(&new_hm, tmp->key, tmp->data);
		free(tmp);
	}

	free(hm->map);
	free(hm->idx_map);
	free(hm);
	return new_hm;
}

void *hm_get(HashMap *hm, char *key) {
	u64 idx = hm_hash(hm, key);

	HMNode *bucket = hm->map[idx];
	if (bucket != NULL) {
		if (strcmp(bucket->key, key) == 0) {
			return bucket->data;
		} else {
			HMNode *tmp = bucket;
			while (tmp->next != NULL) {
				tmp = tmp->next;
				if (strcmp(tmp->key, key) == 0) {
					return tmp->data;
				}
			}
			DEBUG_PRINT("key %s, hm->map[%llu] is NULL?\n", key, idx);
			return NULL;
		}
	} else {
		DEBUG_PRINT("key %s, hm->map[%llu] is NULL?\n", key, idx);
		return NULL;
	}
}

void hn_free(HMNode *bucket) {
	free(bucket->key);
	free(bucket);
}

void hn_free_data(HMNode *bucket) {
	free(bucket->data);
	hn_free(bucket);
}

void hm_free(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		while (bucket->next != NULL) {
			HMNode *tmp = bucket->next;
			hn_free(bucket);
			bucket = tmp;
		}
	}
}

void hm_free_data(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		while (bucket->next != NULL) {
			HMNode *tmp = bucket->next;
			hn_free_data(bucket);
			bucket = tmp;
		}
	}
}

#endif
