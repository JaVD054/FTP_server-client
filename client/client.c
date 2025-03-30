#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

void send_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename);
void receive_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename);

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Send authentication request
    char username[20], password[20];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    sprintf(buffer, "%s %s", username, password);
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    int addr_len = sizeof(server_addr);
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);

    if (strcmp(buffer, "AUTH_SUCCESS") != 0) {
        printf("Authentication failed\n");
        return 1;
    }

    printf("Authenticated successfully!\n");

    while (1) {
        printf("Enter command (GET <filename> / PUT <filename> / EXIT): ");
        scanf(" %[^\n]", buffer);

        if (strncmp(buffer, "EXIT", 4) == 0) {
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            break;
        }

        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        char command[5], filename[BUFFER_SIZE];
        sscanf(buffer, "%s %s", command, filename);

        if (strcmp(command, "GET") == 0) {
            receive_file(sockfd, server_addr, filename);
        } else if (strcmp(command, "PUT") == 0) {
            send_file(sockfd, server_addr, filename);
        }
    }

    // Cleanup
    closesocket(sockfd);
    WSACleanup();
    return 0;
}

// Function to send a file to the server
void send_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("File not found\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    fclose(file);
    printf("File sent successfully\n");
}

// Function to receive a file from the server
void receive_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) return;

    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, NULL)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
        if (bytes_received < BUFFER_SIZE) break;
    }

    fclose(file);
    printf("File received successfully\n");
}