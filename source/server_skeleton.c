/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/server_skeleton.h"
#include "../include/stats.h"

pthread_rwlock_t rwlock;
long operations_time;
int operations_complete;

struct table_t *server_skeleton_init(int n_lists) {
    struct table_t *table = table_create(n_lists);
    pthread_rwlock_init(&rwlock, NULL);
    operations_time = 0;
    operations_complete = 0;
    return table;
}

int server_skeleton_destroy(struct table_t *table) {
    pthread_rwlock_destroy(&rwlock);
    return table_destroy(table);
}

int invoke(MessageT *msg, struct table_t *table) {
    if (msg->opcode == MESSAGE_T__OPCODE__OP_BAD) {
        msg->opcode = MESSAGE_T__OPCODE__OP_BAD;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return 0;
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT) {
        pthread_rwlock_wrlock(&rwlock);
        long first = get_operation_time_stamp();
        void *data = malloc(msg->entry->value.len);
        memcpy(data, msg->entry->value.data, msg->entry->value.len);
        struct block_t *table_block = block_create(msg->entry->value.len, data);
        if(table_put(table, msg->entry->key, table_block) == 0) {
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            block_destroy(table_block);
            long last = get_operation_time_stamp();
            operations_complete++;
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
        else {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            block_destroy(table_block);
            long last = get_operation_time_stamp();
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return -1;
        }
        pthread_rwlock_unlock(&rwlock);
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_GET) {
        pthread_rwlock_rdlock(&rwlock);
        long first = get_operation_time_stamp();
        struct block_t *value = table_get(table, msg->key);
        if (value != NULL) {
            msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            msg->value.len = value->datasize;
            msg->value.data = malloc(value->datasize);
            if (msg->value.data == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                long last = get_operation_time_stamp();
                operations_time += (last - first);
                pthread_rwlock_unlock(&rwlock);
                return -1;
            }
            memcpy(msg->value.data, value->data, value->datasize);
            block_destroy(value);
            long last = get_operation_time_stamp();
            operations_complete++;
            operations_time += (last - first);       
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
        else {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            block_destroy(value);
            long last = get_operation_time_stamp();
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {
        pthread_rwlock_wrlock(&rwlock);
        long first = get_operation_time_stamp();
        if (table_remove(table, msg->key) == 0) {
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            long last = get_operation_time_stamp();
            operations_complete++;
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
        else {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            long last = get_operation_time_stamp();
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return -1;
        }
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_SIZE) {
        pthread_rwlock_rdlock(&rwlock);
        long first = get_operation_time_stamp();
        int size = table_size(table);
        if (size > -1) {
            msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = size;
            long last = get_operation_time_stamp();
            operations_complete++;
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
        else {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            long last = get_operation_time_stamp();
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return -1;
        }
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS) {
        pthread_rwlock_rdlock(&rwlock);
        long first = get_operation_time_stamp();
        char **keys = table_get_keys(table);
        if (keys != NULL) {
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            
            int length = 0;
            while (keys[length] != NULL) {
                length++;
            }
            msg->keys = malloc(length*sizeof(char *) + sizeof(char *));
            if (keys == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                long last = get_operation_time_stamp();
                operations_time += (last - first);
                pthread_rwlock_unlock(&rwlock);
                return -1;
            }
            for (int i = 0; i < length; i++) {
                msg->keys[i] = malloc(strlen(keys[i]) + 1);
                strcpy(msg->keys[i], keys[i]);
            }
            msg->n_keys = length;                
            table_free_keys(keys);
            long last = get_operation_time_stamp();
            operations_complete++;
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
        else {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            long last = get_operation_time_stamp();
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return -1;
        }
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_GETTABLE) {
        pthread_rwlock_rdlock(&rwlock);
        long first = get_operation_time_stamp();
        if (table != NULL) {
            char **keys = table_get_keys(table);
            msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;

            int n_keys = 0;
            while (keys[n_keys] != NULL) {
                n_keys++;
            }
            msg->entries = malloc(n_keys*sizeof(EntryT*));
            for (int i = 0; i < n_keys; i++) {
                msg->entries[i] = malloc(sizeof(EntryT));
                entry_t__init(msg->entries[i]);
                msg->entries[i]->key = malloc(strlen(keys[i]) + 1);
                strcpy(msg->entries[i]->key, keys[i]);
                struct block_t *currentBlock = table_get(table, keys[i]);
                msg->entries[i]->value.len = currentBlock->datasize;
                msg->entries[i]->value.data = malloc(msg->entries[i]->value.len);
                memcpy(msg->entries[i]->value.data, currentBlock->data, msg->entries[i]->value.len);
                block_destroy(currentBlock);
            }
            msg->n_entries = n_keys;
            table_free_keys(keys);
            long last = get_operation_time_stamp();
            operations_complete++;
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
        else {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            long last = get_operation_time_stamp();
            operations_time += (last - first);
            pthread_rwlock_unlock(&rwlock);
            return -1;
        }
    }

    else if (msg->opcode == MESSAGE_T__OPCODE__OP_STATS) {
        pthread_rwlock_rdlock(&rwlock);
        if (msg->stats->online_clients > 0) {
            msg->opcode = MESSAGE_T__OPCODE__OP_STATS + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;
            msg->stats->operations_made = operations_complete;
            msg->stats->time = operations_time;
            pthread_rwlock_unlock(&rwlock);
            return 0;
        }
        else {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            pthread_rwlock_unlock(&rwlock);
            return -1;
        }

    }

    else {
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return -1;
    }
    
}
