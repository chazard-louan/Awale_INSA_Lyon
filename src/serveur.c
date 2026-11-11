//
// Created by louan on 21/10/2026.
//

/* Serveur sockets TCP
 * affichage de ce qui arrive sur la socket
 *    socket_server port (port > 1024 sauf root)
 */

#include "serveur.h"
#include "reseau.h"
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



int connectionToServer(Serveur* serveur){
  int clientfd;
  struct sockaddr_in cli_addr = {0};
  socklen_t clilen = sizeof(cli_addr);

  // Wait for a client
  clientfd = accept(serveur->socketfd, (struct sockaddr *)&cli_addr, &clilen);
  

  if (clientfd < 0) {
      if (errno == EINTR)
          return 0;

      perror("accept");
      return 0;
  }
  
  return clientfd;

}

void *cleanup(Client* client)
{
  printf("Client disconnected (ID: %d)\n", client->id);
  disconnectClient(tableClient, client->id);
  close(client->mainSockfd);
  close(client->chatSockfd);
  return NULL;

}

Client* register_client(int mainfd, int chatfd)
{
  char response;
  char username[USERNAME_SIZE] = {0};
  Client* myClient;

  while (1)
  {
    memset(username, 0, USERNAME_SIZE);

    if (recv_all(mainfd, username, USERNAME_SIZE) != 0)
      return NULL;

    // Check username termination
    if (username[USERNAME_SIZE - 1] != '\0')
    {
      response = 0;

      if (send_all(mainfd, &response, sizeof(response)) != 0)
        return NULL;

      continue;
    }

    pthread_mutex_lock(&tableClient->mutex);

    while (cannotTryConnect(tableClient))
    {
      pthread_cond_wait(&tableClient->cond, &tableClient->mutex);
    }

    int id = getExistingUserId(tableClient, username);

    if (id == -1 && !nomDejaPris(tableClient, username))
    {
      myClient = ajoutNouveauClient(tableClient, mainfd,chatfd, username);

      pthread_mutex_unlock(&tableClient->mutex);

      response = 1;

      if (send_all(mainfd, &response, sizeof(response)) != 0)
        return NULL;

      printf("Client added and connected: %s (ID: %d)\n", username, myClient->id);

      return myClient;
    }

    if (tableClient->table[id] != NULL && !tableClient->table[id]->connected)
    {
      connectClient(tableClient, id, mainfd);
      pthread_mutex_unlock(&tableClient->mutex);

      response = 1;

      if (send_all(mainfd, &response, sizeof(response)) != 0)
        return NULL;

      printf("Client connected: %s (ID: %d)\n", username, myClient->id);

      return myClient;
    }
    response = 1;

    if (send_all(mainfd, &response, sizeof(response)) != 0)
      return NULL;
    return myClient;
  }
}
int send_available_players(int fd)
{
  char msg[MAX_PLAYER_ONLINE * USERNAME_SIZE + 1] = {0};
  int cursor = 0;

  pthread_mutex_lock(&tableClient->mutex);

  for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
  {
    if (tableClient->table[i] != NULL &&
        tableClient->table[i]->estDisponible)
    {
      const char *username =
          tableClient->table[i]->nomUtilisateur;

      size_t len = strlen(username) + 1;

      strcpy(msg + cursor, username);
      cursor += len;
    }
  }

  msg[cursor] = (char)EOF;

  pthread_mutex_unlock(&tableClient->mutex);

  return send_all(fd, msg, sizeof(msg));
}

int send_available_player_ids(int fd)
{
  int msg_Ids[MAX_PLAYER_ONLINE];

  for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
    msg_Ids[i] = -1;

  int cursor = 0;

  pthread_mutex_lock(&tableClient->mutex);

  for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
  {
    if (tableClient->table[i] != NULL &&
        tableClient->table[i]->estDisponible)
    {
      msg_Ids[cursor++] = i;
    }
  }

  pthread_mutex_unlock(&tableClient->mutex);

  return send_all(fd, msg_Ids, sizeof(msg_Ids));
}

int get_player_socket(int player_id)
{
  if (player_id < 0 || player_id >= MAX_PLAYER_ONLINE)
    return -1;

  pthread_mutex_lock(&tableClient->mutex);

  if (tableClient->table[player_id] == NULL ||
      !tableClient->table[player_id]->estDisponible)
  {
    pthread_mutex_unlock(&tableClient->mutex);
    return -1;
  }

  int player_fd = tableClient->table[player_id]->mainSockfd;

  pthread_mutex_unlock(&tableClient->mutex);

  return player_fd;
}

void handle_game(int fd1, int fd2)
{
  int coup;

  while (1)
  {
    // Receive move from player 1
    if (recv_all(fd1, &coup, sizeof(coup)) != 0)
      break;

    if (coup == 0)
      break;

    // Forward move to player 2
    if (send_all(fd2, &coup, sizeof(coup)) != 0)
      break;

    // Receive next move from player 1
    if (recv_all(fd2, &coup, sizeof(coup)) != 0)
      break;

    if (coup == 0)
      break;

    // Forward move to player 2
    if (send_all(fd1, &coup, sizeof(coup)) != 0)
      break;
  }
}

