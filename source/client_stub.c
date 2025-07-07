/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/inet.h"
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/htmessages.pb-c.h"
#include "../include/client_network.h"
#include "../include/stats.h"

struct rtable_t *rtable_connect(char *address_port) {
    struct rtable_t *rtable = (struct rtable_t *) malloc(sizeof(struct rtable_t));
    int sockfd;

    // Hostname
    char *address_port_substring = strtok(address_port, ":");
    if (address_port_substring == NULL) {
        return NULL;
    }
    int length = strlen(address_port_substring);
    rtable->server_address = (char *) malloc(length*sizeof(char) + 1);
    if (rtable->server_address == NULL) {
        return NULL;
    }
    strcpy(rtable->server_address, address_port_substring);

    // Port
    address_port_substring = strtok(NULL, ":");
    if (address_port_substring == NULL) {
        return NULL;
    }
    rtable->server_port = atoi(address_port_substring);

    // Sockfd
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro a criar socket TCP");
        return NULL;    
    }
    rtable->sockfd = sockfd;

    if (network_connect(rtable) < 0 ) {
        return NULL;
    }

    return rtable;
}

int rtable_disconnect(struct rtable_t *rtable) {
    if (network_close(rtable) < 0) {
        return -1;
    }
    free(rtable->server_address);
    close(rtable->sockfd);
    free(rtable);

    return 0;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    StatisticsT stats = STATISTICS_T__INIT;
    msg.stats = &stats;
    
    EntryT msg_entry = ENTRY_T__INIT;

    msg.entry = &msg_entry; 
    if (msg.entry == NULL) {
        return -1;
    }

    msg.entry->key = malloc(strlen(entry->key) + 1);
    if (msg.entry->key == NULL) {
        return -1;
    }
    strcpy(msg.entry->key, entry->key);

    msg.entry->value.len = entry->value->datasize;
    msg.entry->value.data = malloc((entry->value->datasize));
    if (msg.entry->value.data == NULL) {
        free(msg.entry->key);
        return -1;
    }
    memcpy(msg.entry->value.data, entry->value->data, entry->value->datasize);

    MessageT *returnMsg = network_send_receive(rtable, &msg);
    if (returnMsg == NULL) {
        free(msg.entry->key);
        free(msg.entry->value.data);
        return -1;
    }

    if (returnMsg->opcode == MESSAGE_T__OPCODE__OP_PUT+1 && returnMsg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        free(msg.entry->key);
        free(msg.entry->value.data);
        message_t__free_unpacked(returnMsg, NULL);
        return 0;
    } else {
        free(msg.entry->key);
        free(msg.entry->value.data);
        message_t__free_unpacked(returnMsg, NULL);
        return -1;
    }
}

struct block_t *rtable_get(struct rtable_t *rtable, char *key) {
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    StatisticsT stats = STATISTICS_T__INIT;
    msg.stats = &stats;

    msg.key = malloc(strlen(key) + 1);
    if (msg.key == NULL) {
        return NULL;
    }
    strcpy(msg.key, key);

    MessageT *returnMsg = network_send_receive(rtable, &msg);
    if (returnMsg == NULL) {
        free(msg.key);
        return NULL;
    }

    if (returnMsg->c_type == MESSAGE_T__C_TYPE__CT_VALUE) {
        void *data_to_block = malloc(returnMsg->value.len);
        memcpy(data_to_block, returnMsg->value.data, returnMsg->value.len);

        struct block_t *returnBlock = block_create(returnMsg->value.len, data_to_block);

        message_t__free_unpacked(returnMsg, NULL);
        free(msg.key);

        return returnBlock;
    } else {
        message_t__free_unpacked(returnMsg, NULL);
        free(msg.key);
        return NULL;
    }

}

int rtable_del(struct rtable_t *rtable, char *key) {
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    StatisticsT stats = STATISTICS_T__INIT;
    msg.stats = &stats;

    msg.key = malloc(strlen(key) + 1);
    if (msg.key == NULL) {
        return -1;
    }
    strcpy(msg.key, key);

    MessageT *returnMsg = network_send_receive(rtable, &msg);
    if (returnMsg == NULL) {
        free(msg.key);
        return -1;
    }
    free(msg.key);

    if (returnMsg->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(returnMsg, NULL);
        return -1;
    } else {
        message_t__free_unpacked(returnMsg, NULL);
        return 0;
    }

}

