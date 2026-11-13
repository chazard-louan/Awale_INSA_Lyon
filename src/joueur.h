//
// Created by louan on 21/10/2026.
//

#ifndef AWALE_JOUEUR_H
#define AWALE_JOUEUR_H
#include "constants.h"

struct Joueur
{
  int id;
  int score;
  char* name;
};
typedef struct Joueur Joueur;
Joueur* constructJoueur(int id);
void destructJoueur(Joueur* joueur);

#endif //AWALE_JOUEUR_H
