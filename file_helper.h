#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include "common.h"

typedef struct File {
	char *filename;
	char *string;
	u64 size;
} File;

File *read_file(char *filename) {
	FILE *file = fopen(filename, "r");

	if (file == NULL) {
		printf("%s not found!\n", filename);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	u64 length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *file_string = (char *)malloc(length + 1);
	length = fread(file_string, 1, length, file);
	file_string[length] = 0;

	fclose(file);

	File *f = (File *)malloc(sizeof(File));
	f->filename = filename;
	f->string = file_string;
	f->size = length;

	return f;
}

char *file_to_string(const char *filename) {
	FILE *file = fopen(filename, "r");

	if (file == NULL) {
		printf("%s not found!\n", filename);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	u64 length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *file_string = (char *)malloc(length + 1);
	length = fread(file_string, 1, length, file);
	file_string[length] = 0;

	fclose(file);
	return file_string;
}

#endif
