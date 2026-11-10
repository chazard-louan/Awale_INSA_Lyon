/*
 * fonction pour la gestion de la liste des client
 * Usage: 
 */

#include "liste_client.h"
#include "constants.h"

#include <stdlib.h>
#include <string.h>


TableClient* initialiserTableClient(int taille)
{
    TableClient* tableClient=malloc(sizeof(TableClient));
    tableClient->table=malloc(sizeof(Client*)*taille);
    tableClient->taille=taille;
    tableClient->current_size=0;
    tableClient->connected_size=0;
    for(int i=0;i<taille;i++)
    {
        tableClient->table[i]=NULL;
    }
    if (pthread_mutex_init(&(tableClient->mutex), NULL) != 0) {
      // Gestion d'erreur
    }
    pthread_cond_init(&(tableClient->cond), NULL);
    return tableClient;

}

int ajoutNouveauClient(TableClient* tableClient, int sockfd,char* nomUtilisateur)
{
    Client* nouveauClient=malloc(sizeof(Client));
    int premierePlace=premierePlaceLibre(tableClient);
    nouveauClient->estDisponible=1;
    nouveauClient->connected=1;
    nouveauClient->sockfd=sockfd;
    nouveauClient->id=premierePlace;
    nouveauClient->nomUtilisateur=malloc(sizeof(char)*(USERNAME_SIZE));
    strcpy(nouveauClient->nomUtilisateur,nomUtilisateur);
    tableClient->table[premierePlace]=nouveauClient;
    tableClient->current_size++;
    tableClient->connected_size++;
    return premierePlace;
}

int premierePlaceLibre(TableClient *tableClient)
{
    for(int i=0;i<tableClient->taille;i++){
        if(tableClient->table[i]==NULL)
        {
            return i;
        }
    }
    return -1;
}

int tablePleine(TableClient *tableClient)
{
    return tableClient->current_size == tableClient->taille;
}

int cannotTryConnect(TableClient *tableClient)
{
  return tableClient->connected_size == tableClient->taille;
}

int userExist(TableClient* tableClient, char*nom)
{
  for (int i=0;i<tableClient->taille;i++)
  {
    if(tableClient->table[i] !=NULL && strcmp(nom, tableClient->table[i]->nomUtilisateur)==0)
    {
      return 1;
    }
  }
  return 0;
}

int getExistingUserId(TableClient* tableClient, char*nom)
{
  for (int i=0;i<tableClient->taille;i++)
  {
    if(tableClient->table[i] !=NULL && !tableClient->table[i]->connected && strcmp(nom, tableClient->table[i]->nomUtilisateur)==0)
    {
      return i;
    }
  }
  return -1;
}

int nomDejaPris(TableClient* tableClient, char*nom)
{
    for (int i=0;i<tableClient->taille;i++)
    {
        if(tableClient->table[i] !=NULL && strcmp(nom, tableClient->table[i]->nomUtilisateur)==0)
        {
            return 1;
        }
    }
    return 0;
}

Client* rechercherClientParID(TableClient* tableClient,int id)
{
    if(id<tableClient->taille)
    {
        return tableClient->table[id];
    }
    else
    {
        return NULL;
    } 
}

int connectClient(TableClient* tableClient, int id, int sockfd)
{
  if(id<tableClient->taille)
  {
    if(tableClient->table[id]!=NULL)
    {
      tableClient->connected_size++;
      tableClient->table[id]->connected = 1;
      tableClient->table[id]->estDisponible = 1;
      tableClient->table[id]->sockfd = sockfd;
      return 1;
    }
    return -1;
  }
  return -2;
}


int disconnectClient(TableClient* tableClient,int id)
{
  if(id<tableClient->taille)
  {
    if(tableClient->table[id]!=NULL)
    {
      tableClient->connected_size--;
      tableClient->table[id]->connected = 0;
      tableClient->table[id]->estDisponible = 0;
      tableClient->table[id]->sockfd = -1;
      return 1;
    }
    return -1;
  }
  return -2;
}

int retirerClient(TableClient* tableClient,int id)
{
    if(id<tableClient->taille)
    {
        if(tableClient->table[id]!=NULL)
        {
            free(tableClient->table[id]->nomUtilisateur);
            free(tableClient->table[id]);
            tableClient->table[id]=NULL;
            tableClient->current_size--;
            tableClient->connected_size--;
            return 1;
        }
      return -1;
    }
  return -2;
}

void destructTableClient(TableClient *tableClient)
{
    for(int i=0;i<tableClient->taille;i++)
    {
        if(tableClient->table[i]!=NULL)
        {
            free(tableClient->table[i]);
        }
    }
    pthread_mutex_destroy(&tableClient->mutex);
    free(tableClient->table);
    free(tableClient);

}


