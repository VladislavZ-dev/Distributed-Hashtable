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

#include "../include/inet.h"
#include "../include/server_network.h"
#include "../include/server_skeleton.h"
#include "../include/message-private.h"

int online_clients;

typedef struct {
    int connsockfd;
    struct table_t *table;
} thread_arg;


int server_network_init(short port) {
    int sockfd;
    int opt = 1;
    struct sockaddr_in server;
    online_clients = 0;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error defining SO_REUSEADDR");
        close(sockfd);
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Error creating bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 0) < 0) {
        perror("Error executing lister");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// Nao sei se temos de fazer no .h também
void *handle_client(void *arg) {
    printf("Client connection established\n");
    thread_arg *connsockfd_arg = (thread_arg *) arg;
    //incrementar nº clients
    online_clients++;
    MessageT *msg;
    while ((msg = network_receive(connsockfd_arg->connsockfd)) != NULL) {
        //comunicar na mensagem o nº de clientes
        msg->stats->online_clients = online_clients;
        invoke(msg, connsockfd_arg->table);
        if (network_send(connsockfd_arg->connsockfd, msg) == -1) {
            message_t__free_unpacked(msg, NULL);
            break;
        }
    }
    close(connsockfd_arg->connsockfd);
    free(arg);
    //decrementar nº clients
    online_clients--;
    return NULL;
}

int network_main_loop(int listening_socket, struct table_t *table) {
    int connsockfd;
    struct sockaddr_in client;
    socklen_t client_size = sizeof(client);
    printf("Server ready, waiting for connections\n");
    
    while (1) {
        connsockfd = accept(listening_socket, (struct sockaddr *) &client, &client_size);       
        if(connsockfd != -1) {
            pthread_t thr;
            thread_arg *connsockfd_arg = malloc(sizeof(thread_arg)); // Free() made in handle_client()
            connsockfd_arg->connsockfd = connsockfd;
            connsockfd_arg->table = table;
            pthread_create(&thr, NULL, &handle_client, connsockfd_arg);
            pthread_detach(thr);                      
        }
    }
    server_network_close(listening_socket);
    return -1;
}

MessageT *network_receive(int client_socket) {
    uint16_t msg_len = 0;

    int temp = read_all(client_socket, &msg_len, sizeof(msg_len));

    msg_len = ntohs(msg_len);

    uint8_t *buffer = malloc(msg_len);
    if (buffer == NULL) {
        perror("Error initializing buffer");
        return NULL;
    }

    size_t read_msg = read_all(client_socket, buffer, msg_len);
    if (read_msg == 0) {
        printf("Client connection closed\n");
        free(buffer);
        return NULL;
    } else if (read_msg == -1) {
        perror("Error reading message by server");
        free(buffer);
        return NULL;
    }

    MessageT *message = message_t__unpack(NULL, read_msg, buffer);
    if (message == NULL) {
        perror("Error unpacking message by server");
        free(buffer);
        return NULL;
    }

    free(buffer);
    return message;
}

int network_send(int client_socket, MessageT *msg) {

    unsigned int msg_size = message_t__get_packed_size(msg);

    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL) {
        perror("Buffer failed to malloc");
        return -1;
    }

    message_t__pack(msg, buffer);
    uint16_t len_network_order = htons(msg_size);

    if (write_all(client_socket, &len_network_order, sizeof(len_network_order)) != sizeof(len_network_order)) {
        perror("server failed to send message");
        free(buffer);
        return -1;
    }

    if (write_all(client_socket, buffer, msg_size) != msg_size) {
        perror("server failed to send message");
        free(buffer);
        return -1;
    }

    message_t__free_unpacked(msg, NULL);
    free(buffer);
    
    return 0;
}

int server_network_close(int socket) {
    if (close(socket) < 0) {
        return-1;
    }
    return 0;
}