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

typedef struct Route {
	DynArr *path;
	HashMap *from_data;
	HashMap *transit_times;
	f32 accum_time;
	char *start;
	char *start_line;
	char *end;
	char *end_line;
} Route;

Route *new_route(DynArr *path, HashMap *from_data, HashMap *transit_times, f32 accum_time, char *start, char *start_line, char *end, char *end_line) {
	Route *route = (Route *)malloc(sizeof(Route));
	route->path = path;
	route->from_data = from_data;
	route->transit_times = transit_times;
	route->accum_time = accum_time;
	route->start = start;
	route->start_line = start_line;
	route->end = end;
	route->end_line = end_line;
	return route;
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

void free_route(Route *route) {
	hm_free(route->transit_times);
	hm_free(route->from_data);

	if (route->path != NULL) {
		da_free(route->path);
	}
	free(route);
}

StationNode *new_station(char *name, char *line) {
	StationNode *node = (StationNode *)malloc(sizeof(StationNode));
	node->name = strdup(name);
	node->line = strdup(line);
	node->conn = da_init();
	return node;
}

void free_station(StationNode *node) {
	free(node->name);
	free(node->line);
	da_free(node->conn);
	free(node);
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

DynArr *flatten_map_keys(HashMap *hm) {
	DynArr *flat = da_init();
	for (u64 i = 0; i < hm->idx_map_size; i++) {
		HMNode *bucket = hm->map[hm->idx_map[i]];
		da_insert(flat, bucket->key);
		while (bucket->next != NULL) {
			da_insert(flat, bucket->next->key);
			bucket = bucket->next;
		}
	}

	return flat;
}

Route *find_route(HashMap *map, char *start, char *start_line, char *end, char *end_line) {
	char *start_lookup = station_lookup(start, start_line);
	char *end_lookup = station_lookup(end, end_line);
    StationNode *start_station = hm_get(map, start_lookup);
    StationNode *end_station = hm_get(map, end_lookup);

	PriorityQueue *frontier = pq_init();
	pq_push(frontier, hm_get(map, start_lookup), 0);

	HashMap *from = hm_init();
	HashMap *accrued_cost = hm_init();

	hm_insert(&from, start_lookup, NULL);
	hm_insert(&accrued_cost, start_lookup, FLOATVOID(0.0f));

	while (frontier->heap->size > 0) {
		StationNode *current = pq_pop(frontier);

		if (current == end_station) {
			break;
		}

		for (u64 i = 0; i < current->conn->size; i++) {
			ConnNode *next_conn = ((ConnNode *)current->conn->buffer[i]);

			char *current_lookup = station_lookup(current->name, current->line);
			char *next_lookup = station_lookup(next_conn->station->name, next_conn->station->line);

			f32 new_cost = VOIDFLOAT(hm_get(accrued_cost, current_lookup)) + next_conn->time;

			if (hm_get(accrued_cost, next_lookup) == NULL || new_cost < VOIDFLOAT(hm_get(accrued_cost, next_lookup))) {
				hm_insert(&accrued_cost, next_lookup, FLOATVOID(new_cost));
				pq_push(frontier, next_conn->station, new_cost);
				hm_insert(&from, next_lookup, current);
			}

			free(current_lookup);
			free(next_lookup);
		}
	}

	char *current_lookup = station_lookup(end_station->name, end_station->line);
	f32 accum_time = VOIDFLOAT(hm_get(accrued_cost, current_lookup));

	free(start_lookup);
	free(end_lookup);
	free(current_lookup);
	pq_free(frontier);

	return new_route(NULL, from, accrued_cost, accum_time, start, start_line, end, end_line);
}

Route *find_best_route(HashMap *map, DynArr *line_list, char *start, char *end) {
	DynArr *start_options = da_init();
	DynArr *end_options = da_init();

	for (u64 i = 0; i < line_list->size; i++) {
    	char *current_start_lookup = station_lookup(start, ((char *)line_list->buffer[i]));
    	char *current_end_lookup = station_lookup(end, ((char *)line_list->buffer[i]));
		if (hm_get(map, current_start_lookup) != NULL) {
			da_insert(start_options, line_list->buffer[i]);
		}
		if (hm_get(map, current_end_lookup) != NULL) {
			da_insert(end_options, line_list->buffer[i]);
		}
		free(current_start_lookup);
		free(current_end_lookup);
	}

	DynArr *route_options = da_init();
	for (u64 i = 0; i < start_options->size; i++) {
		for (u64 j = 0; j < end_options->size; j++) {
			da_insert(route_options, find_route(map, start, start_options->buffer[i], end, end_options->buffer[j]));
		}
	}

	da_free(start_options);
	da_free(end_options);

	Route *best = route_options->buffer[0];
	for (u64 i = 0; i < route_options->size; i++) {
		Route *new = route_options->buffer[i];
		if (best->accum_time > new->accum_time) {
			best = new;
		}
	}

	for (u64 i = 0; i < route_options->size; i++) {
		Route *cur = ((Route *)route_options->buffer[i]);
		if (cur != best) {
			free_route(cur);
		}
	}

	free(route_options->buffer);
	free(route_options);

	// Fill walkable path for best route
	DynArr *path = da_init();
	char *start_lookup = station_lookup(start, best->start_line);
	char *end_lookup = station_lookup(end, best->end_line);
    StationNode *start_station = hm_get(map, start_lookup);
    StationNode *end_station = hm_get(map, end_lookup);
	da_insert(path, end_station);
	da_insert(path, end_station);

	free(start_lookup);
	free(end_lookup);

	StationNode *current = end_station;
	while (current != start_station) {
		char *current_lookup = station_lookup(current->name, current->line);
		current = hm_get(best->from_data, current_lookup);
        free(current_lookup);

		da_insert(path, current);
	}
	best->path = path;

	return best;
}

void print_route(Route *route) {
	printf("-------------\n");
	printf("Trip Summary\n");
	printf("   %s -> %s\n", route->start, route->end);
	printf("-------------\n\n");
	for (u64 i = route->path->size - 1; i > 0; i--) {
		StationNode *current = (StationNode *)route->path->buffer[i];
		printf("  %s %s\n", current->name, current->line);
	}
	printf("\ntravel time: %.2g minutes\n", route->accum_time);
	printf("-----------------------\n");
}

int main() {
	File *station_file = read_file("stations.log");
	DynArr *file_lines = read_all_lines(station_file);
	HashMap *map = hm_init();
	HashMap *line_map = hm_init();

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

		hm_insert(&map, lookup1, station_node1);
		hm_insert(&map, lookup2, station_node2);

		hm_insert(&line_map, line1, (void *)1);
		hm_insert(&line_map, line2, (void *)1);
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

	DynArr *line_list = flatten_map_keys(line_map);
	for (u64 i = 0; i < 100000; i++) {
		Route *route = find_best_route(map, line_list, "G", "Z");
		free_route(route);
	}

	return 0;
}
