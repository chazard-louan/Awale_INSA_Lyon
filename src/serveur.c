//
// Created by louan on 21/10/2026.
//

/* Serveur sockets TCP
 * affichage de ce qui arrive sur la socket
 *    socket_server port (port > 1024 sauf root)
 */

#include "serveur.h"
#include "liste_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

TableClient* tableClient;

void *comm_client(void *args)
{
  int fd = *(int *)args;
  free(args);

  // give an id to the player needs to be thread safe

  printf("We are in a thread.\n");

  char buffer[BUFFER_SIZE] = {0};

  char username[USERNAME_SIZE] = {0};
  int username_good = 0;
  while (!username_good)
  {
    ssize_t received = recv(fd, username, sizeof(username), MSG_WAITALL);

    if (received != sizeof(username)) {
      char response = 0;
      send(fd, &response, sizeof(response), 0);
    }

    if (username[USERNAME_SIZE - 1] != '\0') {
      char response = 0;
      send(fd, &response, sizeof(response), 0);
    }

    // Check whether username already exists
    //thread safe
    pthread_mutex_lock(&tableClient->mutex);
    while (tablePleine(tableClient)) {pthread_cond_wait(&tableClient->cond, &tableClient->mutex);}


    if (!nomDejaPris(tableClient, username))
    {
      ajoutNouveauClient(tableClient, fd, username);
      username_good = 1;
    }
    else
    {
      char response = 0;
      send(fd, &response, sizeof(response), 0);
    }
    pthread_cond_signal(&tableClient->cond);
    pthread_mutex_unlock(&tableClient->mutex);
  }

  char response = 1;

  if (send(fd, &response, sizeof(response), 0) != sizeof(response)) {
    close(fd);
    return NULL;
  }

  printf("Client connected with username: %s\n", username);

  // TODO: Handle client communication here

  // TODO : Wait for the client to ask something like knowing list of players

  // TODO : respond with the list of player and ids so the clients can respond back for starting a match with another client

  close(fd);
  return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: socket_server <port>\n");
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);

    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        return EXIT_FAILURE;
    }

    printf("Server starting on port %d...\n", port);

    // Create listening socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Initialize server address
    struct sockaddr_in serv_addr = {0};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Start listening
    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("listen");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Server listening...\n");


    tableClient = initialiserTableClient(MAX_PLAYER_ONLINE);
    int end = 0;
    while (!end) {
        struct sockaddr_in cli_addr = {0};
        socklen_t clilen = sizeof(cli_addr);

        // Wait for a client
        int scomm = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (scomm < 0) {
            if (errno == EINTR)
                continue;

            perror("accept");
            continue;
        }

        printf("Connection accepted!\n");

        // Allocate memory for the client socket descriptor
        int *client_fd = malloc(sizeof(int));

        if (client_fd == NULL) {
            perror("malloc");
            close(scomm);
            continue;
        }

        *client_fd = scomm;

        pthread_t threadComm;

        // Create communication thread
        int err = pthread_create(
            &threadComm,
            NULL,
            comm_client,
            client_fd
        );

        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));

            free(client_fd);
            close(scomm);
            continue;
        }

        // Detach thread (no need to join it later)
        pthread_detach(threadComm);
    }
    destructTableClient(tableClient);

    close(sockfd);
    return EXIT_SUCCESS;
}
