#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

void send_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename,char * msg);
void receive_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename,char * msg);

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

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send authentication
    char username[20], password[20];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    sprintf(buffer, "%s %s", username, password);
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    int addr_len = sizeof(server_addr);
    memset(buffer, 0, sizeof(buffer));

    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);

    if (strcmp(buffer, "AUTH_SUCCESS") != 0) {
        printf("Authentication failed.\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Authenticated successfully!\n");

    while (1) {
        printf("\nEnter command (GET <filename> / PUT <filename> / EXIT): ");
        scanf(" %[^\n]", buffer);

        
        if (strncmp(buffer, "EXIT", 4) == 0) {
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            break;
        }


        char command[5], filename[BUFFER_SIZE];
        sscanf(buffer, "%s %s", command, filename);

        if (strcmp(command, "GET") == 0) {
            receive_file(sockfd, server_addr, filename,buffer);
        } else if (strcmp(command, "PUT") == 0) {
            send_file(sockfd, server_addr, filename,buffer);
        } else {
            printf("Invalid command.\n");
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}

void send_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename,char * msg) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("File not found: %s\n", filename);
        return;
    }
    sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    char buffer[BUFFER_SIZE];
    int bytes_read;
    int addr_len = sizeof(server_addr);

    while (!feof(file)) {
        bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
        int bytes_sent = sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)&server_addr, addr_len);
        printf("Bytes sent: %d\n", bytes_sent);
    }

    // Send termination message
    sendto(sockfd, "END", strlen("END"), 0, (struct sockaddr *)&server_addr, addr_len);

    fclose(file);
    printf("File %s sent successfully.\n", filename);
}

void receive_file(SOCKET sockfd, struct sockaddr_in server_addr, char *filename,char * msg) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int addr_len = sizeof(server_addr);

    sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);

    if (strncmp(buffer, "FILE_NOT_FOUND",14) == 0) {
        printf("File not found on server.\n");
        return;
    }
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error creating file: %s\n", filename);
        return;
    }
    
    
    while (1) {
        bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
        
        printf("Server response: %s\n", buffer);
        
        fwrite(buffer, 1, bytes_received, file);
        
        if (bytes_received < BUFFER_SIZE) {
            break;
            printf("End of file received.\n");
        }

    }

    fclose(file);
    printf("File %s received successfully.\n", filename);
}