//
// Created by louan on 21/10/2026.
//

#ifndef AWALE_PLATEAU_H
#define AWALE_PLATEAU_H

#include<stdlib.h>
#include"joueur.h"
typedef struct Plateau{
  int camps[12];
}Plateau;
//construction
Plateau* constructPlateau();
//jeu
int jouerUnCoup(Plateau* plateau, Joueur* joueur, int champ);
int champ(Plateau* plateau,int champ);
int joueurPeutJouer(Plateau* plateau, Joueur* joueur);
int finPartie(Plateau* plateau, Joueur* joueurFinal);
//déconstruction
void destructPlateau(Plateau* plateau);


#endif //AWALE_PLATEAU_H