int rtable_size(struct rtable_t *rtable) {
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    StatisticsT stats = STATISTICS_T__INIT;
    msg.stats = &stats;

    MessageT *returnMsg = network_send_receive(rtable, &msg);
    if (returnMsg == NULL) {
        return -1;
    }

    if (returnMsg->opcode == MESSAGE_T__OPCODE__OP_SIZE+1 && returnMsg->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {
        int n = returnMsg->result;
        message_t__free_unpacked(returnMsg, NULL);
        return n;
    } else {
        message_t__free_unpacked(returnMsg, NULL);
        return -1;
    }
}

char **rtable_get_keys(struct rtable_t *rtable) {
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    StatisticsT stats = STATISTICS_T__INIT;
    msg.stats = &stats;

    MessageT *returnMsg = network_send_receive(rtable, &msg);
    if (returnMsg == NULL) {
        return NULL;
    }

    if (returnMsg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS+1 && returnMsg->c_type == MESSAGE_T__C_TYPE__CT_KEYS) {
        char** keys = malloc((returnMsg->n_keys)*sizeof(char *) + sizeof(char *));
        if (keys == NULL) {
            return NULL;
        }
        for (int i = 0; i < returnMsg->n_keys; i++) {
            keys[i] = malloc(strlen(returnMsg->keys[i]) + 1);
            strcpy(keys[i], returnMsg->keys[i]);
        }
        keys[returnMsg->n_keys] = NULL;        
        message_t__free_unpacked(returnMsg, NULL);

        return keys;
    } else {
        message_t__free_unpacked(returnMsg, NULL);
        return NULL;
    }
}

void rtable_free_keys(char **keys) {
    int n = 0;
    while (keys[n] != NULL) {
        free(keys[n]);
        n++;
    }
    free(keys[n]);
    free(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    StatisticsT stats = STATISTICS_T__INIT;
    msg.stats = &stats;

    MessageT *returnMsg = network_send_receive(rtable, &msg);
    if (returnMsg == NULL) {
        return NULL;
    }
    
    if (returnMsg->opcode == MESSAGE_T__OPCODE__OP_GETTABLE+1 && returnMsg->c_type == MESSAGE_T__C_TYPE__CT_TABLE) {
        struct entry_t **entries = malloc((returnMsg->n_entries)*sizeof(struct entry_t *) + sizeof(struct entry_t *));
        if (entries == NULL) {
            return NULL;
        }
        for (int i = 0; i < returnMsg->n_entries; i++) {
            char* entry_key = malloc(strlen(returnMsg->entries[i]->key) + 1);
            strcpy(entry_key, returnMsg->entries[i]->key);
            void *entry_data = malloc(returnMsg->entries[i]->value.len);
            memcpy(entry_data, returnMsg->entries[i]->value.data, returnMsg->entries[i]->value.len);

            entries[i] = entry_create(entry_key, block_create(returnMsg->entries[i]->value.len, entry_data));
        }
        entries[returnMsg->n_entries] = NULL;

        message_t__free_unpacked(returnMsg, NULL);

        return entries;
    } else {
        message_t__free_unpacked(returnMsg, NULL);
        return NULL;
    }

}

void rtable_free_entries(struct entry_t **entries) {
    int n = 0;
    while (entries[n] != NULL) {
        entry_destroy(entries[n]);
        n++;
    }
    free(entries[n]);
    free(entries);
}

//função pra ir buscar as stats
struct statistics_t *rtable_stats(struct rtable_t *rtable) {
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_STATS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    StatisticsT stats = STATISTICS_T__INIT;
    msg.stats = &stats;

    MessageT *returnMsg = network_send_receive(rtable, &msg);
    if (returnMsg == NULL) {
        return NULL;
    }

    if (returnMsg->opcode == MESSAGE_T__OPCODE__OP_STATS+1 && returnMsg->c_type == MESSAGE_T__C_TYPE__CT_STATS) {
        struct statistics_t *returnStats = malloc(sizeof(struct statistics_t));
        stats_complete(returnStats, returnMsg->stats->online_clients, returnMsg->stats->operations_made, returnMsg->stats->time);
        message_t__free_unpacked(returnMsg, NULL);
        return returnStats;
    } else {
        message_t__free_unpacked(returnMsg, NULL);
        return NULL;
    }
}