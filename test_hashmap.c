#include "hashmap.h"
#include "stdlib.h"
#include "assert.h"

int main() {
	bool ret;

	u64 test_size = 1000000;
    char **keys = malloc(sizeof(char *) * test_size);
	for (u64 i = 0; i < test_size; i++) {
		char *key = calloc(sizeof(char), 255);
		sprintf(key, "toast-%llu", i);
		keys[i] = key;
	}

	HashMap *map = hm_init();
	for (u64 i = 0; i < 5; i++) {
		char *key = keys[i];

		ret = hm_insert(&map, key, (void *)"floopy");
		assert(ret == true);

		ret = hm_insert(&map, key, (void *)"floopy");
		assert(ret == false);

		ret = hm_remove(&map, key);
		assert(ret == true);

		ret = hm_remove(&map, key);
		assert(ret == false);
	}


	u64 start = get_time_ms();
	for (u64 i = 0; i < test_size; i++) {
		char *key = keys[i];
		ret = hm_insert(&map, key, (void *)"floopy");
		assert(ret == true);
	}
	printf("Allocation took: %llu ms\n", get_time_ms() - start);

	start = get_time_ms();
	for (u64 i = 0; i < test_size; i++) {
		char *key = keys[i];
		ret = hm_remove(&map, key);
		assert(ret == true);
	}
	printf("Removal took: %llu ms\n", get_time_ms() - start);

	hm_free(map);
}
