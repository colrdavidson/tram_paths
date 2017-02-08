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

typedef struct FloatWrapper {
	f32 f;
} FloatWrapper;

FloatWrapper *fw_init(f32 f) {
	FloatWrapper *fw = (FloatWrapper *)malloc(sizeof(FloatWrapper));
	fw->f = f;
	return fw;
}

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

void print_station_list(DynArr *station_list) {
	for (u64 i = 0; i < station_list->size; i++) {
		StationNode *station = (StationNode *)(station_list->buffer[i]);
		printf("%s %s\n", station->line, station->name);
	}
}

void print_station_map(HashMap *hm) {
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		StationNode *station = ((StationNode *)bucket->data);
		if (station != NULL) {
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
	DynArr *station_list = da_init();

	for (u64 i = 0; i < file_lines->size; i++) {
		char station1[257] = {0};
		char station2[257] = {0};
		char line1[257] = {0};
		char line2[257] = {0};
		sscanf((char *)(file_lines->buffer[i]), "%256[^','], %256[^','], %256[^','], %256[^','], %*s", station1, line1, station2, line2);

		char *lookup1 = station_lookup(station1, line1);
		char *lookup2 = station_lookup(station2, line2);

		StationNode *station_node1 = new_station(station1, line1);
		StationNode *station_node2 = new_station(station2, line2);

		bool stat1_new = hm_insert(&map, lookup1, station_node1);
		bool stat2_new = hm_insert(&map, lookup2, station_node2);

		if (stat1_new) {
			da_insert(station_list, station_node1);
		}
		if (stat2_new) {
			da_insert(station_list, station_node2);
		}
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

	char *start_lookup = station_lookup("Z", "VIOLET");
	char *end_lookup = station_lookup("G", "GREEN");
    StationNode *start = hm_get(map, start_lookup);
    StationNode *end = hm_get(map, end_lookup);

	PriorityQueue *frontier = pq_init();
	pq_push(frontier, hm_get(map, start_lookup), 0);

	HashMap *from = hm_init();
	HashMap *accrued_cost = hm_init();

	hm_insert(&from, start_lookup, NULL);
	hm_insert(&accrued_cost, start_lookup, fw_init(0.0f));

	while (frontier->heap->size > 0) {
		StationNode *current = pq_pop(frontier);
		char *current_lookup = station_lookup(current->name, current->line);

		if (current == end) {
			break;
		}

		for (u64 i = 0; i < current->conn->size; i++) {
			ConnNode *next_conn = ((ConnNode *)current->conn->buffer[i]);
			f32 new_cost = ((FloatWrapper *)hm_get(accrued_cost, current_lookup))->f + next_conn->time;
			char *next_lookup = station_lookup(next_conn->station->name, next_conn->station->line);

			if (hm_get(accrued_cost, next_lookup) == NULL || new_cost < ((FloatWrapper *)hm_get(accrued_cost, next_lookup))->f) {
				hm_insert(&accrued_cost, next_lookup, fw_init(new_cost));
				pq_push(frontier, next_conn->station, new_cost);
				hm_insert(&from, next_lookup, current);
			}
		}
	}

	DynArr *path = da_init();
	char *current_lookup = station_lookup(end->name, end->line);
	f32 accm_time = ((FloatWrapper *)hm_get(accrued_cost, current_lookup))->f;

	StationNode *current = end;
	da_insert(path, end);
	da_insert(path, current);
	while (current != start) {
		char *current_lookup = station_lookup(current->name, current->line);
		current = hm_get(from, current_lookup);
		da_insert(path, current);
	}

	printf("-------------\n");
	printf("Trip Summary\n");
	printf("   %s -> %s\n", start->name, end->name);
	printf("-------------\n\n");
	for (u64 i = path->size - 1; i > 0; i--) {
		StationNode *current = (StationNode *)path->buffer[i];
		printf("  %s %s\n", current->name, current->line);
	}
	printf("\ntravel time: %.2g minutes\n", accm_time);
	printf("-----------------------\n");

	return 0;
}
