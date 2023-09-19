#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8888
#define BUFFER_SIZE 1024

typedef struct {
    int clientSocket;
    struct sockaddr_in clientAddress;
} ClientInfo;

void* handleClient(void* arg) {
    ClientInfo* clientInfo = (ClientInfo*)arg;
    int clientSocket = clientInfo->clientSocket;
    struct sockaddr_in clientAddress = clientInfo->clientAddress;
    char buffer[BUFFER_SIZE];

    printf("Client connected: %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            // Error or connection closed by client
            break;
        }

        printf("Received message from client: %s\n", buffer);

        // Process the received message (you can add your custom logic here)

        // Send response back to the client
        if (send(clientSocket, buffer, bytesRead, 0) == -1) {
            perror("Error sending response");
            exit(1);
        }
    }

    printf("Client disconnected: %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

    // Close client socket
    close(clientSocket);

    // Free client info memory
    free(clientInfo);

    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength;
    pthread_t threadId;
    ClientInfo* clientInfo;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error creating server socket");
        exit(1);
    }

    // Set up server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Bind socket to server address
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        perror("Error listening for connections");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept incoming connection
        clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        // Create client info structure for passing to the thread
        clientInfo = (ClientInfo*)malloc(sizeof(ClientInfo));
        clientInfo->clientSocket = clientSocket;
        clientInfo->clientAddress = clientAddress;

        // Create a new thread to handle the client connection
        if (pthread_create(&threadId, NULL, handleClient, (void*)clientInfo) != 0) {
            perror("Error creating thread");
            exit(1);
        }

        // Detach the thread to allow it to run independently
        pthread_detach(threadId);
    }

    // Close server socket
    close(serverSocket);

    return 0;
}
