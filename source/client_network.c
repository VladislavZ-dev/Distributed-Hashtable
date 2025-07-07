/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../include/client_network.h"
#include "../include/client_stub-private.h"
#include "../include/message-private.h"

int network_connect(struct rtable_t *rtable) {
    struct sockaddr_in server;

    rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rtable->sockfd < 0) {
        perror("Error creating socket");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);

    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) <= 0) {
        perror("Error to convert IP address");
        close(rtable->sockfd);
        return -1;
    }

    if (connect(rtable->sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error connecting to the server");
        close(rtable->sockfd);
        return -1;
    }

    printf("Connection made with server :D \n");
    return 0;
}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    uint8_t *buffer;
    unsigned int len = message_t__get_packed_size(msg);
    buffer = malloc(len);
    if (buffer == NULL) {
        perror("Error alocating memory (buffer from network_send_receive)");
        return NULL;
    }

    message_t__pack(msg, buffer);
    uint16_t len_network_order = htons(len);

    if (write_all(rtable->sockfd, &len_network_order, sizeof(len_network_order)) != sizeof(len_network_order)) {
        perror("Error sending size of message");
        free(buffer);
        return NULL;
    }

    if (write_all(rtable->sockfd, buffer, len) != len) {
        perror("Error sending serialized message");
        free(buffer);
        return NULL;
    }
    free(buffer);
    uint16_t response_len;
    if (read_all(rtable->sockfd, &response_len, sizeof(response_len)) != sizeof(response_len)) {
        perror("Error reading size of response");
        return NULL;
    }
    response_len = ntohs(response_len);

    uint8_t *response_buffer = malloc(response_len);
    if (response_buffer == NULL) {
        perror("Error alocating response memory");
        return NULL;
    }

    if (read_all(rtable->sockfd, response_buffer, response_len) != response_len) {
        perror("Error reading server response");
        free(response_buffer);
        return NULL;
    }
    MessageT *response = message_t__unpack(NULL, response_len, response_buffer);
    free(response_buffer);
    if (response == NULL) {
        perror("Error deserializing response");
        return NULL;
    }

    return response;
}

int network_close(struct rtable_t *rtable) {
    if (close(rtable->sockfd) < 0) {
        perror("Error closing the connection!");
        return -1;
    }
    return 0;
}