#include <stdio.h>
#include <stdlib.h>
#include "joueur.h"
#include "plateau.h"


void afficherPlateau(Plateau* plateau, Joueur* joueur1, Joueur* joueur2)
{
    int i;
    printf("\t\t Joueur 1\n");
    printf("        ");
    for(i=6; i>0;i--)
    {
        printf("%d    ",i);
        
    }

     printf("\n");
    for(i=0;i<41;i++)//je ne sais pas quoi mettre a la place don je laisse \
41
        printf("_");

    printf("\n| S%d |",joueur1->id);
    for(i=6;i>=1;i--)
    {
        printf(" %2d |",recupererChamp(plateau,joueur1,i));
    }
    printf("    |\n");

    printf("| %2d |", joueur1->score);

    for (i=0; i<29;i++)
    {
        printf("-");
    }
    printf("| %2d |\n",joueur2->score);
    printf("|    |");
    for(i=1;i<=6;i++)
    {
        printf(" %2d |",recupererChamp(plateau,joueur2,i));
    }
    printf(" S%d |\n",joueur2->id);
    for(i=0;i<41;i++)
    {
        printf("-");
    }
    printf("\n        ");
    for(i=1;i<=6;i++)
    {
        printf("%d    ",i);
    }
    printf("\n\t\t Joueur 2\n");
}

void boucleJeu(Plateau* plateau,Joueur** joueurs,int coteDepart)
{
    int coup;
    int isValide;
    int fini=0;
    int joueurQuiJoue=coteDepart;
    while(!fini)
    {
        printf("\n");
        afficherPlateau(plateau,joueurs[0],joueurs[1]);
        printf("\n");

        isValide=0;
        while(isValide!=1)
	    {
            printf("coup du joueur %d: \t",joueurs[joueurQuiJoue]->id);
            scanf("%d",&coup);
            isValide=coupValide(plateau,joueurs[joueurQuiJoue],coup);
            if(isValide==0)
            {
                printf("coup en dehors des limites\n");
            }
            else if(isValide==-1)
            {
                printf("champs vide\n");
            }
            else if(isValide==-2)
            {
                printf("affame l'adversaire\n");
            }

        }
        jouerUnCoup(plateau, joueurs[joueurQuiJoue], coup);
        if(joueurs[joueurQuiJoue]->score>=25){
            fini=1;
        }
        joueurQuiJoue=(joueurQuiJoue+1)%2;
        if(!joueurPeutJouer(plateau,joueurs[joueurQuiJoue]))
        {
            afficherPlateau(plateau,joueurs[0],joueurs[1]);
            finPartieFamine(plateau, joueurs[joueurQuiJoue]);
            
            fini=1;
        }
        

      
    }
    
    
    



}
void jeuClassique()
{
    Plateau* plateau=constructPlateau();
    Joueur** joueurs=malloc(sizeof(Joueur*)*2);
    Joueur* joueur1=constructJoueur(1);
    Joueur* joueur2=constructJoueur(2);
    int coup;
    int isValide;
    joueurs[0]=joueur1;
    joueurs[1]=joueur2;
    
    boucleJeu(plateau,joueurs,0);

    afficherPlateau(plateau,joueurs[0],joueurs[1]);
    destructPlateau(plateau);
    destructJoueur(joueur1);
    destructJoueur(joueur2);
    free(joueurs);

}

void testcollecte()
{
    Plateau* plateau=constructPlateau();
    Joueur** joueurs=malloc(sizeof(Joueur*)*2);
    joueurs[0]=constructJoueur(1);
    joueurs[1]=constructJoueur(2);
    plateau->champs[6]=1;
    plateau->champs[8]=10;
    afficherPlateau(plateau,joueurs[0],joueurs[1]);
    jouerUnCoup(plateau, joueurs[1],3 );
    afficherPlateau(plateau,joueurs[0],joueurs[1]);
    jouerUnCoup(plateau, joueurs[0], 2);
    afficherPlateau(plateau,joueurs[0],joueurs[1]);
    destructPlateau(plateau);
    destructJoueur(joueurs[0]);
    destructJoueur(joueurs[1]);
    free(joueurs);
}
void testFinPartie(){
    Plateau* plateau=constructPlateau();
    Joueur** joueurs=malloc(sizeof(Joueur*)*2);
    joueurs[0]=constructJoueur(1);
    joueurs[1]=constructJoueur(2);
    joueurs[1]->score=18;
    joueurs[0]->score=24;
    plateau->champs[0]=3;
    plateau->champs[1]=0;
    plateau->champs[2]=0;
    plateau->champs[3]=0;
    plateau->champs[4]=0;
    plateau->champs[5]=0;
    plateau->champs[6]=0;
    plateau->champs[7]=0;
    plateau->champs[8]=0;
    plateau->champs[9]=0;
    plateau->champs[10]=0;
    plateau->champs[11]=3;
    
    boucleJeu(plateau,joueurs, 1);

    afficherPlateau(plateau,joueurs[0],joueurs[1]);




    destructPlateau(plateau);
    destructJoueur(joueurs[0]);
    destructJoueur(joueurs[1]);
    free(joueurs);

}

int main()
{
    //jeuClassique();
    //testcollecte();
    testFinPartie();
    
    
    return 0;
    
    

}
