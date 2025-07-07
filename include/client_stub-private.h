/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include "table.h"

struct rtable_t {
    char *server_address;
    int server_port;
    int sockfd;
};

/* Retorna as estatisticas da mensagem
 */
struct statistics_t *rtable_stats(struct rtable_t *rtable);

#endif
