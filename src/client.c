#include "client.h"
#include "reseau.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "joueur.h"
#include "plateau.h"

#define LONGUEUR_MESSAGE 100


void listenChat(int chatSockfd)
{
  char message[LONGUEUR_MESSAGE];
  while(1)
  {
    if ((recv_all(chatSockfd,message,sizeof(message)))!=0)
    {
      break;
    }
    printf("%s\n",message);
    
  }
}

void afficherPlateau(Plateau* plateau, Joueur* joueur1, Joueur* joueur2)
{
    int i;

    printf("\t\t Joueur 1\n");
    printf("        ");

    for (i = 6; i > 0; i--)
        printf("%d    ", i);

    printf("\n");

    for (i = 0; i < 41; i++)
        printf("_");

    printf("\n| S%d |", joueur1->id);

    for (i = 6; i >= 1; i--)
        printf(" %2d |", recupererChamp(plateau, joueur1, i));

    printf("    |\n");

    printf("| %2d |", joueur1->score);

    for (i = 0; i < 29; i++)
        printf("-");

    printf("| %2d |\n", joueur2->score);
    printf("|    |");

    for (i = 1; i <= 6; i++)
        printf(" %2d |", recupererChamp(plateau, joueur2, i));

    printf(" S%d |\n", joueur2->id);

    for (i = 0; i < 41; i++)
        printf("-");

    printf("\n        ");

    for (i = 1; i <= 6; i++)
        printf("%d    ", i);

    printf("\n\t\t Joueur 2\n");
}


/*
 * Read an integer from stdin.
 * Returns 0 on success, -1 on invalid input or EOF.
 */
int lireEntier(int *value)
{
    char buffer[64];

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return -1;

    char *end;
    long result = strtol(buffer, &end, 10);

    while (*end == ' ' || *end == '\t')
        end++;

    if (end == buffer || (*end != '\n' && *end != '\0'))
        return -1;

    *value = (int)result;

    return 0;
}


/*
 * Username authentication
 */
int authentification(int sockfd)
{
    while (1)
    {
        char username[USERNAME_SIZE] = {0};

        printf("Enter username (maximum %d characters): ",
               USERNAME_SIZE - 1);

        if (fgets(username, sizeof(username), stdin) == NULL)
            return -1;

        username[strcspn(username, "\n")] = '\0';

        if (strlen(username) == 0)
        {
            printf("Username cannot be empty.\n");
            continue;
        }

        if (send_all(sockfd, username, sizeof(username)) != 0)
        {
            perror("send username");
            return -1;
        }

        char response;

        if (recv_all(sockfd, &response, sizeof(response)) != 0)
        {
            printf("Server disconnected.\n");
            return -1;
        }

        if (response == 1)
        {
            printf("Username accepted! Welcome %s.\n", username);
            return 0;
        }

        printf("Username already in use. Try another one.\n");
    }
}


/*
 * Display available players
 */
int errorAfficherJoueurs(char ** tableN_noms)
{
  for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
    free(tableN_noms[i]);

  return -1;
}

