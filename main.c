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
	char *line = (char *)malloc(line_length);
	memcpy(line, tmp_start, line_length - 1);
	line[line_length - 1] = 0;

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
	char *station;
	char *line1;
	char *line2;
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

ConnNode *new_connection(char *station, char *line1, char *line2, float time) {
	ConnNode *node = (ConnNode *)malloc(sizeof(ConnNode));
	node->station = station;
	node->line1 = line1;
	node->line2 = line2;
	node->time = time;
	return node;
}

void print_connections(DynArr *conn) {
	for (u64 i = 0; i < conn->size; i++) {
		ConnNode *node = (ConnNode *)conn->buffer[i];
		printf("[%s] -> [%s] %s in %.2gs\n", node->line1, node->line2, node->station, node->time);
	}
}

void print_station_map(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		printf("Station %s\n---------\n", bucket->key);
		print_connections(((StationNode *)bucket->data)->conn);
		while (bucket->next != NULL) {
			printf("Station %s\n---------\n", bucket->key);
			print_connections(((StationNode *)bucket->next->data)->conn);
			bucket = bucket->next;
		}
		puts("");
	}
}

int main() {
	File *station_file = read_file("stations.log");
	DynArr *file_lines = read_all_lines(station_file);
	DynArr *inserted_stations = da_init();
	HashMap *map = hm_init();

	for (u64 i = 0; i < file_lines->size; i++) {
		char *station1 = malloc(sizeof(char) * 256);
		char *station2 = malloc(sizeof(char) * 256);
		sscanf((char *)(file_lines->buffer[i]), "%256[^','], %*[^','], %256[^','], %*[^','], %*s", station1, station2);

		bool station1_inserted = hm_insert(&map, station1, (void *)new_station(station1));
		bool station2_inserted = hm_insert(&map, station2, (void *)new_station(station2));

		if (!station1_inserted) {
			free(station1);
		}

		if (!station2_inserted) {
			free(station2);
		}
	}

	for (u64 i = 0; i < file_lines->size; i++) {
		char *station1 = malloc(sizeof(char) * 256);
		char *line1 = malloc(sizeof(char) * 256);
		char *station2 = malloc(sizeof(char) * 256);
		char *line2 = malloc(sizeof(char) * 256);
		char *time = malloc(sizeof(char) * 256);
		sscanf((char *)(file_lines->buffer[i]), "%256[^','], %256[^','], %256[^','], %256[^','], %256s", station1, line1, station2, line2, time);

		da_insert(((StationNode *)hm_get(map, station1))->conn, new_connection(station2, line1, line2, strtof(time, NULL)));
		da_insert(((StationNode *)hm_get(map, station2))->conn, new_connection(station1, line2, line1, strtof(time, NULL)));
	}

	print_station_map(map);

	return 0;
}
