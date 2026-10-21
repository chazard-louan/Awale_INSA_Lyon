//
// Created by louan on 21/10/2026.
//

#ifndef AWALE_PLATEAU_H
#define AWALE_PLATEAU_H

#include<stdlib.h>
#include"joueur.h"
typedef struct Plateau
{
  int champs[12];
} Plateau;
//construction
Plateau* constructPlateau();
//jeu
int coupValide(Plateau* plateau, Joueur* joueur, int champ);//champ entre 1 et 6
// renvoie -1 si en dehors de limite possible , 0 si champ vide, 1 si coup possible
void jouerUnCoup(Plateau* plateau, Joueur* joueur, int champ);//champ entre 1 et 6
int recupererChamp(Plateau* plateau, Joueur* joueur, int champ);
int joueurPeutJouer(Plateau* plateau, Joueur* joueur);
void finPartie(Plateau* plateau, Joueur* joueurFinal);
//déconstruction
void destructPlateau(Plateau* plateau);


#endif //AWALE_PLATEAU_H