void set_players_available(int id1, int id2, int available)
{
  pthread_mutex_lock(&tableClient->mutex);

  if (tableClient->table[id1] != NULL)
    tableClient->table[id1]->estDisponible = available;

  if (tableClient->table[id2] != NULL)
    tableClient->table[id2]->estDisponible = available;

  pthread_cond_broadcast(&tableClient->cond);

  pthread_mutex_unlock(&tableClient->mutex);
}


int handle_challenge(int fd, int my_id, int player_id)
{
  int player2_sockfd = get_player_socket(player_id);

  if (player2_sockfd < 0)
  {
    char refusal = 'n';
    return send_all(fd, &refusal, sizeof(refusal));
  }

  // Send challenge
  char challenge = 'c';

  if (send_all(player2_sockfd, &challenge, sizeof(challenge)) != 0)
    return -1;

  // Receive response
  char response;

  if (recv_all(player2_sockfd, &response, sizeof(response)) != 0)
    return -1;

  if (response == 'n')
  {
    return send_all(fd, &response, sizeof(response));
  }

  if (response != 'y')
    return -1;

  // Notify requesting player
  if (send_all(fd, &response, sizeof(response)) != 0)
    return -1;

  // Both players are now unavailable
  set_players_available(my_id, player_id, 0);

  // Start game
  handle_game(fd, player2_sockfd);

  // Game finished
  set_players_available(my_id, player_id, 1);

  return 0;
}





void *comm_client(void *args)
{
  int mainfd = ((int *)args)[0];
  int chatfd = ((int *)args)[0];
  free(args);
  

  int my_id = 0;

  char username[USERNAME_SIZE] = {0};
  char command;
  Client* myClient;

  // Step 1: Register client 
  if ((myClient=register_client(mainfd,chatfd) )==NULL ){
    close (mainfd);
    close(chatfd);
    return NULL;
  }
    


  while (1)
  {
    // Step 1: Receive first command
    if (recv_all(myClient->mainSockfd, &command, sizeof(command)) != 0)
      return cleanup(myClient);

    if (command == 'l')
    {
      if (send_available_players(myClient->mainSockfd) != 0)
        return cleanup(myClient);
    }

    if (command == 'L')
    {
      if (send_available_player_ids(myClient->mainSockfd) != 0)
        return cleanup(myClient);
    }

    if (command == 'x')
    {

      int player_id;

      if (recv_all(myClient->mainSockfd, &player_id, sizeof(player_id)) != 0)
        return cleanup(myClient);

      printf("Player %d wants to challenge player %d\n",
             my_id, player_id);

      // Step 6: Handle challenge
      if (handle_challenge(myClient->mainSockfd, my_id, player_id) != 0)
        return cleanup(myClient);
    }
  }


  return cleanup(myClient);
}

Serveur* initialiserServeur(int port)
{
  Serveur* serveur;
  Serveur* serveurChat;
 
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
    //serveur->serv_addr=serv_addr;
    serveur->socketfd=sockfd;

    // Bind socket
    if (bind(serveur->socketfd, (struct sockaddr *) &(serv_addr),
             sizeof(serv_addr)) < 0) {
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
    Serveur* serveurChat;

    if (argc != 3) {
        fprintf(stderr, "Usage: socket_server <port>\n");
        return EXIT_FAILURE;
    }

    int mainPort = atoi(argv[1]);
    int chatPort = atoi(argv[2]);
    if ( (mainServeur=initialiserServeur(mainPort))==NULL)
    {
        return EXIT_FAILURE;
    }
        
    printf("Main Server listening...\n");

    if ( (serveurChat=initialiserServeur(chatPort))==NULL)
    {
        close(mainServeur->socketfd);
        free(mainServeur);
        return EXIT_FAILURE;
    }
    printf("Chat Server listening...\n");




    tableClient = initialiserTableClient(MAX_PLAYER_ONLINE);
    int end = 0;
    while (!end) {

        // Wait for a client
        int scomm, schat;

        if((scomm=connectionToServer(mainServeur))==0)
        {
          perror("connexion");
          continue;
        } 
        int portChat=serveurChat->port;

        if(send_all(scomm,&portChat,sizeof(portChat))!=0){
          close(scomm);
          perror("sending port");
          continue;

        }


        if((schat=connectionToServer(serveurChat))==0)
        {
          close(scomm);
          perror("connexion");
          continue;
        } 

        printf("Connection accepted!\n");

        // Allocate memory for the client socket descriptor
        int *client_fd = malloc(sizeof(int)*2);

        if (client_fd == NULL) {
            perror("malloc");
            close(scomm);
            close(schat);
            continue;
        }

        client_fd[0] = scomm;
        client_fd[1] = schat;

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
            close(schat);
            continue;
        }

        // Detach thread (no need to join it later)
        pthread_detach(threadComm);
    }
    destructTableClient(tableClient);

    close(mainServeur->socketfd);
    close(serveurChat->socketfd);
    free(mainServeur);
    free(serveurChat);
    return EXIT_SUCCESS;
}