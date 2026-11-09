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

#include "joueur.h"
#include "plateau.h"

void afficherPlateau(Plateau* plateau, Joueur* joueur1, Joueur* joueur2)
{
    int i;
    printf("\t\t Joueur 1\n");
    printf("        ");
    for(i=6; i>0;i--)
    {
        printf("%d    ",i);

    }

     printf("\n");
    for(i=0;i<41;i++)//je ne sais pas quoi mettre a la place don je laisse 41
        printf("_");

    printf("\n| S%d |",joueur1->id);
    for(i=6;i>=1;i--)
    {
        printf(" %2d |",recupererChamp(plateau,joueur1,i));
    }
    printf("    |\n");

    printf("| %2d |", joueur1->score);

    for (i=0; i<29;i++)
    {
        printf("-");
    }
    printf("| %2d |\n",joueur2->score);
    printf("|    |");
    for(i=1;i<=6;i++)
    {
        printf(" %2d |",recupererChamp(plateau,joueur2,i));
    }
    printf(" S%d |\n",joueur2->id);
    for(i=0;i<41;i++)
    {
        printf("-");
    }
    printf("\n        ");
    for(i=1;i<=6;i++)
    {
        printf("%d    ",i);
    }
    printf("\n\t\t Joueur 2\n");
}

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



    while (1) {
        printf("Press 'q' to disconnect.\n");
        printf("Press 'l' to list players.\n");
        printf("Press 'c' to check challenges.\n");
        c = getchar();
      if (c == 'c')
      {
        c = -1;
        read(sockfd, &c, sizeof(c));
        if (c==-1)
        {
          //"No challenge"
        }
        else
        {
          int responded = 0;
          printf("You have been Challenged do you accept [y/n] ?\n");
          while (!responded)
          {
            c = getchar();
            if (c=='y' || c=='n')
            {
              responded = 1;
              send(sockfd, &c, sizeof(c), MSG_WAITALL);
              break;
            }
            printf("[y/n] ?\n");
          }
          // start the board as player 2
          Plateau * plateau = constructPlateau();
          Joueur **joueurs = malloc(sizeof(Joueur *) * 2);
          joueurs[1]=constructJoueur(2);
          joueurs[0]=constructJoueur(1);

          // linsten input from serveur
          int coup;
          int isValide;
          int fini=0;
          int joueurQuiJoue=0;

          while(!fini)
          {
            printf("\n");
            afficherPlateau(plateau,joueurs[0],joueurs[1]);
            printf("\n");

            isValide=0;
            while(isValide!=1)
            {
              printf("coup du joueur %d: \t",joueurs[joueurQuiJoue]->id);

              if (joueurQuiJoue == 1)
              {
                scanf("%d",&coup);
                isValide=coupValide(plateau,joueurs[joueurQuiJoue],coup);
                send(sockfd, &coup, sizeof(coup), MSG_WAITALL);
              }
              if (joueurQuiJoue == 0)
              {
                ssize_t received = recv(sockfd, &coup, sizeof(coup), MSG_WAITALL);
                isValide=1;
              }

              if(isValide==0)
              {
                printf("coup en dehors des limites\n");
              }
              else if(isValide==-1)
              {
                printf("champs vide\n");
              }
              else if(isValide==-2)
              {
                printf("affame l'adversaire\n");
              }

            }
            jouerUnCoup(plateau, joueurs[joueurQuiJoue], coup);
            if(joueurs[joueurQuiJoue]->score>=25){
              fini=1;
            }
            joueurQuiJoue=(joueurQuiJoue+1)%2;
            if(!joueurPeutJouer(plateau,joueurs[joueurQuiJoue]))
            {
              afficherPlateau(plateau,joueurs[0],joueurs[1]);
              finPartieFamine(plateau, joueurs[joueurQuiJoue]);

              fini=1;
            }
          }
        }

      }
        if (c == 'l') {
          if (send(sockfd, &c, sizeof(c), 0) <= 0)
          {
            perror("send");
            break;
          }

          // Receive server response
          char response[MAX_PLAYER_ONLINE*USERNAME_SIZE+1];

          ssize_t received = recv(sockfd, &response, sizeof(response), MSG_WAITALL);

          char** tableN_noms = malloc(MAX_PLAYER_ONLINE*USERNAME_SIZE*sizeof(char*));
          int cursor = 0;
          int size_list = 0;
          for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
          {
            if (response[cursor] != EOF)
            {
              tableN_noms[i] = malloc(USERNAME_SIZE*sizeof(char));
              strcpy(tableN_noms[i], (response + cursor));
              cursor = cursor + strlen(response) + 1 ;
            }
            else
            {
              size_list = i;
              break;
            }
          }
          c = 'L';

          if (send(sockfd, &c, sizeof(c), 0) <= 0)
          {
            perror("send");
            break;
          }
          int tableIDs[MAX_PLAYER_ONLINE];

          received = recv(sockfd, &tableIDs, sizeof(tableIDs), MSG_WAITALL);

          // Prints player list : ids
          printf("%d Players connected\n", size_list);
          printf("Players_name : id\n");
          for (int i = 0; i < size_list; i++)
          {
            printf("%s : %d\n", tableN_noms[i], tableIDs[i]);
          }
          printf("Enter the player id to challenge :\n");
          int id_player;
          scanf("%d", &id_player);

          // send challenge id player
          if (send(sockfd, &id_player, sizeof(id_player), 0) <= 0)
          {
            perror("send");
            break;
          }

          // waiting for resonse challenge

          char rep;
          printf("Waiting for response\n");
          received = recv(sockfd, &rep, sizeof(rep), MSG_WAITALL);

          if (received != sizeof(rep)) {}

          if (rep == 'y')
          {
            // start the board as player 1
            Plateau * plateau = constructPlateau();
            Joueur **joueurs = malloc(sizeof(Joueur *) * 2);
            joueurs[1]=constructJoueur(2);
            joueurs[0]=constructJoueur(1);

            int coup;
            int isValide;
            int fini=0;
            int joueurQuiJoue=0;

            while(!fini)
            {
              printf("\n");
              afficherPlateau(plateau,joueurs[0],joueurs[1]);
              printf("\n");

              isValide=0;
              while(isValide!=1)
              {
                printf("coup du joueur %d: \t",joueurs[joueurQuiJoue]->id);

                if (joueurQuiJoue == 0)
                {
                  scanf("%d",&coup);
                  isValide=coupValide(plateau,joueurs[joueurQuiJoue],coup);
                  send(sockfd, &coup, sizeof(coup), MSG_WAITALL);
                }
                if (joueurQuiJoue == 1)
                {
                  ssize_t received = recv(sockfd, &coup, sizeof(coup), MSG_WAITALL);
                  isValide=1;
                }

                if(isValide==0)
                {
                  printf("coup en dehors des limites\n");
                }
                else if(isValide==-1)
                {
                  printf("champs vide\n");
                }
                else if(isValide==-2)
                {
                  printf("affame l'adversaire\n");
                }

              }
              jouerUnCoup(plateau, joueurs[joueurQuiJoue], coup);
              if(joueurs[joueurQuiJoue]->score>=25){
                fini=1;
              }
              joueurQuiJoue=(joueurQuiJoue+1)%2;
              if(!joueurPeutJouer(plateau,joueurs[joueurQuiJoue]))
              {
                afficherPlateau(plateau,joueurs[0],joueurs[1]);
                finPartieFamine(plateau, joueurs[joueurQuiJoue]);

                fini=1;
              }
            }
          }
          if (rep == 'n')
          {
            printf("Player refused the challenge.\n");
          }
        }

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
