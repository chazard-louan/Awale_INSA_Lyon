//
// Created by louan on 21/10/2026.
//

/* Serveur sockets TCP
 * affichage de ce qui arrive sur la socket
 *    socket_server port (port > 1024 sauf root)
 */

#include "serveur.h"
#include "liste_client.h"
#include "reseau.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <poll.h>





TableClient* tableClient;


void * cleanup(int fd, int my_id)
{
  printf("Client disconnected (ID: %d)\n", my_id);
  disconnectClient(tableClient, my_id);
  close(fd);
  return NULL;
}

int register_client(int fd, int *my_id, char *username)
{
  char response;

  while (1)
  {
    printf("Doing mem set for username checking \n");
    memset(username, 0, USERNAME_SIZE);

    printf("Listenning to username  from client k \n");
    if (recv_all(fd, username, USERNAME_SIZE) != 0)
      return -1;

    // Check username termination
    printf("cheking integrity of string  from client k \n");
    if (username[USERNAME_SIZE - 1] != '\0')
    {
      response = 0;

      if (send_all(fd, &response, sizeof(response)) != 0)
        return -1;

      continue;
    }

    printf("locking table client checking username %s\n", username);
    pthread_mutex_lock(&tableClient->mutex);

    while (cannotTryConnect(tableClient))
    {
      pthread_cond_wait(&tableClient->cond, &tableClient->mutex);
    }

    int id = getExistingUserId(tableClient, username);//returns -1 if not found
    printf("Getting id of the username %s : %d\n", username, id);


    if (id == -1 && !nomDejaPris(tableClient, username))
    {
      printf("id disconected not found and username not already taken : %s\n", username);

      *my_id = ajoutNouveauClient(tableClient, fd, username);

      pthread_mutex_unlock(&tableClient->mutex);

      printf("Respond positively to client : %s\n", username);
      response = 1;

      if (send_all(fd, &response, sizeof(response)) != 0)
        return -1;

      printf("Client added and connected: %s (ID: %d)\n", username, *my_id);

      return 0;
    }

    if (tableClient->table[id] != NULL && !tableClient->table[id]->connected)
    {
      printf("id disconnected found and username already taken but disconnected, so connecting: %s\n", username);
      connectClient(tableClient, id, fd);
      pthread_mutex_unlock(&tableClient->mutex);

      printf("Respond positively to client : %s\n", username);
      response = 1;

      if (send_all(fd, &response, sizeof(response)) != 0)
        return -1;

      printf("Client connected: %s (ID: %d)\n", username, *my_id);

      return 0;
    }
    printf("id disconected not found and username already taken but disconnected, so connecting: %s\n", username);
    pthread_mutex_unlock(&tableClient->mutex);
    response = 0;// respond invalid username

    if (send_all(fd, &response, sizeof(response)) != 0)
      return -1;
    return 0;
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

int isSocketEmpty(int sock_fd)
{
  struct pollfd pfd;

  pfd.fd = sock_fd;
  pfd.events = POLLIN;

  int ret = poll(&pfd, 1, 0);

  if (ret > 0) {
    if (pfd.revents & POLLIN)
    {
      return 0;
    }
    if (pfd.revents & POLLHUP)
    {
      // Peer disconnected.
      return -1;
    }
    if (pfd.revents & POLLERR)
    {
      // Socket error.
      return -1;
    }
  }
  else if (ret == 0)
  {
    return 1;
  }
  else
  {
    return -1;
    // poll() error; check errno.
  }
  return -1;
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

  int player_fd = tableClient->table[player_id]->sockfd;

  pthread_mutex_unlock(&tableClient->mutex);

  return player_fd;
}

void handle_game(int fd1, int fd2)
{
  int coup;

  while (1)
  {
    // Receive move from player 1
    printf("\nWaiting for player 1 to move...\n");
    fflush(stdout);
    if (recv_all(fd1, &coup, sizeof(coup)) != 0)
      break;

    printf("Move %d received from player 1\n", coup);

    if (coup == 0)
    {
      printf("%d move END of the game\n", coup);
      printf("Forwarding %d move to player 2\n", coup);
      if (send_all(fd2, &coup, sizeof(coup)) != 0)
      break;
    }

    // Forward move to player 2
    printf("Forwarding %d move to player 2\n", coup);
    if (send_all(fd2, &coup, sizeof(coup)) != 0)
      break;

    // Receive next move from player 2
    printf("Waiting for player 2 to move...\n");
    fflush(stdout);
    if (recv_all(fd2, &coup, sizeof(coup)) != 0)
      break;
    printf("\nMove %d received from player 2\n", coup);

    if (coup == 0)
    {
      printf("%d move END of the game\n", coup);
      printf("\nForwarding %d move to player 1\n", coup);
      if (send_all(fd1, &coup, sizeof(coup)) != 0)
      break;
    }

    // Forward move to player 1
    printf("\nForwarding %d move to player 1\n", coup);
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
    printf("Challenge refused, send refusal to player %d\n", my_id);
    return send_all(fd, &refusal, sizeof(refusal));
  }

  // Send challenge
  char challenge = 'c';
  printf("Ask Challenge to player %d\n", player_id);

  if (send_all(player2_sockfd, &challenge, sizeof(challenge)) != 0)
    return -1;

  while (isSocketEmpty(player2_sockfd))
  {
    //wait
  }
  sleep(1); // waiting for other socket to be killed

  // Receive response
  char response;

  if (recv_all(player2_sockfd, &response, sizeof(response)) != 0)
    return -1;

  printf("Challenge responded by player %d by '%c'\n", player_id, response);

  if (response == 'n')
  {
    printf("Challenge refused, send refusal to player %d\n", my_id);
    int ret = send_all(fd, &response, sizeof(response));
    return ret;
  }

  if (response != 'y')
    return -1;

  // Notify requesting player
  printf("Challenge accepted, send approval to player %d\n", my_id);
  if (send_all(fd, &response, sizeof(response)) != 0)
    return -1;

  printf("setting players to unavailable\n");

  // Both players are now unavailable
  set_players_available(my_id, player_id, 0);

  // Start game
  printf("Starting game \n");
  handle_game(fd, player2_sockfd);

  // Game finished
  set_players_available(my_id, player_id, 1);


  // Allocate memory for the client socket descriptor
  int *client_fd = malloc(sizeof(int));

  if (client_fd == NULL) {
    perror("malloc");
    free(client_fd);// not useful since malloc failed but should pass...
    close(player2_sockfd);
    return 0;
  }

  client_fd[0] = player2_sockfd;
  client_fd[1] = 1;//to skip authentification
  client_fd[2] = player_id;


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
    close(player2_sockfd);
    return 0;
  }
  pthread_detach(threadComm); // not need to be called anymore
  return 0;
}


void *comm_client(void *args)
{
  int fd = ((int *)args)[0];
  int skip_authentification = ((int *)args)[1];
  int my_id = 0;
  if (skip_authentification)
  { my_id = ((int *)args)[2]; }
  else
  {
  char username[USERNAME_SIZE] = {0};

  // Step 1: Register client
  if (register_client(fd, &my_id, username) != 0)
    return cleanup(fd,my_id);
  }
  free(args);

  char command;
  while (1)
  {
    if ( recv_all(fd, &command, sizeof(command)) != 0)
      return cleanup(fd,my_id);


    if (command == 'l')
    {
      if (send_available_players(fd) != 0)
        return cleanup(fd,my_id);
    }
    else if (command == 'L')
    {
      if (send_available_player_ids(fd) != 0)
        return cleanup(fd,my_id);
    }
    else if (command == 'k')
    {// kills the thread to let someone else take care of the comm
      return NULL;
    }
    else if (command == 'x')
    {

      int player_id;

      if (recv_all(fd, &player_id, sizeof(player_id)) != 0)
        return cleanup(fd,my_id);

      printf("Player %d wants to challenge player %d\n",
             my_id, player_id);

      // Handle challenge
      if (handle_challenge(fd, my_id, player_id) != 0)
        return cleanup(fd,my_id);
    }
    else
    {
      printf("Not normal message sent %d\n as char", command);
      if (send_all(fd, &command, sizeof(command)) !=0)
      {
        return cleanup(fd,my_id);
      }
    }
  }
  return cleanup(fd,my_id);
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
    struct timeval timeout;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    // Allow address reuse
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &timeout, sizeof(timeout));

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

        client_fd[0] = scomm;
        client_fd[1] = 0;//to not skip authentification


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
