# UDP File Transfer System Documentation

## Introduction
This document provides an explanation of the UDP-based file transfer system, comprising a client and a server application. The system uses UDP sockets for communication and supports file upload (PUT), file download (GET), and client authentication.

## Prerequisites
- Windows operating system
- Winsock library (ws2_32.lib)

---

## Usage Instructions
1. Compile the source code with gcc with the flag `lwsock32`
```bash
gcc server.c -lwsock32 -o server
gcc client.c -lwsock32 -o client
```
2. Run the server application.
   ```bash
   ./server.exe
   ```
3. Run the client application and provide credentials.
   ```bash
   ./client.exe
   ```
3. Use GET or PUT commands to transfer files.
   ```bash
   GET filename.txt
   PUT newfile.txt
   ```

## Server Code Explanation
The server code initializes a UDP socket and listens for incoming connections. It authenticates users and processes GET and PUT commands.

### Key Functionalities
1. **Initialization of Winsock:**
   ```c
   WSAStartup(MAKEWORD(2, 2), &wsa);
   ```
   Initializes the Winsock library.

2. **Socket Creation:**
   ```c
   sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   ```
   Creates a UDP socket.

3. **Binding the Socket:**
   ```c
   bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
   ```
   Binds the socket to an IP address and port.

4. **Authentication:**
   The server authenticates clients by comparing received credentials with hard-coded ones.

5. File Handling - GET and PUT with Edge Case Handling:
   - GET: Sends the requested file to the client, handling cases where the file does not exist by sending a 'FILE_NOT_FOUND' response.
   - PUT: Receives a file from the client and stores it locally, handling cases where file creation fails.
   
---

## Client Code Explanation
The client program interacts with the server for file transfer and authentication.

### Key Functionalities
1. **Authentication:**
   The client prompts the user for a username and password, sending them to the server for verification.

   ```c
   sprintf(buffer, "%s %s", username, password);
   sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
   ```

2. **Sending Files (PUT Command):**
   Reads the file in binary mode and sends it in chunks.

   ```c
   while (!feof(file)) {
       bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
       sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)&server_addr, addr_len);
   }
   ```

3. **Receiving Files (GET Command):**
   The client requests a file from the server and writes the received data to a local file.

   ```c
   while ((bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len)) > 0) {
       fwrite(buffer, 1, bytes_received, file);
   }
   ```

---


## Conclusion
This file transfer system demonstrates the use of UDP for transmitting files between a client and a server. The authentication feature ensures secure file access.

