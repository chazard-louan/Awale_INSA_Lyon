//
// Created by louan on 10/11/2026.
//

#ifndef AWALE_RESEAU_H
#define AWALE_RESEAU_H


#include <stddef.h>

int send_all(int socket, const void *message, size_t taille);
int recv_all(int socket, void *message, size_t taille);

#endif //AWALE_RESEAU_H
