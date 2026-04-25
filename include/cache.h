#ifndef ASLN_CACHE_H
#define ASLN_CACHE_H

#include <time.h>
#include <pthread.h>

typedef struct cache_entry {
    char* key;
    char* value;
    time_t expires;
    struct cache_entry* next;
} cache_entry_t;

typedef struct {
    cache_entry_t** buckets;
    int size;
    int ttl_seconds;
    pthread_mutex_t mutex;
} cache_t;

cache_t* cache_create(int bucket_size, int ttl_seconds);
void cache_set(cache_t* cache, const char* key, const char* value);
char* cache_get(cache_t* cache, const char* key);
void cache_delete(cache_t* cache, const char* key);
void cache_cleanup(cache_t* cache);
void cache_destroy(cache_t* cache);

#endif
