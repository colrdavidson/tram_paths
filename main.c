#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_FLAG 0
#include "common.h"
#include "hashmap.h"
#include "dynarr.h"

char *file_next_line(File *file, u64 *idx) {
	if (*idx >= file->size) {
		return NULL;
	}

	char *tmp_start = file->string + *idx;
	char *tmp = tmp_start;
	while ((*tmp != '\n') && (*tmp != '\0')) {
		tmp++;
	}
	tmp++;

	u64 line_length = tmp - tmp_start;
	char *line = (char *)malloc(line_length + 1);
	memcpy(line, tmp_start, line_length + 1);
	line[line_length] = 0;

	*idx += line_length;

	return line;
}


DynArr *read_all_lines(File *file) {
	DynArr *da = da_init();
	u64 idx = 0;

	bool inserted = false;
	do {
		inserted = da_insert(da, file_next_line(file, &idx));
	} while (inserted);

	return da;
}

void print_lines(DynArr *da) {
	for (u64 i = 0; i < da->size; i++) {
		printf("[%llu] %s", i, (char *)(da->buffer[i]));
	}
}

typedef struct ConnNode {
	char *name;
	char *line;
	float time;
} ConnNode;

typedef struct StationNode {
	char *name;
	DynArr *conn;
} StationNode;

StationNode *new_station(char *name) {
	StationNode *node = (StationNode *)malloc(sizeof(StationNode));
	node->name = name;
	node->conn = da_init();
	return node;
}

int main() {
	File *station_file = read_file("stations.log");
	DynArr *da = read_all_lines(station_file);
	HashMap *map = hm_init();

	for (u64 i = 0; i < da->size; i++) {
		char *station1 = strtok((char *)(da->buffer[i]), ", ");
		char *line1 = strtok(NULL, ", ");
		char *station2 = strtok(NULL, ", ");
		char *line2 = strtok(NULL, ", ");
		bool inserted = hm_insert(&map, station1, (void *)new_station(station1));
		printf("(%s) [%s] %s -> [%s] %s\n", BOOL_FMT(inserted), line1, station1, line2, station2);
	}
	return 0;
}
