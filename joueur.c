//
// Created by louan on 21/10/2026.
//
#include <stdlib.h>
#include "joueur.h"

Joueur* constructJoueur(int id)
{
    Joueur* joueur;
    joueur=malloc(sizeof(Joueur));
    joueur->id=id;
    joueur->score=0;
    return joueur;
}

void destructJoueur(Joueur* joueur)
{
    if (joueur!=NULL)
    {
        free(joueur);
    }
    
}