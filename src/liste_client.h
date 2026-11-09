//
// Created by louan on 21/10/2026.
//

#ifndef AWALE_LISTE_CLIENT_H
#define AWALE_LISTE_CLIENT_H

#include <pthread.h>

typedef struct Client
{
    int id;
    int sockfd;
    char* nomUtilisateur;
    int estDisponible;
}Client;

typedef struct TableClient
{
    Client** table;
    int taille;
    int current_size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

}TableClient;

TableClient* initialiserTableClient(int taille);
int ajoutNouveauClient(TableClient* tableClient,int sockfd, char* nomUtilisateur);//ajoute un nouveau client dans la table des clients. LA TABLE DOIT AVOIR DES PLACE DE LIBRE
int premierePlaceLibre(TableClient* tableClient);
int tablePleine(TableClient* tableClient);//dit si la table est pleine
int nomDejaPris(TableClient* tableClient,char* nom);
Client* rechercherClientParID(TableClient* tableClient,int id);
int retirerClient(TableClient* tableClient, int id);
void destructTableClient(TableClient* tableClient);


#endif //AWALE_LISTE_CLIENT_H
