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
  int my_id = 0;
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
      my_id = ajoutNouveauClient(tableClient, fd, username);
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
  char charresp;
  ssize_t received = recv(fd, &charresp, sizeof(charresp), MSG_WAITALL);

  if (received != sizeof(charresp)){}
  char msg[MAX_PLAYER_ONLINE*USERNAME_SIZE+1];

  int cursor = 0;
  if (charresp == 'l')
  {
    pthread_mutex_lock(&tableClient->mutex);
    for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
    {
      if (tableClient->table[i] != NULL && tableClient->table[i]->estDisponible)
      {
        strcpy(msg + cursor, (tableClient->table[i])->nomUtilisateur);
        cursor = cursor + strlen(msg) + 1;
      }
    }
    *(msg + cursor) = EOF;
    pthread_cond_signal(&tableClient->cond);
    pthread_mutex_unlock(&tableClient->mutex);

    if (send(fd, &msg, sizeof(msg), 0) != sizeof(msg)) {
      close(fd);
      return NULL;
    }
  }

  received = recv(fd, &charresp, sizeof(charresp), MSG_WAITALL);

  if (received != sizeof(charresp)){}
  int msg_Ids[MAX_PLAYER_ONLINE];
  cursor = 0;
  if (charresp == 'L')
  {
    pthread_mutex_lock(&tableClient->mutex);
    for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
    {
      if (tableClient->table[i] != NULL && tableClient->table[i]->estDisponible)
      {
        msg_Ids[cursor] = i;
        cursor++;
      }
    }
    pthread_cond_signal(&tableClient->cond);
    pthread_mutex_unlock(&tableClient->mutex);
    for (int i = 0; i < cursor; i++)
    {
      //printf("id : %d\n", msg_Ids[i]);
    }


    if (send(fd, &msg_Ids, sizeof(msg_Ids), 0) != sizeof(msg_Ids)) {
      close(fd);
      return NULL;
    }

    int player_id;
    received = recv(fd, &player_id, sizeof(player_id), MSG_WAITALL);
    if (received != sizeof(charresp)){}
    printf("Player:%d -> challenge player:%d\n", my_id, player_id);

    pthread_mutex_lock(&tableClient->mutex);
    if (tableClient->table[player_id] == NULL)
    {
      // player disconected T_T
    }
    int player2_sockfd = tableClient->table[player_id]->sockfd;
    pthread_mutex_unlock(&tableClient->mutex);

    // ask chalenge
    char msg_12 = 'c'; // send challenge
    if(send(player2_sockfd, &msg_12, sizeof(msg_12), 0) != sizeof(msg_12))
    {
      //error
    }
    charresp = 'n';
    received = recv(player2_sockfd, &charresp, sizeof(charresp), MSG_WAITALL);
    if (received != sizeof(charresp)){}

    if (charresp =='y')
    {
      send(fd, &charresp, sizeof(charresp), 0) != sizeof(charresp);
      pthread_mutex_lock(&tableClient->mutex);
      tableClient->table[my_id]->estDisponible = 0;
      tableClient->table[player_id]->estDisponible = 0;
      pthread_mutex_unlock(&tableClient->mutex);

      //start game
      int game_end = 0;
      int coup;
      while (!game_end)
      {
        received = recv(fd, &coup, sizeof(coup), MSG_WAITALL);
        if (coup==0)
        {
          // fin de partie
          game_end = 1;
          continue;
        }
        send(player2_sockfd, &coup, sizeof(coup), 0) != sizeof(charresp);
        received = recv(fd, &coup, sizeof(coup), MSG_WAITALL);
        if (coup==0)
        {
          // fin de partie
          game_end = 1;
          continue;
        }
        send(player2_sockfd, &coup, sizeof(coup), 0) != sizeof(charresp);

      }
      // partie terminé
      pthread_mutex_lock(&tableClient->mutex);
      tableClient->table[my_id]->estDisponible = 1;
      tableClient->table[player_id]->estDisponible = 1;
      pthread_mutex_unlock(&tableClient->mutex);
    }
    if (charresp =='n')
    {
      // inform client challenge refused
      send(fd, &charresp, sizeof(charresp), 0) != sizeof(charresp);
    }
  }
  close(fd);
  return NULL;
}

Serveur* initialiserServeur(int port)
{
  Serveur* serveur;
 

  if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        return NULL;
    }

    printf("Server starting on port %d...\n", port);

    // Create listening socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return NULL;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Initialize server address
    struct sockaddr_in serv_addr = {0};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    serveur=malloc(sizeof(Serveur));
    serveur->port=port;
    serveur->serv_addr=serv_addr;
    serveur->socketfd=sockfd;

    // Bind socket
    if (bind(serveur->socketfd, (struct sockaddr *) &(serveur->serv_addr),
             sizeof(serveur->serv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        free(serveur);
        return NULL;
    }

    // Start listening
    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("listen");
        close(sockfd);
        free(serveur);
        return NULL;
    }
    return serveur;

}

int main(int argc, char **argv)
{
    Serveur* mainServeur;
    if (argc != 2) {
        fprintf(stderr, "Usage: socket_server <port>\n");
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);

    mainServeur=initialiserServeur(port);
    printf("Server listening...\n");


    tableClient = initialiserTableClient(MAX_PLAYER_ONLINE);
    int end = 0;
    while (!end) {
        struct sockaddr_in cli_addr = {0};
        socklen_t clilen = sizeof(cli_addr);

        // Wait for a client
        int scomm = accept(mainServeur->socketfd, (struct sockaddr *)&cli_addr, &clilen);

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

    close(mainServeur->socketfd);
    free(mainServeur);
    return EXIT_SUCCESS;
}