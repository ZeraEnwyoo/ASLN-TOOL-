#include "../include/asln.h"
#include "../include/cache.h"
#include <string.h>
#include <stdlib.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

static unsigned long hash_key(const char* key, int bucket_size) {
    unsigned long hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (unsigned long)(*p);
        hash *= FNV_PRIME;
    }
    return hash % bucket_size;
}

cache_t* cache_create(int bucket_size, int ttl_seconds) {
    cache_t* cache = malloc(sizeof(cache_t));
    if (!cache) return NULL;
    cache->buckets = calloc(bucket_size, sizeof(cache_entry_t*));
    if (!cache->buckets) { free(cache); return NULL; }
    cache->size = bucket_size;
    cache->ttl_seconds = ttl_seconds;
    pthread_mutex_init(&cache->mutex, NULL);
    return cache;
}

void cache_set(cache_t* cache, const char* key, const char* value) {
    if (!cache || !key || !value) return;
    pthread_mutex_lock(&cache->mutex);
    unsigned long idx = hash_key(key, cache->size);
    cache_entry_t* curr = cache->buckets[idx];
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            free(curr->value);
            curr->value = strdup(value);
            if (!curr->value) { pthread_mutex_unlock(&cache->mutex); return; }
            curr->expires = time(NULL) + cache->ttl_seconds;
            pthread_mutex_unlock(&cache->mutex);
            return;
        }
        curr = curr->next;
    }
    cache_entry_t* entry = malloc(sizeof(cache_entry_t));
    if (!entry) { pthread_mutex_unlock(&cache->mutex); return; }
    entry->key = strdup(key);
    entry->value = strdup(value);
    if (!entry->key || !entry->value) {
        free(entry->key); free(entry->value); free(entry);
        pthread_mutex_unlock(&cache->mutex); return;
    }
    entry->expires = time(NULL) + cache->ttl_seconds;
    entry->next = cache->buckets[idx];
    cache->buckets[idx] = entry;
    pthread_mutex_unlock(&cache->mutex);
}

char* cache_get(cache_t* cache, const char* key) {
    if (!cache || !key) return NULL;
    pthread_mutex_lock(&cache->mutex);
    unsigned long idx = hash_key(key, cache->size);
    cache_entry_t* curr = cache->buckets[idx];
    time_t now = time(NULL);
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            if (curr->expires > now) {
                char* result = strdup(curr->value);
                pthread_mutex_unlock(&cache->mutex);
                return result;
            } else {
                pthread_mutex_unlock(&cache->mutex);
                cache_delete(cache, key);
                return NULL;
            }
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&cache->mutex);
    return NULL;
}

void cache_delete(cache_t* cache, const char* key) {
    if (!cache || !key) return;
    pthread_mutex_lock(&cache->mutex);
    unsigned long idx = hash_key(key, cache->size);
    cache_entry_t* curr = cache->buckets[idx];
    cache_entry_t* prev = NULL;
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            if (prev) prev->next = curr->next;
            else cache->buckets[idx] = curr->next;
            free(curr->key); free(curr->value); free(curr);
            pthread_mutex_unlock(&cache->mutex);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(&cache->mutex);
}

void cache_cleanup(cache_t* cache) {
    if (!cache) return;
    pthread_mutex_lock(&cache->mutex);
    time_t now = time(NULL);
    for (int i = 0; i < cache->size; i++) {
        cache_entry_t* curr = cache->buckets[i];
        cache_entry_t* prev = NULL;
        while (curr) {
            if (curr->expires <= now) {
                cache_entry_t* to_delete = curr;
                if (prev) { prev->next = curr->next; curr = curr->next; }
                else { cache->buckets[i] = curr->next; curr = curr->next; }
                free(to_delete->key); free(to_delete->value); free(to_delete);
            } else { prev = curr; curr = curr->next; }
        }
    }
    pthread_mutex_unlock(&cache->mutex);
}

void cache_destroy(cache_t* cache) {
    if (!cache) return;
    pthread_mutex_lock(&cache->mutex);
    for (int i = 0; i < cache->size; i++) {
        cache_entry_t* curr = cache->buckets[i];
        while (curr) {
            cache_entry_t* next = curr->next;
            free(curr->key); free(curr->value); free(curr);
            curr = next;
        }
    }
    free(cache->buckets);
    pthread_mutex_unlock(&cache->mutex);
    pthread_mutex_destroy(&cache->mutex);
    free(cache);
}
