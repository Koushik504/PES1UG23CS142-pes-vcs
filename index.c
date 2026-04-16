#include "index.h"
#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// initialize index
int index_init() {
    FILE *f = fopen(".pes/index", "w");
    if (!f) return -1;
    fclose(f);
    return 0;
}

// save index to file
int index_save(const Index *idx) {
    FILE *f = fopen(".pes/index", "w");
    if (!f) return -1;

    for (int i = 0; i < idx->count; i++) {
        char hex[65];
        hash_to_hex(&idx->entries[i].hash, hex);

        fprintf(f, "%o %s %s\n",
                idx->entries[i].mode,
                hex,
                idx->entries[i].path);
    }

    fclose(f);
    return 0;
}

// load index from file
int index_load(Index *idx) {
    FILE *f = fopen(".pes/index", "r");
    if (!f) return -1;

    idx->count = 0;

    while (!feof(f)) {
        IndexEntry *e = &idx->entries[idx->count];

        char hex[65];

        if (fscanf(f, "%o %64s %255s",
                   &e->mode,
                   hex,
                   e->path) != 3)
            break;

        hex_to_hash(hex, &e->hash);

        idx->count++;
    }

    fclose(f);
    return 0;
}

// add file to index
int index_add(Index *idx, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    // read file content
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    void *data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);

    IndexEntry *e = &idx->entries[idx->count];

    // write blob object
    if (object_write(OBJ_BLOB, data, size, &e->hash) != 0) {
        free(data);
        return -1;
    }

    free(data);

    // set mode + path
    e->mode = 0100644;
    strcpy(e->path, path);

    idx->count++;

    return 0;
}
