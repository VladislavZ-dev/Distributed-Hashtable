/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "../include/client_network.h"
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/stats.h"

struct rtable_t *rtable;

void handle_sigint(int signum) {
    printf("\nBye, bye!\n");
    rtable_disconnect(rtable);
    exit(0);
}

int main(int argc, char *argv[]) {
    
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handle_sigint);

    if (argc != 2) {
        fprintf(stderr, "Used: %s <server>:<port>\n", argv[0]);
        return -1;
    }

    rtable = rtable_connect(argv[1]);

    if (rtable == NULL) {
        perror("Failed to connect!");
        return -1;
    }

    while (1) {

        char command[1024];
        
        printf("Command: ");

        if (fgets(command,sizeof(command), stdin) == NULL) break;

        command[strcspn(command, "\n")] = 0;

        char *com = strtok(command, " ");
        if (com == NULL) {
            continue;
        } 

        if (strcmp(com, "put") == 0 || strcmp(com, "p") == 0) {
            // put command
            char *key = strtok(NULL, " ");
            char *value = strtok(NULL, "");
            if (key == NULL || value == NULL) {
                fprintf(stderr, "Invalid arguments. Usage: put <key> <value>\n");
                continue;
            }

            char* key_entry = malloc(strlen(key)+1);
            strcpy(key_entry, key);
            char* value_block = malloc(strlen(value)+1);
            strcpy(value_block, value);

            struct block_t *block_value = block_create(strlen(value), value_block);
            struct entry_t *entry = entry_create(key_entry, block_value);

            if (rtable_put(rtable, entry) != 0) {
                perror("Failed to add entry...");
            }

            entry_destroy(entry);

        } else if (strcmp(com, "get") == 0 || strcmp(com, "g") == 0) {
            // get command
            char* key = strtok(NULL, " ");
            if (key == NULL) {
                fprintf(stderr, "Invalid arguments. Usage: get <key>\n");
                continue;
            }

            struct block_t *value = rtable_get(rtable, key);

            if (value != NULL) {
                char original_string[value->datasize + 1]; 
                memcpy(original_string, value->data, value->datasize);
                original_string[value->datasize] = '\0';  
                printf("%s\n", original_string);  
            } else {
                printf("Error in rtable_get or key not found!\n");
            }

            block_destroy(value);

        } else if (strcmp(com, "del") == 0 || strcmp(com, "d") == 0) {
            // del command
            char *key = strtok(NULL, " ");
            if (key == NULL) {
                fprintf(stderr, "Invalid arguments. Usage: del <key>\n");
                continue;
            }

            if (rtable_del(rtable, key) == 0) {
                printf("Entry removed successfully.\n");
            } else {
                printf("Error in rtable_del or key not found!\n");
            }

        } else if (strcmp(com, "size") == 0 || strcmp(com, "s") == 0) {
            // size command
            int size = rtable_size(rtable);
            if (size != -1) {
                printf("Table size: %d\n", size);
            } else {
                perror("Error retrieving table size");
            }

        } else if (strcmp(com, "getkeys") == 0 || strcmp(com, "k") == 0) {
            // getkeys command
            char **keys = rtable_get_keys(rtable);
            if (keys != NULL) {
                for (int i = 0; keys[i] != NULL; i++) {
                    printf("%s\n", keys[i]);
                }
                rtable_free_keys(keys); 
            } else {
                perror("Error retrieving keys!");
            }

        } else if (strcmp(com, "gettable") == 0 || strcmp(com, "t") == 0) {
            // gettable command
            struct entry_t **entries = rtable_get_table(rtable);
            if (entries != NULL) {
                for (int i = 0; entries[i] != NULL; i++) {
                    char original_string[entries[i]->value->datasize + 1]; 
                    memcpy(original_string, entries[i]->value->data, entries[i]->value->datasize);
                    original_string[entries[i]->value->datasize] = '\0';  
                    printf("%s :: ", entries[i]->key);
                    printf("%s\n", original_string);
                }
                rtable_free_entries(entries);
            } else {
                perror("Error retrieving table entries");
            }

        } else if (strcmp(com, "stats") == 0) {
            // stats command
            struct statistics_t *stats = rtable_stats(rtable);
            if (stats != NULL) {
                print_stats(stats);
                free(stats);
            } else {
                perror("Error retrieving table stats");
                free(stats);
            }
            
        } else if (strcmp(com, "quit") == 0 || strcmp(com, "q") == 0) {
            // quit command
            printf("Bye, bye!\n");
            break;

        } else {
            fprintf(stderr, "Unknown command, enter a valid command:\n");
            printf("Usage: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | q[uit] | stats\n");
        }
        sleep(0.5);
    }
    rtable_disconnect(rtable);
    return 0;
}