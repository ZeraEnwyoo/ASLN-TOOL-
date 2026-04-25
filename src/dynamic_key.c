#include "../include/asln.h"
#include "../include/cache.h"
#include <curl/curl.h>
#include <openssl/sha.h>

struct MemoryStruct { char* memory; size_t size; };
static cache_t* api_cache = NULL;
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
static char g_nasa_api_key[64] = "DEMO_KEY";

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

static void init_api_cache(void) {
    if (!api_cache) {
        pthread_mutex_lock(&init_mutex);
        if (!api_cache) api_cache = cache_create(256, 300);
        pthread_mutex_unlock(&init_mutex);
    }
}

static char* call_api_with_cache(const char* url, const char* cache_key) {
    init_api_cache();
    char* cached = cache_get(api_cache, cache_key);
    if (cached) return cached;
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    if (!chunk.memory) { curl_easy_cleanup(curl); return NULL; }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res == CURLE_OK && chunk.memory && chunk.size > 0) {
        cache_set(api_cache, cache_key, chunk.memory);
        return chunk.memory;
    }
    free(chunk.memory);
    return NULL;
}

static char* call_nasa_api_cached(void) {
    char url[256];
    snprintf(url, sizeof(url), "https://api.nasa.gov/planetary/apod?api_key=%s", g_nasa_api_key);
    return call_api_with_cache(url, "nasa_apod");
}

static void generate_dynamic_key(asln_handle_t* handle, uint8_t* output) {
    char nasa_data[256] = "The Moon";
    char* api_result = call_nasa_api_cached();
    if (api_result) {
        char* title_start = strstr(api_result, "\"title\"");
        if (title_start) {
            char* colon = strchr(title_start, ':');
            if (colon) {
                char* quote1 = strchr(colon, '"');
                if (quote1) {
                    char* quote2 = strchr(quote1 + 1, '"');
                    if (quote2) {
                        int len = quote2 - quote1 - 1;
                        if (len > 0 && len < 100) strncpy(nasa_data, quote1 + 1, len);
                    }
                }
            }
        }
        free(api_result);
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long half_sec = ts.tv_nsec / 500000000;
    char key_input[512];
    const char* agency_str = "";
    switch (handle->current_agency) {
        case AGENCY_NASA: agency_str = "nasa"; break;
        case AGENCY_SPACEX: agency_str = "starlink"; break;
        case AGENCY_ESA: agency_str = "esa"; break;
        case AGENCY_CNSA: agency_str = "tiangong"; break;
        case AGENCY_ROSCOSMOS: agency_str = "baikonur"; break;
    }
    snprintf(key_input, sizeof(key_input), "%s_%s_%ld_%ld", nasa_data, agency_str, ts.tv_sec, half_sec);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, key_input, strlen(key_input));
    SHA256_Final(output, &ctx);
}

void* key_rotation_thread(void* arg) {
    asln_handle_t* handle = (asln_handle_t*)arg;
    while (handle->running) {
        uint8_t new_key[DYNAMIC_KEY_LEN];
        generate_dynamic_key(handle, new_key);
        pthread_mutex_lock(&handle->key_mutex);
        memcpy(handle->current_key, new_key, DYNAMIC_KEY_LEN);
        pthread_mutex_unlock(&handle->key_mutex);
        usleep(500000);
    }
    return NULL;
}

int init_key_generator(asln_handle_t* handle) {
    if (!handle) return -1;
    generate_dynamic_key(handle, handle->current_key);
    return 0;
}

int get_current_key(asln_handle_t* handle, uint8_t* output) {
    if (!handle || !output) return -1;
    pthread_mutex_lock(&handle->key_mutex);
    memcpy(output, handle->current_key, DYNAMIC_KEY_LEN);
    pthread_mutex_unlock(&handle->key_mutex);
    return 0;
}

int start_key_rotation(asln_handle_t* handle) {
    if (!handle) return -1;
    handle->running = 1;
    return pthread_create(&handle->key_thread, NULL, key_rotation_thread, handle);
}

int stop_key_rotation(asln_handle_t* handle) {
    if (!handle) return -1;
    handle->running = 0;
    pthread_join(handle->key_thread, NULL);
    return 0;
}
