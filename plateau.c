//
// Created by louan on 21/10/2026.
//
#include "plateau.h"
#include "joueur.h"

Plateau* constructPlateau()
{
    Plateau* plateau;
    plateau=malloc(sizeof(Plateau));
    for(int i=0;i<12;i++)
    {
        plateau->champs[i]=4;
    }
    return plateau;
}

int recupererChamp(Plateau* plateau,Joueur* joueur, int champ)
{
    int camp=joueur->id-1;
    if(champ<1 || champ>6)
    {
        return -1;
    }

    return plateau->champs[champ-1+camp*6];
    
}
int coupValide(Plateau* plateau, Joueur* joueur, int champ)
{
    int indChamp;
    if(champ<1 || champ>6){
        return -1;
    }
    indChamp=(joueur->id-1)*6+(champ-1);
    if(plateau->champs[indChamp]==0){
        return 0;
    }
    return 1;
}
void jouerUnCoup(Plateau* plateau, Joueur* joueur, int champ)
{
    int indChamp;//l'indice du champ choisi dans le plateau(par rapport au tableau de champ)
    int valChamp;
    int camp;
    int trouAVerser;
    int nbChampSaute;

    camp=joueur->id-1;

    indChamp=(camp)*6+(champ-1);
    valChamp=plateau->champs[indChamp];

    nbChampSaute=valChamp/12;
    for(int graine=1; graine<=(valChamp+nbChampSaute); graine++)
    {
        trouAVerser=(graine+indChamp)%12;
        if (trouAVerser!=indChamp)
        {
            plateau->champs[trouAVerser]++;
        }
    }

// potentielle récupération des graines
    while((trouAVerser>=((camp+1)*6)%12 && trouAVerser<((camp+2)*6-1)%12) 
        && (plateau->champs[trouAVerser]==2 || plateau->champs[trouAVerser]==3))
    {
        joueur->score += plateau->champs[trouAVerser];
        plateau->champs[trouAVerser]=0;
    }

    
}

int joueurPeutJouer(Plateau* plateau, Joueur* joueur)
{
    int peutJouer=0;
    int camp=joueur->id-1;
    for(int indChamp=0;indChamp<6;indChamp++)
    {
        if (plateau->champs[indChamp+camp*6] != 0 )
        {
            peutJouer=1;
            break;
        }
    }
    return peutJouer;
}

void finPartieFamine(Plateau* plateau, Joueur* joueurFinal)
{
    int camp=joueurFinal->id-1;
    for(int indChamp=0;indChamp<6;indChamp++)
    {
       joueurFinal->score+=plateau->champs[indChamp];
    }
    
}

void destructPlateau(Plateau* plateau)
{
    if (plateau!=NULL)
    {
        free(plateau);
    }
}