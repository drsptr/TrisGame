#include "tris_client.h"

char griglia[9];
char simbolo_client;
char* altro_giocatore_nome;
int turno_client;
int partita_avviata;

void inizializza(int primo)
{
	int i;	
	for(i=0; i<9; i++)
		griglia[i] = ' ';
	turno_client = primo;
}

int marca(int n,char sign)
{
	if(n<1 || n>9)
		return -1;
	if(griglia[n-1] != ' ')
		return -2;
	griglia[n-1] = sign;
	return 0;
}

void stampa()
{
	int i,j;
	printf(" ___ ___ ___");
	for(i=2; i>=0; i--)
	{
		printf("\n|");
		for(j=i*3; j<((i*3)+3); j++)
			printf(" %c |",griglia[j]);
		printf("\n|___|___|___|");
	}
	printf("\n\n");
}

int controlla_vittoria(char sign)
{
	int i;	
	// controllo per righe: (1,2,3) (4,5,6) (7,8,9)
		for(i=0; i<9; i+=3)
			if((griglia[i]==sign) && (griglia[i+1]==sign) && (griglia[i+2]==sign))
				return 0;
	// controllo per colonne: (1,4,7) (2,5,8) (3,6,9)
		for(i=0; i<3; i++)
			if((griglia[i]==sign) && (griglia[i+3]==sign) && (griglia[i+6]==sign))
				return 0;
	// controllo diagonali: (1,5,9) (3,5,7)
		if((griglia[0]==sign) && (griglia[4]==sign) && (griglia[8]==sign))
			return 0;
		if((griglia[2]==sign) && (griglia[4]==sign) && (griglia[6]==sign))
			return 0;
	// controllo pareggio
		for(i=0; i<9; i++)
			if(griglia[i]==' ')
				return -1;
		return -2;
}
