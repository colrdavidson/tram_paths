#ifndef DYNARR_H
#define DYNARR_H

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
			DEBUG_PRINT("[DA] growing capacity from %llu to %llu because size is %llu\n", da->capacity, da->size * 2, da->size);
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

#endif
