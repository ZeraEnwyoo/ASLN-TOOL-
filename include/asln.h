#ifndef ASLN_H
#define ASLN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <curl/curl.h>
#include <openssl/sha.h>

#define ASLN_VERSION "2.2.0"
#define KEY_ROTATION_MS 500
#define DYNAMIC_KEY_LEN 32

typedef enum {
    AGENCY_NASA = 0,
    AGENCY_SPACEX = 1,
    AGENCY_ESA = 2,
    AGENCY_CNSA = 3,
    AGENCY_ROSCOSMOS = 4
} agency_t;

typedef struct asln_handle {
    int active;
    int running;
    agency_t current_agency;
    uint8_t current_key[DYNAMIC_KEY_LEN];
    pthread_t key_thread;
    pthread_mutex_t key_mutex;
    void* api_cache;
    char nasa_api_key[64];
} asln_handle_t;

int init_key_generator(asln_handle_t* handle);
int get_current_key(asln_handle_t* handle, uint8_t* output);
int start_key_rotation(asln_handle_t* handle);
int stop_key_rotation(asln_handle_t* handle);

#endif
