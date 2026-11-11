//
// Created by louan on 10/11/2026.
//
#include "reseau.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int send_all(int socket, const void *message, size_t taille)
{
  const char *ptr = message;
  size_t total = 0;

  while (total < taille)
  {
    ssize_t sent = send(socket, ptr + total, taille - total, MSG_NOSIGNAL);

    if (sent < 0)
    {
      if (errno == EINTR)
        continue;

      return -1;
    }

    if (sent == 0)
      return -1;

    total += sent;
  }

  return 0;
}


int recv_all(int socket, void *message, size_t taille)
{
  char *ptr = message;
  size_t total = 0;

  while (total < taille)
  {
    ssize_t received = recv(socket, ptr + total, taille - total, 0);

    if (received < 0)
    {
      if (errno == EINTR)
        continue;

      return -1;
    }

    // Le client a fermé la connexion
    if (received == 0)
      return 1;

    total += received;
  }

  return 0;
}
