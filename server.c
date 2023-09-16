#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSockets[MAX_CLIENTS];
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[BUFFER_SIZE];
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error creating server socket");
        exit(1); }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8888);


    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error binding server socket");
        exit(1); }


    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Error listening for connections");
        exit(1); }
    
    printf("Server started. Waiting for connections...\n");

    int clientCount = 0;
    int i, activity, newSocket, clientSocket, maxSd;
    int sd;
    int maxClients = MAX_CLIENTS;

    fd_set readFds;


    for (i = 0; i < maxClients; i++) {
        clientSockets[i] = 0; }

    while (1) {
        FD_ZERO(&readFds);


        FD_SET(serverSocket, &readFds);
        maxSd = serverSocket;

        for (i = 0; i < maxClients; i++) {
            sd = clientSockets[i];
            if (sd > 0)
                FD_SET(sd, &readFds);
            if (sd > maxSd)
                maxSd = sd; }

        activity = select(maxSd + 1, &readFds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            printf("Error selecting sockets for activity\n"); }

        if (FD_ISSET(serverSocket, &readFds)) {
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddress)) < 0) {
                perror("Error accepting connection");
                exit(1); }

            printf("New connection, socket fd: %d, IP: %s, port: %d\n", newSocket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

            for (i = 0; i < maxClients; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    break; } } }

        for (i = 0; i < maxClients; i++) {
            clientSocket = clientSockets[i];
            if (FD_ISSET(clientSocket, &readFds)) {
                memset(buffer, 0, BUFFER_SIZE);
                int bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
                if (bytesRead == 0) {
                    printf("Client disconnected, socket fd: %d\n", clientSocket);
                    close(clientSocket);
                    clientSockets[i] = 0; } else {
                    for (i = 0; i < maxClients; i++) {
                        sd = clientSockets[i];
                        if (sd != serverSocket && sd != clientSocket) {
                            write(sd, buffer, strlen(buffer)); } } } } } }

    close(serverSocket);
    return 0; }
