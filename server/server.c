#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <sys/stat.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024
#define USERNAME "user1"
#define PASSWORD "password1"

void handle_get(SOCKET sockfd, struct sockaddr_in client_addr, int addr_len, char *filename);
void handle_put(SOCKET sockfd, struct sockaddr_in client_addr, int addr_len, char *filename);

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int addr_len = sizeof(client_addr);

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    // Bind to address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        return 1;
    }

    printf("Server started on port %d...\n", SERVER_PORT);

    while (1) {
        // Receive authentication request
        recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, USERNAME " " PASSWORD) == 0) {
            sendto(sockfd, "AUTH_SUCCESS", strlen("AUTH_SUCCESS"), 0, (struct sockaddr *)&client_addr, addr_len);
        } else {
            sendto(sockfd, "AUTH_FAILED", strlen("AUTH_FAILED"), 0, (struct sockaddr *)&client_addr, addr_len);
            continue;
        }

        while (1) {
            // Receive command (GET or PUT)
            memset(buffer, 0, sizeof(buffer));
            recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            
            if (strncmp(buffer, "EXIT", 4) == 0) {
                printf("Client disconnected.\n");
                break;
            }

            char command[5], filename[BUFFER_SIZE];
            sscanf(buffer, "%s %s", command, filename);

            if (strcmp(command, "GET") == 0) {
                handle_get(sockfd, client_addr, addr_len, filename);
            } else if (strcmp(command, "PUT") == 0) {
                handle_put(sockfd, client_addr, addr_len, filename);
            }
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}

void handle_get(SOCKET sockfd, struct sockaddr_in client_addr, int addr_len, char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        sendto(sockfd, "FILE_NOT_FOUND", strlen("FILE_NOT_FOUND"), 0, (struct sockaddr *)&client_addr, addr_len);
        return;
    }

    struct stat file_stat;
    stat(filename, &file_stat);
    char file_size_str[20];
    sprintf(file_size_str, "%ld", file_stat.st_size);

    sendto(sockfd, file_size_str, strlen(file_size_str), 0, (struct sockaddr *)&client_addr, addr_len);

    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)&client_addr, addr_len);
    }

    fclose(file);
}

void handle_put(SOCKET sockfd, struct sockaddr_in client_addr, int addr_len, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
        if (bytes_received < BUFFER_SIZE) {
            break;
        }
    }

    fclose(file);
}