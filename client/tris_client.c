#include "tris_client.h"

int socket_TCP,socket_UDP;
int porta_udp;
char nome_client[20];
struct sockaddr_in server_addr,peer_addr,my_addr;
fd_set master_rd,master_wr,rdset,wrset;
int fdmax;

// variabili di stato del client
int _recv;
int _send;
int chiudi;
int simbolo_scritto;
int dim_dati_rcv;
int dim_dati_snd;
void* buffer_send;
void* buffer_send_udp;

int main(int argc,char* argv[])
{
	int i;
	int tmp;
	int ret;
	char c;
	char in_cmd[30];
	void* buffer_recv;
	struct sockaddr_in tmp_addr;
	struct timeval timer,timer_copy;
	socklen_t addrlen;
		

	if(!argv[1] || !argv[2])
	{
		printf("Indirizzo IP o porta del server non valida\n");
		exit(-1);
	}	
	
	socket_TCP = socket(AF_INET,SOCK_STREAM,0);
	if(socket_TCP == -1)
	{
		printf("Errore nella creazione del socket TCP\n");
		exit(-1);
	}
	socket_UDP = socket(AF_INET,SOCK_DGRAM,0);
	if(socket_UDP == -1)
	{
		printf("Errore nella creazione del socket UDP\n");
		exit(-1);
	}
	// inizializzazione variabili di stato
	_recv = 0;
	_send = 0;
	chiudi = 0;
	partita_avviata = 0;
	simbolo_scritto = 0;
  // inizializzazione timer
	timer.tv_sec = 60;
	timer.tv_usec = 0;
  // inizializzazione sockaddr_in server_addr
	if(atoi(argv[2])<1024 || atoi(argv[2])>65535)
	{
		printf("Porta del server non valida\n");
		printf("Il client verrà terminato\n");
		exit(-1);
	}
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	ret = inet_pton(AF_INET,argv[1],&server_addr.sin_addr.s_addr);
	if(ret == -1)
	{
		printf("IP del server non valido\n");
		printf("Il client verrà terminato\n");
		exit(-1);
	}
  // connessione del socket TCP
	ret = connect(socket_TCP,(struct sockaddr*)&server_addr,sizeof(server_addr));
	if(ret == -1)
	{
		printf("Impossibile contattare il server\n");
		exit(-1);
	}
	printf("Connessione al server %s porta(%s) effettuata con successo\n",argv[1],argv[2]);
  // stampa del menù
	printf("\n\n");
	gestisci_cmd_stdin("!help");
	printf("\n");
  // registrazione
   registra_nome();
  	registra_porta();
  	flush(stdin);
  	printf("Registrazione effettuata con succcesso!\n\n");
  // inizializzazione sockaddr_in my_addr
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(porta_udp);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  // bind sul socket udp
	ret = bind(socket_UDP,(struct sockaddr*)&my_addr,sizeof(my_addr));
  	if(ret == -1)
  	{
  		printf("Errore nell'esecuzione della bind sul socket udp\n");
  		exit(-1);
  	}
  // azzeramento fd_set
	FD_ZERO(&master_rd);
	FD_ZERO(&master_wr);
  // settaggio fd_set
  	FD_SET(0,&master_rd);
	FD_SET(socket_TCP,&master_rd);
	fdmax = (socket_TCP > socket_UDP)? socket_TCP :socket_UDP;
	
while(chiudi==0)
{
	rdset = master_rd;
	wrset = master_wr;
	timer_copy = timer;
	c = (partita_avviata == 0)? '>' : '#';
	if(simbolo_scritto == 0)
	{	
		printf("%c ",c);
		simbolo_scritto = 1;
	}
	fflush(stdout);
	ret = select(fdmax+1,&rdset,&wrset,NULL,&timer_copy);
	if(ret == -1)
	{
		printf("Errore nell'esecuzione della select\n");
  		chiudi = 1;
	}
	if(ret==0 && partita_avviata==1)
	{
		printf("Giocatori inattivi\n");
		printf("Chiusura partita con '%s'..\n",altro_giocatore_nome);
		
		partita_avviata = 0;
		simbolo_scritto = 0;
		free(altro_giocatore_nome);
		dim_dati_snd = sizeof(int);
		buffer_send = malloc(dim_dati_snd);
		*(int*)buffer_send = htonl(CMD_TIMER);
		FD_CLR(socket_UDP,&master_rd);
		FD_SET(socket_TCP,&master_wr);
		
		printf("Partita chiusa\n\n");
		continue;
	}
	for( i=0; i<=fdmax; i++)
	{
		// settato un file descriptor in lettura
		if(FD_ISSET(i,&rdset))
		{
			// stdin
			if(i == 0)
			{
				if(partita_avviata == 0)
					string_in("","> ",in_cmd,sizeof(in_cmd));
				else
					string_in("","# ",in_cmd,sizeof(in_cmd));
				gestisci_cmd_stdin(in_cmd);
			}
			
			// socket TCP
			if(i == socket_TCP)
			{
				if(_recv == 0)
				{
					ret = recv(socket_TCP,(void*)&tmp,HEADER,0);
					if(ret == -1)
					{
						printf("Errore nella ricezione della dimensione dei dati\n");
						chiudi = 1;
						break;
					}
					if(ret == 0)
					{
						printf("Il server si e' disconnesso\n");
						chiudi = 1;
						break;
					}
					dim_dati_rcv = ntohl(tmp);
					_recv = 1;
				}
				else
				{
					buffer_recv = malloc(dim_dati_rcv);
					ret = recv(socket_TCP,buffer_recv,dim_dati_rcv,0);
					if(ret == -1)
					{
						printf("Errore nella ricezione dei dati\n");
						chiudi = 1;
						break;
					}
					if(ret == 0)
					{
						printf("Il server si e' disconnesso\n");
						chiudi = 1;
						break;
					}
					gestisci_cmd_TCP(buffer_recv);
					_recv = 0;
					free(buffer_recv);
				}
			}
			
			if(i==socket_UDP)
			{
				buffer_recv = malloc(MSG_UDP);
				addrlen = sizeof(tmp_addr);
				
				ret = recvfrom(socket_UDP,buffer_recv,MSG_UDP,0,(struct sockaddr*)&tmp_addr,&addrlen);
				if(ret == -1)
				{
					printf("Errore nella ricezione dei dati sul socket UDP\n");
					chiudi = 1;
					break;
				}
				
				// controllo sul mittente
				if(partita_avviata==1 && tmp_addr.sin_addr.s_addr==peer_addr.sin_addr.s_addr)
				{
					ret = gestisci_cmd_UDP(buffer_recv);
					if(ret == -1)
					{
						chiudi = 1;
						break;
					}
				}
				free(buffer_recv);
			}
		}
		
		
		// settato un file descriptor in scrittura
		if(FD_ISSET(i,&wrset))
		{
			if(i == socket_TCP)
			{
				if(_send == 0)
				{
					ret = invia_header(socket_TCP,dim_dati_snd);
					if(ret == -1)
					{
						printf("Errore nell'invio della dimensione sul socket TCP\n");
						chiudi = 1;
						break;
					}
					_send = 1;
				}
				else
				{
					ret = send(socket_TCP,buffer_send,dim_dati_snd,0);
					if(ret == -1)
					{
						printf("Errore nell'invio del comando al server sul socket TCP\n");
						chiudi = 1;
						break;
					}
					_send = 0;
					free(buffer_send);
					FD_CLR(socket_TCP,&master_wr);
				}
			}
			
			//socket udp
			if(i == socket_UDP)
			{
				ret = sendto(socket_UDP,buffer_send_udp,MSG_UDP,0,(struct sockaddr*)&peer_addr,sizeof(peer_addr));
				if(ret == -1)
				{
					printf("Errore nell'invio dei dati al peer sul socket UDP\n");
					chiudi = 1;
					break;
				}
				free(buffer_send_udp);
				FD_CLR(socket_UDP,&master_wr);
			}
		}
	}				
}

	close(socket_TCP);
	close(socket_UDP);
	return 0;
}
