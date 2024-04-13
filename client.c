#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main() {
    int clientSocket;
    struct sockaddr_in serverAddressawdawdawdawdawd;
    char buffer[BUFFER_SIZE];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error creating client socket");
        exit(1); }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8888);

    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(1); }


    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error connecting to the server");
        exit(1); }

    printf("Connected to the server. Start typing messages:\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        fgets(buffer, BUFFER_SIZE, stdin);


        if (write(clientSocket, buffer, strlen(buffer)) < 0) {
            perror("Error sending message to the server");
            exit(1); }}

    close(clientSocket);
    return 0; }
