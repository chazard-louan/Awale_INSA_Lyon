/*
 * Client pour les sockets
 * Usage: socket_client ip_server port
 */

#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    int sockfd;
    char c;

    if (argc != 3) {
        fprintf(stderr, "Usage: socket_client server port\n");
        return EXIT_FAILURE;
    }

    printf("Client starting...\n");

    // Initialize server address
    struct sockaddr_in serv_addr = {0};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid server IP address\n");
        return EXIT_FAILURE;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Connected to server!\n");

    // Username authentication
    int usernameValid = 0;

    while (!usernameValid) {

        char username[USERNAME_SIZE] = {0};

        printf("Enter username (maximum 60 characters): ");

        if (fgets(username, sizeof(username), stdin) == NULL) {
            close(sockfd);
            return EXIT_FAILURE;
        }

        // Remove newline
        username[strcspn(username, "\n")] = '\0';

        if (strlen(username) == 0) {
            printf("Username cannot be empty.\n");
            continue;
        }

        // Send fixed-size username
        ssize_t sent = send(sockfd, username, sizeof(username), 0);

        if (sent != sizeof(username)) {
            perror("send");
            close(sockfd);
            return EXIT_FAILURE;
        }

        // Receive server response
        char response;

        ssize_t received = recv(sockfd, &response, sizeof(response), MSG_WAITALL);

        if (received != sizeof(response)) {
            printf("Server disconnected or invalid response.\n");
            close(sockfd);
            return EXIT_FAILURE;
        }

        if (response == 1) {
            usernameValid = 1;
            printf("Username accepted! Welcome %s.\n", username);
        }
        else {
            printf("Username already in use. Try another one.\n");
        }
    }

    // Main communication loop
    printf("You can now communicate with the server.\n");
    printf("Press 'q' to disconnect.\n");

    while (1) {

        c = getchar();

        if (c == EOF || c == 'q') {
            break;
        }

        if (send(sockfd, &c, sizeof(c), 0) <= 0) {
            perror("send");
            break;
        }
    }

    close(sockfd);

    printf("Disconnected from server.\n");

    return EXIT_SUCCESS;
}
