//
// Created by louan on 21/10/2026.
//
/* Client pour les sockets
 *    socket_client ip_server port
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "constants.h"

int main(int argc, char** argv )
{
  int    sockfd,newsockfd,clilen,chilpid,ok,nleft,nbwriten;
  char c;
  struct sockaddr_in cli_addr,serv_addr;

  if (argc!=3) {printf ("usage  socket_client server port\n");exit(0);}


  /*
   *  partie client
   */
  printf ("client starting\n");

  /* initialise la structure de donnee */
  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family       = AF_INET;
  serv_addr.sin_addr.s_addr  = inet_addr(argv[1]);
  serv_addr.sin_port         = htons(atoi(argv[2]));

  /* ouvre le socket */
  if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
  {printf("socket error\n");exit(0);}

  /* effectue la connection */
  if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
  {printf("socket error\n");exit(0);}


  /* repete dans le socket tout ce qu'il entend */
  int connected = 1;

  char username[USERNAME_SIZE];

  int usernameValid = 0;

  while (!usernameValid)
  {
    printf("ENTER username least than 60 character: ");
    scanf("%s", &username);
    printf("\nUsername entered :%s\n", username);
    printf("Sending to the server to verify validity of username");
    send (sockfd, username, strlen(username)*sizeof(char),0);

    unsigned int len = next_pow2(USERNAME_SIZE*MAX_PLAYER_ONLINE + 1);

    char buffer[len];
    buffer[len-1] = EOF;
    recv(sockfd,buffer,len*sizeof(char),0);

    if (buffer[0])
    {
      usernameValid = 1;
      printf("Users Online :\n");
      int i = 1 ;
      while (buffer[i] != EOF)
      {
        if (buffer[i] == '\0')
        {
          printf("\n");
        }
        printf("%c", buffer[i]);
        i++;
      }
      printf("\n");
      if (i == 1)
      {
        printf("No user online");
      }
    }
    else
    {
      printf("Invalid Username already in use");
    }
  }

  while (connected) {


    c=getchar();
    write (sockfd,&c,1);
    if (c == 'q')
    {
      connected = 0;
    }
  }
  close(sockfd);


  /*  attention il s'agit d'une boucle infinie
   *  le socket n'est jamais ferme !
   */

  return 1;

}