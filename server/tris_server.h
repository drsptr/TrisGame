#include <stdlib.h>   	// malloc(), free(), system()
#include <stdio.h>    	// printf(), fopen(), fclose(), ...
#include <string.h>   	// strlen(), strncpy(), ...
#include <sys/stat.h> 	// stat()
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>

#define HEADER 4
#define CODICE_VITTORIA 0
#define CODICE_PAREGGIO 1
#define CMD_CLIENT_DISCONNECTED -1
#define CMD_REG_NOME 0
	#define REG_NOME_ERR 0
	#define REG_NOME_OK 1
#define CMD_REG_PORTA 1
#define CMD_WHO 2
#define CMD_CONNECT 3
	#define ERR_CONNECT_CLIENT_INESISTENTE 0
	#define ERR_CONNECT_CLIENT_STESSO 1
	#define ERR_CONNECT_CLIENT_OCCUPATO 2
	#define REQ_CONNECT_CLIENT_SENT 3
#define CMD_REQUEST 4
#define CMD_RESPONSE 5
	#define MSG_RESP_CLIENT_ADDR 0
	#define MSG_RESP_CLIENT_POS 1
	#define MSG_RESP_CLIENT_NEG 2
#define CMD_DISCONNECT 6
#define CMD_FINE 7
#define CMD_TIMER 8


struct client
{
	int recv; // se 0 si riceve la dimensione, se 1 si ricevono i dati veri e propri
	int send; // se 0 si invia la dimensione, se 1 si inviano i dati veri e propri
	int dim_op;
	int reg;
	int id;
	int id_altro_giocatore;
	int libero;
	int chiudi;
	int porta_udp;
	char* nome;
	void* buffer_invio;
	struct sockaddr_in client_addr;
	struct client* pointer;
};

extern struct client* lista;
extern fd_set master_rd,master_wr,rdset,wrset;
extern int fdmax;

int invia_header(int,int);
void stampa();
void aggiungi(int,struct sockaddr_in);
int elimina(int);
struct client* cerca(int);
struct client* cerca_per_nome(char*);
int avverti(int);
void gestisci_cmd(struct client*,void*);
