//
// Created by louan on 21/10/2026.
//

#ifndef AWALE_LISTE_CLIENT_H
#define AWALE_LISTE_CLIENT_H

#include <pthread.h>

typedef struct Client
{
    int id;
    int mainSockfd;
    int chatSockfd;
    char* nomUtilisateur;
    int estDisponible;
    int connected;
}Client;

typedef struct TableClient
{
    Client** table;
    int taille;
    int current_size;
    int connected_size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

}TableClient;

TableClient* initialiserTableClient(int taille);
Client* ajoutNouveauClient(TableClient* tableClient, int mainSockfd,int chatSockfd, char* nomUtilisateur);//ajoute un nouveau client dans la table des clients. LA TABLE DOIT AVOIR DES PLACE DE LIBRE
int premierePlaceLibre(TableClient* tableClient);
int tablePleine(TableClient* tableClient);//dit si la table est pleine
int cannotTryConnect(TableClient *tableClient);
int nomDejaPris(TableClient* tableClient,char* nom);
Client* rechercherClientParID(TableClient* tableClient,int id);
int getExistingUserId(TableClient* tableClient, char*nom);
int connectClient(TableClient* tableClient, int id, int sockfd);
int disconnectClient(TableClient* tableClient, int id);
int retirerClient(TableClient* tableClient, int id);
void destructTableClient(TableClient* tableClient);


#endif //AWALE_LISTE_CLIENT_H
