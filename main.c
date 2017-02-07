#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_FLAG 0
#include "common.h"
#include "hashmap.h"
#include "dynarr.h"
#include "pqueue.h"

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

typedef struct StationNode {
	char *name;
	char *line;
	DynArr *conn;
} StationNode;

typedef struct ConnNode {
	StationNode *station;
	f32 time;
} ConnNode;

StationNode *new_station(char *name, char *line) {
	StationNode *node = (StationNode *)malloc(sizeof(StationNode));
	node->name = strdup(name);
	node->line = strdup(line);
	node->conn = da_init();
	return node;
}

ConnNode *new_connection(StationNode *station, f32 time) {
	ConnNode *node = (ConnNode *)malloc(sizeof(ConnNode));
	node->station = station;
	node->time = time;
	return node;
}

void print_connections(DynArr *conn) {
	for (u64 i = 0; i < conn->size; i++) {
		ConnNode *node = (ConnNode *)(conn->buffer[i]);
		printf("%s %s in %.2gs\n", node->station->line, node->station->name, node->time);
	}
	puts("");
}

void print_station_map(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		StationNode *station = ((StationNode *)bucket->data);
		printf("Station %s | %s\n---------\n", station->name, station->line);
		print_connections(station->conn);

		while (bucket->next != NULL) {
			StationNode *station = ((StationNode *)bucket->next->data);
			printf("Station %s | %s\n---------\n", station->name, station->line);
			print_connections(station->conn);
			bucket = bucket->next;
		}
	}
}

void print_station_names(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		StationNode *station = ((StationNode *)bucket->data);
		printf("Station %s | %s\n", station->name, station->line);
		while (bucket->next != NULL) {
			StationNode *station = ((StationNode *)bucket->data);
			printf("Station %s | %s\n", station->name, station->line);
			bucket = bucket->next;
		}
	}
}

int main() {
	File *station_file = read_file("stations.log");
	DynArr *file_lines = read_all_lines(station_file);
	HashMap *map = hm_init();
	u64 num_stations = 0;

	for (u64 i = 0; i < file_lines->size; i++) {
		char station1[257] = {0};
		char station2[257] = {0};
		char line1[257] = {0};
		char line2[257] = {0};
		sscanf((char *)(file_lines->buffer[i]), "%256[^','], %256[^','], %256[^','], %256[^','], %*s", station1, line1, station2, line2);

		char *lookup1 = station_lookup(station1, line1);
		char *lookup2 = station_lookup(station2, line2);
		num_stations += hm_insert(&map, lookup1, new_station(station1, line1));
		num_stations += hm_insert(&map, lookup2, new_station(station2, line2));
	}

	for (u64 i = 0; i < file_lines->size; i++) {
		char station1[257] = {0};
		char station2[257] = {0};
		char time[257] = {0};
		char line1[257] = {0};
		char line2[257] = {0};
		sscanf((char *)(file_lines->buffer[i]), "%256[^','], %256[^','], %256[^','], %256[^','], %256s", station1, line1, station2, line2, time);

		char *lookup1 = station_lookup(station1, line1);
		char *lookup2 = station_lookup(station2, line2);
		da_insert(((StationNode *)hm_get(map, lookup1))->conn, new_connection(hm_get(map, lookup2), strtof(time, NULL)));
	    da_insert(((StationNode *)hm_get(map, lookup2))->conn, new_connection(hm_get(map, lookup1), strtof(time, NULL)));
	}

	PriorityQueue *frontier = pq_init();
	pq_push(frontier, hm_get(map, "M~YELLOW"), 0);

	return 0;
}
