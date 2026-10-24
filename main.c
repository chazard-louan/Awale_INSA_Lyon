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

    printf("\n| S1 |");
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
    printf((" S2 |\n"));
    for(i=0;i<41;i++)
    {
        printf("-");
    }
    printf("\n       ");
    for(i=1;i<=6;i++)
    {
        printf("%d    ",i);
    }
    printf("\n\t\t Joueur 2\n");
}

int main()
{
    Plateau* plateau=constructPlateau();
    Joueur* joueur1=constructJoueur(1);
    Joueur* joueur2=constructJoueur(2);
    afficherPlateau(plateau,joueur1,joueur2);
    /*int fini=0;
    while(!fini){

    }*/
    destructPlateau(plateau);
    destructJoueur(joueur1);
    destructJoueur(joueur2);
    return 0;
    
    

}