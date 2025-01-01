#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>
#include <wchar.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024

char username[50];

void *receive_messages(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;
    
    while((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("\r%s\nYou: ", buffer); 
        fflush(stdout); 
    }
    
    if(read_size == 0) {
        printf("Friend disconnected.\n");
    } else if(read_size == -1) {
        perror("recv failed");
    }
    
    return 0;
}

void display_welcome_art() {
    setlocale(LC_CTYPE, "en_US.UTF-8");
    _setmode(_fileno(stdout), _O_U16TEXT); 
    wprintf(L"\n");
    wprintf(L" ██████ ██   ██  █████  ████████ ██ ███████ ██    ██ \n");
    wprintf(L"██      ██   ██ ██   ██    ██    ██ ██       ██  ██  \n");
    wprintf(L"██      ███████ ███████    ██    ██ █████     ████   \n");
    wprintf(L"██      ██   ██ ██   ██    ██    ██ ██         ██    \n");
    wprintf(L" ██████ ██   ██ ██   ██    ██    ██ ██         ██    \n");
    wprintf(L"\nWelcome to Chatify the Console Chat!\n");
    _setmode(_fileno(stdout), _O_TEXT); 
}

int main() {
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE];
    char formatted_message[BUFFER_SIZE + 50];
    pthread_t recv_thread;
    WSADATA wsa;

    
    display_welcome_art();

    
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");

    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Could not create socket : %d" , WSAGetLastError());
    }
    printf("Socket created\n");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("connect failed. Error: %d\n", WSAGetLastError());
        return 1;
    }
    
    printf("Connected\n");

    
    printf("Enter your username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0; 

    
    if (pthread_create(&recv_thread, NULL, receive_messages, (void*)&sock) < 0) {
        perror("could not create thread");
        return 1;
    }

    
    while(1) {
        printf("You: ");
        fgets(message, BUFFER_SIZE, stdin);
        
        message[strcspn(message, "\n")] = 0;
        if (strlen(message) > 0) {
            snprintf(formatted_message, sizeof(formatted_message), "%s: %s", username, message);
            if (send(sock, formatted_message, strlen(formatted_message), 0) < 0) {
                perror("Send failed");
                return 1;
            }
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}


