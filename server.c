#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0 && client_sockets[i] != sender_sock) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    while((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s\n", buffer);
        broadcast_message(buffer, sock);
    }

    if(read_size == 0) {
        printf("Client disconnected.\n");
    } else if(read_size == -1) {
        perror("recv failed");
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == sock) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    closesocket(sock);
    return 0;
}

int main() {
    int server_sock, client_sock, c;
    struct sockaddr_in server, client;
    pthread_t client_thread;
    WSADATA wsa;

    
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");

    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Could not create socket : %d" , WSAGetLastError());
    }
    printf("Socket created\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        return 1;
    }
    printf("Bind done\n");

    
    listen(server_sock, 3);

    
    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);
    while((client_sock = accept(server_sock, (struct sockaddr *)&client, &c)) != INVALID_SOCKET) {
        printf("Connection accepted\n");

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_sock;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (pthread_create(&client_thread, NULL, handle_client, (void*)&client_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
    }

    if (client_sock == INVALID_SOCKET) {
        printf("accept failed with error code : %d" , WSAGetLastError());
        return 1;
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}