int afficherJoueurs(int sockfd)
{
    char command = 'l';

    if (send_all(sockfd, &command, sizeof(command)) != 0)
        return -1;

    char response[MAX_PLAYER_ONLINE * USERNAME_SIZE + 1] = {0};

    if (recv_all(sockfd, response, sizeof(response)) != 0)
        return -1;

    char *tableN_noms[MAX_PLAYER_ONLINE] = {0};
    int tableIDs[MAX_PLAYER_ONLINE];

    int cursor = 0;
    int size_list = 0;

    /*
     * Parse the username array.
     * Names are separated by '\0'.
     * EOF marks the end of the list.
     */
    for (int i = 0; i < MAX_PLAYER_ONLINE; i++)
    {
        if ((unsigned char)response[cursor] == (unsigned char)EOF)
            break;

        size_t remaining = sizeof(response) - cursor;
        size_t len = strlen(response + cursor);

        if (len == remaining || len >= USERNAME_SIZE)
            return errorAfficherJoueurs(tableN_noms);

        tableN_noms[size_list] = malloc(len + 1);

        if (tableN_noms[size_list] == NULL)
            return errorAfficherJoueurs(tableN_noms);

        memcpy(tableN_noms[size_list], response + cursor, len + 1);

        cursor += len + 1;
        size_list++;
    }

    command = 'L';

    if (send_all(sockfd, &command, sizeof(command)) != 0)
        return errorAfficherJoueurs(tableN_noms);

    if (recv_all(sockfd, tableIDs, sizeof(tableIDs)) != 0)
        return errorAfficherJoueurs(tableN_noms);

    printf("\n%d Players connected\n", size_list);
    printf("Players_name : id\n");

    for (int i = 0; i < size_list; i++)
        printf("%s : %d\n", tableN_noms[i], tableIDs[i]);

    for (int i = 0; i < size_list; i++)
        free(tableN_noms[i]);

    return 0;
}


/*
 * Game logic
 *
 * player_id = 0 : Local player is player 1
 * player_id = 1 : Local player is player 2
 */
int jouerPartie(int sockfd, int player_id)
{
    Plateau *plateau = constructPlateau();

    if (plateau == NULL)
        return -1;

    Joueur **joueurs = malloc(sizeof(Joueur *) * 2);

    if (joueurs == NULL)
    {
        // Adapt this if your plateau API has a destructor.
        return -1;
    }

    joueurs[0] = constructJoueur(1);
    joueurs[1] = constructJoueur(2);

    if (joueurs[0] == NULL || joueurs[1] == NULL)
    {
        free(joueurs);
        return -1;
    }

    int fini = 0;
    int joueurQuiJoue = 0;

    while (!fini)
    {
        printf("\n");

        afficherPlateau(plateau, joueurs[0], joueurs[1]);

        printf("\n");

        int coup;
        int isValide = 0;

        while (isValide != 1)
        {
            printf("Coup du joueur %d: ",
                   joueurs[joueurQuiJoue]->id);

            if (joueurQuiJoue == player_id)
            {
                if (lireEntier(&coup) != 0)
                {
                    printf("Invalid input.\n");
                    continue;
                }

                isValide = coupValide(
                    plateau,
                    joueurs[joueurQuiJoue],
                    coup
                );

                if (isValide == 1)
                {
                    if (send_all(sockfd, &coup, sizeof(coup)) != 0)
                    {
                        free(joueurs[0]);
                        free(joueurs[1]);
                        free(joueurs);
                        return -1;
                    }
                }
            }
            else
            {
                if (recv_all(sockfd, &coup, sizeof(coup)) != 0)
                {
                    printf("Connection lost during game.\n");

                    free(joueurs[0]);
                    free(joueurs[1]);
                    free(joueurs);

                    return -1;
                }

                isValide = 1;
            }

            if (isValide == 0)
                printf("Coup en dehors des limites.\n");

            else if (isValide == -1)
                printf("Champ vide.\n");

            else if (isValide == -2)
                printf("Affame l'adversaire.\n");
        }

        jouerUnCoup(plateau, joueurs[joueurQuiJoue], coup);

        if (joueurs[joueurQuiJoue]->score >= 25)
            fini = 1;

        joueurQuiJoue = (joueurQuiJoue + 1) % 2;

        if (!joueurPeutJouer(plateau, joueurs[joueurQuiJoue]))
        {
            afficherPlateau(plateau, joueurs[0], joueurs[1]);

            finPartieFamine(plateau, joueurs[joueurQuiJoue]);

            fini = 1;
        }
    }

    free(joueurs[0]);
    free(joueurs[1]);
    free(joueurs);

    return 0;
}


/*
 * Accept or refuse an incoming challenge
 */
