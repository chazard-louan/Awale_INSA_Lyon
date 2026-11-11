//
// Created by louan on 21/10/2026.
//

#ifndef AWALE_SERVEUR_H
#define AWALE_SERVEUR_H
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "constants.h"

typedef struct Serveur{
    int socketfd;
    int port;

}Serveur;

void *comm_client(void *args);

int main(int argc, char** argv );
#endif //AWALE_SERVEUR_H
