#ifndef DYNARR_H
#define DYNARR_H

#include "common.h"

typedef struct DynArr {
	void **buffer;
	u64 size;
	u64 capacity;
} DynArr;

DynArr *da_sized_init(u64 capacity) {
	DynArr *da = (DynArr *)malloc(sizeof(DynArr));
	da->buffer = (void **)malloc(sizeof(void *) * capacity);
	da->size = 0;
	da->capacity = capacity;
	return da;
}

DynArr *da_init() {
	return da_sized_init(1);
}

bool da_insert(DynArr *da, void *data) {
	if (data != NULL) {
		if (da->capacity <= da->size) {
			debug("[DA] growing capacity from %llu to %llu because size is %llu\n", da->capacity, da->size * 2, da->size);
			da->capacity = da->size * 2;
			da->buffer = (void **)realloc(da->buffer, sizeof(void *) * da->capacity);
		}
		da->buffer[da->size] = data;
		da->size++;
		return true;
	}
	return false;
}

void da_print(DynArr *da) {
	for (u64 i = 0; i < da->size; i++) {
		printf("\t%p\n", da->buffer[i]);
	}
}

void da_free(DynArr *da) {
	free(da->buffer);
	free(da);
}

void da_free_data(DynArr *da) {
	for (u64 i = 0; i < da->size; i++) {
		free(da->buffer[i]);
	}
	free(da->buffer);
	free(da);
}

#endif