int recevoirChallenge(int sockfd)
{
    char response;

    if (recv_all(sockfd, &response, sizeof(response)) != 0)
        return -1;

    if (response == '-')
    {
        printf("No challenge.\n");
        return 0;
    }

    if (response != 'c')
    {
        printf("Unexpected server response.\n");
        return -1;
    }

    char answer = 0;

    printf("You have been challenged! Accept? [y/n]\n");

    while (1)
    {
        char input[16];

        if (fgets(input, sizeof(input), stdin) == NULL)
            return -1;

        if (input[0] == 'y' || input[0] == 'n')
        {
            answer = input[0];
            break;
        }

        printf("[y/n] ?\n");
    }

    if (send_all(sockfd, &answer, sizeof(answer)) != 0)
        return -1;

    if (answer == 'y')
    {
        printf("Challenge accepted!\n");

        // Player 2
        return jouerPartie(sockfd, 1);
    }

    printf("Challenge refused.\n");

    return 0;
}


/*
 * Request a challenge
 */
int lancerChallenge(int sockfd)
{
    char command = 'x';

    if (send_all(sockfd, &command, sizeof(command)) != 0)
        return -1;

    printf("Enter the player ID to challenge: ");

    int player_id;



    if (lireEntier(&player_id) != 0)
    {
      printf("Invalid player ID.\n");
      if (send_all(sockfd, &player_id, sizeof(player_id)) != 0)
        return -1;
      return 0;
    }
    if (send_all(sockfd, &player_id, sizeof(player_id)) != 0)
      return -1;

    char response;

    printf("Waiting for response...\n");

    if (recv_all(sockfd, &response, sizeof(response)) != 0)
        return -1;

    if (response == 'y')
    {
        printf("Challenge accepted!\n");

        // Player 1
        return jouerPartie(sockfd, 0);
    }

    if (response == 'n')
        printf("Player refused the challenge.\n");

    return 0;
}
int connectionToServer(int port,char* address)
{
  int sockfd;
  struct sockaddr_in serv_addr = {0};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid server IP address.\n");
        return 0;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 0;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 0;
    }
    return sockfd;
}


int main(int argc, char **argv)
{
    int mainSockfd;
    int chatSockfd;
    char c;
    int portChat;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: socket_client server port\n");
        return EXIT_FAILURE;
    }

    printf("Client starting...\n");

    if ((mainSockfd=connectionToServer(atoi(argv[2]),argv[1]))==0)
    {
      return EXIT_FAILURE;
    }

    if(recv_all(mainSockfd,&portChat,sizeof(portChat))!=0)
    {
      close(mainSockfd);
      return EXIT_FAILURE;
    }
    

    if ((chatSockfd=connectionToServer(portChat,argv[1]))==0)
    {
      close(mainSockfd);
      return EXIT_FAILURE;
    }

    printf("Connected to server!\n");

    if (authentification(mainSockfd) != 0)
    {
        close(chatSockfd);
        close(mainSockfd);
        return EXIT_FAILURE;
    }

    printf("\nYou can now communicate with the server.\n");

    while (1)
    {
        printf("\n");
        printf("Press 'q' to disconnect.\n");
        printf("Press 'l' to list players.\n");
        printf("Press 'c' to check challenges.\n");
        printf("Press 'x' to challenge a player.\n");

        char input[16];

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        c = input[0];

        if (c == 'q')
            break;

        if (c == 'l')
        {
            if (afficherJoueurs(mainSockfd) != 0)
            {
                printf("Communication error.\n");
                break;
            }
        }
        else if (c == 'x')
        {
            if (lancerChallenge(mainSockfd) != 0)
            {
                printf("Communication error.\n");
                break;
            }
        }
        else if (c == 'c')
        {
            if (recevoirChallenge(mainSockfd) != 0)
            {
                printf("Communication error.\n");
                break;
            }
        }
    }
    close(chatSockfd);

    close(mainSockfd);

    printf("Disconnected from server.\n");

    return EXIT_SUCCESS;
}
