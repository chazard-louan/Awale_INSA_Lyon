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
Plateau* copiePlateau(Plateau* plateau){
    Plateau* copie=malloc(sizeof(Plateau));//pas d'appel de construct plateau pour sauter l'étape de l'initialisation des champs
    for(int i=0;i<12;i++)
    {
        copie->champs[i]=plateau->champs[i];
    }
    return copie;
}

int campVide(Plateau* plateau,int idJoueur)
{//permet de dire si un des deux camps est vide (repéré par l'identifiant de son joueur)  
    int estVide=1;
    int camp=idJoueur-1;
    for(int indChamp=0;indChamp<6;indChamp++)
    {
        if (plateau->champs[indChamp+camp*6] != 0 )
        {
            estVide=0;
            break;
        }
    }
    return estVide;

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
    int indChamp;//l'indice du champ choisi dans le plateau(par rapport au tableau de champ donc de 0 à 12)
    int valChamp;
    Joueur* doublure;
    Plateau* plateauCoupJoue;
    if(champ<1 || champ>6){
        return 0;
    }
    indChamp=(joueur->id-1)*6+(champ-1);
    valChamp=plateau->champs[indChamp];
    if(valChamp==0){
        return -1;
    }
    doublure=constructJoueur(joueur->id);
    plateauCoupJoue=copiePlateau(plateau);
    jouerUnCoup(plateauCoupJoue, doublure,champ);
    destructJoueur(doublure);
    if(campVide(plateauCoupJoue,(joueur->id%2)+1))//(joueur->id%2)+1 correspond à l'id de l'autre joueur SI ON SUPPOSE QUE LES IDENTIFIANT SONT 1 ET 2
    {
        destructPlateau(plateauCoupJoue);

        return -2;
    }
    destructPlateau(plateauCoupJoue);
    
    return 1;
}

void jouerUnCoup(Plateau* plateau, Joueur* joueur, int champ)// pour lancer cette fonction, coup valide ne doit pas renvoyer 0 ou -1
{
    int indChamp;//l'indice du champ choisi dans le plateau(par rapport au tableau de champ)
    int valChamp;
    int camp;
    int trouAVerser;
    int nbChampSaute;

    camp=joueur->id-1;

    indChamp=(camp)*6+(champ-1);
    valChamp=plateau->champs[indChamp];
    plateau->champs[indChamp]=0;

    nbChampSaute=valChamp/12;
    for(int graine=1; graine<=(valChamp+nbChampSaute); graine++)// le for traverse tous les champs (si le nombre de graine le permet) mais ne donne pas de graine dans le champ d'origine
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
        trouAVerser--;
    }

    
}

int joueurPeutJouer(Plateau* plateau, Joueur* joueur)
{//sert notamment à proc la fin par famine 
    int peutJouer=0;
    int camp=joueur->id-1;
    for(int indChamp=1;indChamp<=6;indChamp++)
    {
        if (coupValide(plateau,joueur,indChamp) == 1 )
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
       joueurFinal->score+=plateau->champs[indChamp+camp*6];
       plateau->champs[indChamp+camp*6]=0;
    }
    
}



void destructPlateau(Plateau* plateau)
{
    if (plateau!=NULL)
    {
        free(plateau);
    }
}
