/*
 * fonction pour la gestion de la liste des client
 * Usage: 
 */

#include "liste_client.h"

#include <stdlib.h>
#include <string.h>


TableClient* initialiserTableClient(int taille)
{
    TableClient* tableClient=malloc(sizeof(TableClient));
    tableClient->table=malloc(sizeof(Client*)*taille);
    tableClient->taille=taille;
    for(int i=0;i<taille;i++)
    {
        tableClient->table[i]=NULL;
    }
    return tableClient;

}

int ajoutNouveauClient(TableClient* tableClient, int sockfd,char* nomUtilisateur)
{
    Client* nouveauClient=malloc(sizeof(Client));
    int premierePlace=premierePlaceLibre(tableClient);
    nouveauClient->estDisponible=1;
    nouveauClient->sockfd=sockfd;
    nouveauClient->id=premierePlace;
    nouveauClient->nomUtilisateur=nomUtilisateur;
    tableClient->table[premierePlace]=nouveauClient;
    return 1;
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
    for(int i=0;i<tableClient->taille;i++){
        if(tableClient->table[i]==NULL)
        {
            return 0;
        }
    }
    return 1;
}

int nomDejaPris(TableClient* tableClient,char*nom)
{
    for (int i=0;i<tableClient->taille;i++)
    {
        if(strcmp(nom, tableClient->table[i]->nomUtilisateur)==0)
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
            free(tableClient->table[id]);
            tableClient->table[id]=NULL;
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
}


