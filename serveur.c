//
// Created by louan on 21/10/2026.
//

/* Serveur sockets TCP
 * affichage de ce qui arrive sur la socket
 *    socket_server port (port > 1024 sauf root)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char** argv )
{ char datas[] = "hello\n";
  int  scomm,sockfd,newsockfd,clilen,chilpid,ok,nleft,nbwriten,pid;
  char c;
  struct sockaddr_in cli_addr,serv_addr;


  if (argc!=2) {printf ("usage: socket_server port\n");exit(0);}

  printf ("server starting...\n");

  /* ouverture du socket */
  sockfd = socket (AF_INET,SOCK_STREAM,0);
  if (sockfd<0) {printf ("impossible d'ouvrir le socket\n");exit(0);}

  /* initialisation des parametres */
  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family       = AF_INET;
  serv_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
  serv_addr.sin_port         = htons(atoi(argv[1]));

  /* effectue le bind */
  if (bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
     {printf ("impossible de faire le bind\n");exit(0);}

  /* petit initialisation */
  listen(sockfd,1);

  /* attend la connection d'un client */
  clilen = sizeof (cli_addr);



  while (1)
  {
    scomm = accept (sockfd,(struct sockaddr*) &cli_addr, &clilen);
    if (scomm<0)
    {
      printf ("accept error\n");
    }
    else
    {
      printf ("connection accepted\n");
      pid = fork();
      if (pid == 0) /* c’est le fils */
      {
        close(sockfd); /* socket inutile pour le fils */
        pid = getpid();

        /* traiter la communication */
        int is_end = 1;
        while (is_end)
        { while (is_end && read(scomm,&c,1)!=1);
          printf("%d : %c\n",pid, c);
          if (c == 'q')
          {
            printf("\nend comm for process %d\n", pid);
            is_end = 0;
          }
        }

        close(scomm);
        exit(0); /* on force la terminaison du fils */
      }
      else /* c’est le pere */
      {
        close(scomm); /* socket inutile pour le pere */
        printf("\nbegin comm for process %d\n", pid);/*pid is the pid of the child*/
      }
    }

  }
  close(sockfd);

   return 1;
}