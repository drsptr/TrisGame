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

#define MAX_NOME 21
#define HEADER 4
#define MSG_UDP 8
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
#define UDP_CMD_START 0
#define UDP_CMD_HIT 1
#define flush(stdin) while ((getchar()) != '\n')



// variabili
extern char griglia[9];
extern char simbolo_client;
extern char* altro_giocatore_nome;
extern int turno_client;
extern int partita_avviata;

extern char nome_client[20];
extern int porta_udp;
extern int socket_TCP,socket_UDP;
extern struct sockaddr_in server_addr,peer_addr;
extern fd_set master_rd,master_wr,rdset,wrset;
extern int fdmax;
extern int _recv;
extern int _send;
extern int chiudi;
extern int simbolo_scritto;
extern int dim_dati_rcv;
extern int dim_dati_snd;
extern void* buffer_send;
extern void* buffer_send_udp;


// funzioni
void inizializza(int);
int marca(int,char);
void stampa();
int controlla_vittoria(char);

void string_in(char*,char*,char*,int);
int invia_header(int,int);
void registra_nome();
void registra_porta();
void gestisci_cmd_stdin(char*);
void gestisci_cmd_TCP(void*);
int gestisci_cmd_UDP(void*);
