#include "../include/asln.h"
#include "../include/cache.h"
#include "../include/exif_cleaner.h"
#include "../include/agency_data.h"

static asln_handle_t g_handle = {0};
static int g_running = 1;
asln_handle_t* g_asln_handle = &g_handle;

void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
    printf("\n[!] Shutting down ASLN...\n");
}

void print_banner(void) {
    printf("\n");
    printf("   █████╗ ███████╗██╗     ███╗   ██╗\n");
    printf("  ██╔══██╗██╔════╝██║     ████╗  ██║\n");
    printf("  ███████║███████╗██║     ██╔██╗ ██║\n");
    printf("  ██╔══██║╚════██║██║     ██║╚██╗██║\n");
    printf("  ██║  ██║███████║███████╗██║ ╚████║\n");
    printf("  ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝  ╚═══╝\n");
    printf("\n");
    printf("                    SUPER LAYER NETWORK\n");
    printf("\n");
    printf("            Powered by Atomic Super Layer Network Community\n");
    printf("\n");
    printf("  Atomic Super Layer Network v%s\n", ASLN_VERSION);
    printf("  Local Cybersecurity Tool for Professionals\n");
    printf("\n");
}

void print_status(void) {
    printf("\n[STATUS]\n");
    printf("────────\n");
    printf("Active: %s\n", g_handle.active ? "YES" : "NO");
    if (g_handle.active) {
        agency_info_t* agency = &agencies[g_handle.current_agency];
        printf("Agency: %s\n", agency->display_name);
        uint8_t key[DYNAMIC_KEY_LEN];
        if (get_current_key(&g_handle, key) == 0) {
            printf("Dynamic Key: ");
            for (int i = 0; i < 8; i++) printf("%02x", key[i]);
            printf("...\n");
        }
        printf("Key Rotation: %d ms\n", KEY_ROTATION_MS);
    }
    printf("\n");
}

int connect_agency(agency_t agency) {
    agency_info_t* info = &agencies[agency];
    
    if (!info->always_available) {
        time_t now = time(NULL);
        struct tm* tm = localtime(&now);
        if (agency == AGENCY_CNSA) {
            if (!(tm->tm_mon + 1 == 4 && tm->tm_mday == 24) && 
                !(tm->tm_mon + 1 == 10 && tm->tm_mday == 1)) {
                fprintf(stderr, "[!] CNSA only available on Apr 24 or Oct 1\n");
                return -1;
            }
        } else if (agency == AGENCY_ROSCOSMOS) {
            if (!(tm->tm_mon + 1 == 4 && tm->tm_mday == 12) && 
                !(tm->tm_mon + 1 == 5 && tm->tm_mday == 9) && 
                !(tm->tm_mon + 1 == 6 && tm->tm_mday == 12)) {
                fprintf(stderr, "[!] Roscosmos only available on Apr 12, May 9, Jun 12\n");
                return -1;
            }
        }
    }
    
    if (g_handle.active) stop_key_rotation(&g_handle);
    pthread_mutex_init(&g_handle.key_mutex, NULL);
    g_handle.current_agency = agency;
    if (init_key_generator(&g_handle) != 0) return -1;
    if (start_key_rotation(&g_handle) != 0) return -1;
    g_handle.active = 1;
    printf("[+] Connected to %s\n", info->display_name);
    int idx = rand() % info->location_count;
    printf("[+] IP appears as: %s\n", info->locations[idx]);
    printf("[+] Dynamic key rotation active (every 0.5s)\n");
    return 0;
}

void disconnect(void) {
    if (g_handle.active) {
        stop_key_rotation(&g_handle);
        g_handle.active = 0;
        pthread_mutex_destroy(&g_handle.key_mutex);
        printf("[+] Disconnected. IP protection removed.\n");
    }
}

void show_agencies(void) {
    printf("\n[AGENCIES]\n");
    printf("─────────\n");
    for (int i = 0; i < agency_count; i++) {
        agency_info_t* a = &agencies[i];
        printf("  %s %s", a->always_available ? "[ALWAYS]" : "[SPECIAL]", a->display_name);
        if (!a->always_available) printf(" (Special Events Only)");
        printf("\n");
        printf("      Locations: ");
        for (int j = 0; j < a->location_count; j++) printf("%s%s", a->locations[j], (j < a->location_count-1) ? ", " : "");
        printf("\n");
    }
    printf("\n");
}

void print_help(void) {
    printf("\n[COMMANDS]\n");
    printf("──────────\n");
    printf("  asln connect <agency>  - NASA, SpaceX, ESA, CNSA, Roscosmos\n");
    printf("  asln disconnect        - Disconnect\n");
    printf("  asln status            - Show status\n");
    printf("  asln agencies          - List all agencies\n");
    printf("  asln clean <file>      - Remove metadata from image\n");
    printf("  asln batch <dir>       - Batch clean images\n");
    printf("  asln version           - Show version\n");
    printf("  asln help              - This help\n");
    printf("\n");
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    srand(time(NULL));
    memset(&g_handle, 0, sizeof(g_handle));
    print_banner();
    
    if (argc < 2) { print_help(); return 0; }
    
    if (strcmp(argv[1], "connect") == 0 && argc >= 3) {
        int ret = -1;
        if (strcmp(argv[2], "nasa") == 0) ret = connect_agency(AGENCY_NASA);
        else if (strcmp(argv[2], "spacex") == 0) ret = connect_agency(AGENCY_SPACEX);
        else if (strcmp(argv[2], "esa") == 0) ret = connect_agency(AGENCY_ESA);
        else if (strcmp(argv[2], "cnsa") == 0) ret = connect_agency(AGENCY_CNSA);
        else if (strcmp(argv[2], "roscosmos") == 0) ret = connect_agency(AGENCY_ROSCOSMOS);
        else { fprintf(stderr, "[!] Unknown agency: %s\n", argv[2]); return 1; }
        if (ret != 0) return 1;
        while (g_running && g_handle.active) sleep(1);
        if (g_handle.active) disconnect();
    } else if (strcmp(argv[1], "disconnect") == 0) {
        disconnect();
    } else if (strcmp(argv[1], "status") == 0) {
        print_status();
    } else if (strcmp(argv[1], "agencies") == 0) {
        show_agencies();
    } else if (strcmp(argv[1], "clean") == 0 && argc >= 3) {
        char output[512];
        snprintf(output, sizeof(output), "%s.clean", argv[2]);
        if (strip_exif(argv[2], output) == 0) {
            rename(output, argv[2]);
            printf("[+] Cleaned: %s\n", argv[2]);
        } else {
            fprintf(stderr, "[!] Failed: %s\n", argv[2]);
        }
    } else if (strcmp(argv[1], "batch") == 0 && argc >= 3) {
        int count = batch_strip_exif(argv[2]);
        if (count >= 0) printf("[+] Batch cleaned %d images in: %s\n", count, argv[2]);
        else fprintf(stderr, "[!] Batch failed: %s\n", argv[2]);
    } else if (strcmp(argv[1], "version") == 0) {
        printf("ASLN v%s\n", ASLN_VERSION);
    } else if (strcmp(argv[1], "help") == 0) {
        print_help();
    } else {
        fprintf(stderr, "[!] Unknown: %s\n", argv[1]);
        print_help();
        return 1;
    }
    return 0;
}
