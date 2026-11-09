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
    nouveauClient->sockfd=sockfd;
    nouveauClient->id=premierePlace;
    nouveauClient->nomUtilisateur=malloc(sizeof(char)*(USERNAME_SIZE));
    strcpy(nouveauClient->nomUtilisateur,nomUtilisateur);
    tableClient->table[premierePlace]=nouveauClient;
    tableClient->current_size++;
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
            return 1;
        }
        else 
        {
            return -1;
        }
    }
    else
    {
        return -2;
    }
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
    free(tableClient->table);
    free(tableClient);
    pthread_mutex_destroy(&tableClient->mutex);
}


