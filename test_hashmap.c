#define DEBUG

#include "hashmap.h"
#include "stdlib.h"
#include "assert.h"

int main() {
	bool ret;

	u64 test_size = 1000000;
    char **keys = malloc(sizeof(char *) * test_size);
    char **data = malloc(sizeof(char *) * test_size);
	for (u64 i = 0; i < test_size; i++) {
		char *key = malloc(sizeof(char) * 16);
		char *datum = malloc(sizeof(char) * 16);
		sprintf(key, "toast-%llu", i);
		sprintf(datum, "floopy-%llu", i);
		keys[i] = key;
		data[i] = datum;
	}

	printf("Built dataset!\n");

	HashMap *map = hm_sized_init(test_size * 2);
	for (u64 i = 0; i < 5; i++) {
		char *key = keys[i];

		hm_insert(&map, key, (void *)"floopy");
		hm_insert(&map, key, (void *)"floopy");

		ret = hm_remove(map, key);
		assert(ret == true);

		ret = hm_remove(map, key);
		assert(ret == false);
	}
	printf("Finished quick check\n");

	u64 start = get_time_ms();
	for (u64 i = 0; i < test_size; i++) {
		char *key = keys[i];
		char *datum = data[i];
		hm_insert(&map, key, (void *)datum);
	}
	printf("Allocation took: %llu ms\n", get_time_ms() - start);

	start = get_time_ms();
	for (u64 i = 0; i < test_size; i++) {
		char *key = keys[i];
		char *result = (char *)hm_get(map, key);
		assert(result != NULL);
		assert(result[0] == 'f');
	}
	printf("Indexing took: %llu ms\n", get_time_ms() - start);

	start = get_time_ms();
	for (u64 i = 0; i < test_size; i++) {
		char *key = keys[i];
		ret = hm_remove(map, key);
		assert(ret == true);
	}
	printf("Removal took: %llu ms\n", get_time_ms() - start);

	assert(!map->idx_map_size && !map->size);

	hm_free(map);
}
