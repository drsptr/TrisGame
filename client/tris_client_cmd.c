#include "tris_client.h"

void string_in(char* msg_init,char* error_msg,char* string,int max_char)
{
	int i;
	
	while(1)
	{
		printf("%s",msg_init);
		for( i=0; i<max_char-1; i++)
		{
			scanf("%c",&string[i]);
			if(string[i] == '\n')
				break;
		}
		if(string[0] != '\n')
		{
			string[i] = '\0';
			break;
		}
		else
			printf("%s",error_msg);
	}
}

int invia_header(int socket,int msg)
{
	int tmp;
	
	tmp = htonl(msg);
	return send(socket,(void*)&tmp,HEADER,0);
}

void registra_nome()
{
	int dim;
	int ret;
	int valido = REG_NOME_ERR;
	void* buffer;
	char tmp_nome[MAX_NOME];
	
	while(valido == REG_NOME_ERR)
	{
		string_in("Inserisci il tuo nome: ","Nome non valido!\n\n",tmp_nome,sizeof(tmp_nome));
		dim = 4 + strlen(tmp_nome) + 1;
		ret = invia_header(socket_TCP,dim);
		if(ret == -1)
		{
			printf("Errore nell'invio della dimensione del nome\n");
			exit(-1);
		}
	
		buffer = malloc(dim);
		*(int*)buffer = htonl(CMD_REG_NOME);
		strcpy((char*)(buffer+4),tmp_nome);
		ret = send(socket_TCP,buffer,dim,0);
		if(ret == -1)
		{
			printf("Errore nell'invio dei dati\n");
			exit(-1);
		}
		free(buffer);
		
		// verifica se il nome scelto e' valido oppure no
		buffer = malloc(sizeof(int));
		ret = recv(socket_TCP,buffer,sizeof(int),0);
		if(ret == -1)
		{
			printf("Errore nella ricezione della conferma sul nome\n");
			exit(-1);
		}
		if(ret == 0)
		{
			printf("Il server si e' disconnesso");
			exit(-1);
		}
		
		valido = ntohl(*(int*)buffer);
		free(buffer);
		if(valido == REG_NOME_ERR)
		{
			printf("Impossibile registrarsi come '%s'\n",tmp_nome);
			printf("Un altro utente utilizza già questo nome\n\n");
		}
	}
}


void registra_porta()
{
	int ret;
	int dim;
	void* buffer;
	
	dim = sizeof(int) + sizeof(int);
	ret = invia_header(socket_TCP,dim);
	if(ret == -1)
	{
		printf("Errore nell'invio della dimensione della porta\n");
		exit(-1);
	}
	
	printf("Inserisci la porta udp da usare: ");
	scanf("%d",&porta_udp);
	while(porta_udp<1023 || porta_udp>65535)
	{	
		printf("Inserisci una porta udp valida [1023,65535]:");
		scanf("%d",&porta_udp);
	}
	buffer = malloc(dim);
	*(int*)(buffer) = htonl(CMD_REG_PORTA);
	*(int*)(buffer+4) = htonl(porta_udp);
	ret = send(socket_TCP,buffer,dim,0);
	if(ret == -1)
	{
		printf("Errore nell'invio dei dati\n");
		exit(-1);
	}
	free(buffer);
}

void gestisci_cmd_stdin(char* cmd)
{	
	int tmp;
	int ret;
	
	if(strcmp(cmd,"!help") == 0)
	{
		simbolo_scritto = 0;
		printf("Sono disponibili i seguenti comandi:\n");
		printf(" * !help  -->  mostra l'elenco dei comandi disponibili\n");
		printf(" * !who  -->  mostra l'elenco dei client connessi al server\n");
		printf(" * !connect NOME_CLIENT  -->  avvia una partita con l'utente NOME_CLIENT\n");
		printf(" * !disconnect  -->  disconnette il client dall'attuale partita intrapesa con un altro peer\n");
		printf(" * !quit  -->  disconnette il client dal server\n");
		printf(" * !show_map  -->  mostra la mappa di gioco\n");
		printf(" * !hit NUM_CELL  -->  marca la casella NUM_CELL (valido solo quando è il proprio turno)\n");
		printf("\n");
		return;
	}
	
	
	
	if(strcmp(cmd,"!show_map") == 0)
	{
		simbolo_scritto = 0;
		if(partita_avviata == 1)
			stampa();
		else
			printf("Nessuna partita in corso\n");
		printf("\n");
		return;
	}
	
	
	
	if(strcmp(cmd,"!quit") == 0)
	{
		printf("Chiusura del client\n");
		chiudi = 1;
		return;
	}
	
	
	
	if(strcmp(cmd,"!who") == 0)
	{
		dim_dati_snd = sizeof(int);
		buffer_send = malloc(dim_dati_snd);
		*(int*)buffer_send = htonl(CMD_WHO);
		FD_SET(socket_TCP,&master_wr);
		return;
	}
	
	
	
	if(strncmp(cmd,"!connect ",strlen("!connect ")) == 0)
	{
		if(partita_avviata==0)
		{
			if(strlen(cmd) > strlen("!connect "))
			{
				tmp = strlen(cmd) - strlen("!connect ") + 1;
				altro_giocatore_nome = malloc(tmp);
				strcpy(altro_giocatore_nome,cmd + strlen("!connect "));
			
				dim_dati_snd = sizeof(int) + tmp;
				buffer_send = malloc(dim_dati_snd);
				*(int*)buffer_send = htonl(CMD_CONNECT);
				strcpy((char*)(buffer_send + 4),altro_giocatore_nome);
				FD_SET(socket_TCP,&master_wr);
				return;
			}	
		
			simbolo_scritto = 0;
			printf("Nome giocatore non valido!\n\n");
			return;
		}
		
		simbolo_scritto = 0;
		printf("E' in corso gia' una partita!\n");
		printf("Impossibile connettersi con 2 utenti contemporaneamente!\n\n");
		return;
	}
	
	
	
	if(strcmp(cmd,"!disconnect") == 0)
	{
		if(partita_avviata==0)
			printf("Nessuna partita in corso!\n");
		else
		{
			printf("HAI PERSO!\n");
			partita_avviata = 0;
			free(altro_giocatore_nome);
			dim_dati_snd = sizeof(int);
			buffer_send = malloc(dim_dati_snd);
			*(int*)buffer_send = htonl(CMD_DISCONNECT);
			FD_CLR(socket_UDP,&master_rd);
			FD_SET(socket_TCP,&master_wr);		
		}
		
		simbolo_scritto = 0;
		printf("\n");
		return;
	}
	
	
	
	if(strncmp(cmd,"!hit ",strlen("!hit ")) == 0)
	{
		if(partita_avviata == 1)
		{
			if(turno_client == 1)
			{
				if(strlen(cmd) > strlen("!hit "))
				{
					tmp = atoi((cmd + strlen("!hit ")));
					ret = marca(tmp,simbolo_client);
					if (ret == 0)
					{
						turno_client = 0;
						printf("Hai marcato la casella #%d\n",tmp);
						
						buffer_send_udp = malloc(MSG_UDP);
						*(int*)buffer_send_udp = htonl(UDP_CMD_HIT);
						*(int*)(buffer_send_udp + 4) = htonl(tmp);
						FD_SET(socket_UDP,&master_wr);
						
						ret = controlla_vittoria(simbolo_client);
						if(ret != -1)
						{
							dim_dati_snd = 2 * sizeof(int);
							buffer_send = malloc(dim_dati_snd);
							*(int*)buffer_send = htonl(CMD_FINE);
							if(ret == 0)
							{
								printf("HAI VINTO!\n");
								*(int*)(buffer_send + 4) = htonl(CODICE_VITTORIA);
							}
							else
							{
								printf("PAREGGIO!\n");
								*(int*)(buffer_send + 4) = htonl(CODICE_PAREGGIO);
							}
							FD_SET(socket_TCP,&master_wr);
							
							partita_avviata = 0;
							free(altro_giocatore_nome);
							FD_CLR(socket_UDP,&master_rd);
						}
						else
						{
							printf("\nE' il turno di '%s'\n",altro_giocatore_nome);
							return;
						}
					}
					else if(ret == -1)
						printf("Numero della casella errato\n");
					else
						printf("La casella #%d e' già occupata\n",tmp);
				}
				else
					printf("Numero di casella errato\n");
			}
			else
				printf("E' il turno di '%s'!\n",altro_giocatore_nome);
		}
		else
			printf("Nessuna partita in corso!\n");
		
		simbolo_scritto = 0;
		printf("\n");
		return;
	}
	
	
	
	simbolo_scritto = 0;
	printf("'%s': comando sconosciuto\n\n",cmd);
}

void gestisci_cmd_TCP(void* buffer)
{
	int cmd;
	int i;
	int quanti;
	int dim;
	int tmp;
	char c;
	char* tmp_str;

	cmd = ntohl(*(int*)buffer);
	switch(cmd)
	{	
		// l'altro giocatore ha abbandonato la partita
		case CMD_CLIENT_DISCONNECTED:
				printf("L'utente '%s' ha abbandonato la partita\n",altro_giocatore_nome);
				printf("HAI VINTO!\n\n");
				partita_avviata = 0;
				simbolo_scritto = 0;
				free(altro_giocatore_nome);
				FD_CLR(socket_UDP,&master_rd);
			break;
		
		
	
		// ricezione del comando !who: stampa i nomi e lo stato
		case CMD_WHO:
				quanti = ntohl(*(int*)(buffer + 4));
				dim = 8;
				
				printf("Utenti connessi al server: ");
				for( i=0; i<quanti; i++)
				{
					tmp = strlen((char*)(buffer + dim));
					printf("%s",(char*)(buffer + dim));
					if(i+1 < quanti)
						printf(", ");
					dim += tmp + 1;
				}
				printf("\n\n");
				simbolo_scritto = 0;
			break;
			
			
			
		// ricezione del risultato del comando !connect
		case CMD_CONNECT:
				cmd = ntohl(*(int*)(buffer + 4));
				
				switch(cmd)
				{
					case ERR_CONNECT_CLIENT_INESISTENTE:
							printf("L'utente '%s' non esiste\n",altro_giocatore_nome);
							free(altro_giocatore_nome);
						break;
						
					case ERR_CONNECT_CLIENT_STESSO:
							printf("Non puoi avviare una partita con te stesso\n");
							free(altro_giocatore_nome);
						break;
						
					case ERR_CONNECT_CLIENT_OCCUPATO:
							printf("L'utente '%s' e' impegnato in un'altra partita\n",altro_giocatore_nome);
							free(altro_giocatore_nome);
						break;
						
					case REQ_CONNECT_CLIENT_SENT:
							printf("La richiesta e' stata inviata all'utente '%s'\n",altro_giocatore_nome);
							printf("Attendi..\n");
						return;
				}
				printf("\n");
				simbolo_scritto = 0;
			break;
			
			
			
		// si riceve una richiesta di avvio di una partita	
		case CMD_REQUEST:
				printf("L'utente '%s' vuole giocare con te, accetti (y/n)? ",(char*)(buffer + 4));
				scanf("%c",&c);
				flush(stdin);
				while(c!='y' && c!='n')
				{
					printf("Risposta non valida, inserisci y/n: ");
					scanf("%c",&c);
					flush(stdin);
				}
				
				if(c=='y')
				{
					altro_giocatore_nome = malloc(strlen((char*)(buffer + 4)) + 1);
					strcpy(altro_giocatore_nome,(char*)(buffer + 4));
					printf("Hai accettato di giocare con '%s'\n",altro_giocatore_nome);
				}
				else
				{
					simbolo_scritto = 0;
					printf("Hai rifiutato la richiesta di '%s'\n\n",(char*)(buffer + 4));
				}
				dim_dati_snd = sizeof(int) + sizeof(char);
				buffer_send = malloc(dim_dati_snd);
				*(int*)buffer_send = htonl(CMD_RESPONSE);
				*(char*)(buffer_send + 4) = c;
				FD_SET(socket_TCP,&master_wr);
			break;
			
			
		
		// si riceve la risposta alla richiesta di avvio
		case CMD_RESPONSE:
				 cmd = ntohl(*(int*)(buffer + 4));
				 
				 switch(cmd)
				 {
				 	// MSG_RESP_CLIENT_ADDR: questo è il client che ha risposto positivamente
				 	// MSG_RESP_CLIENT_POS: il client richiedente riceve esito positivo
				 	case MSG_RESP_CLIENT_ADDR: case MSG_RESP_CLIENT_POS:
				 			memset(&peer_addr,0,sizeof(peer_addr));
				 			peer_addr.sin_family = AF_INET;
				 			peer_addr.sin_addr.s_addr = ntohl(*(int*)(buffer + 8)); // IP address
				 			peer_addr.sin_port = htons(ntohl(*(int*)(buffer + 12))); //porta
				 			tmp_str = (char*)malloc(INET_ADDRSTRLEN + 1);
				 			inet_ntop(AF_INET,(void*)&peer_addr.sin_addr,tmp_str,INET_ADDRSTRLEN);
				 			if(cmd == MSG_RESP_CLIENT_POS)
				 				printf("L'utente '%s' ha accettato di giocare con te\n",altro_giocatore_nome);
							printf("Partita avviata con '%s' (%s, %d)\n",altro_giocatore_nome,tmp_str,((uint32_t)ntohs(peer_addr.sin_port)));
				 			free(tmp_str);
				 			
				 			partita_avviata = 1;
				 			simbolo_client = (cmd == MSG_RESP_CLIENT_ADDR)? 'O' : 'X';
				 			tmp = (cmd == MSG_RESP_CLIENT_ADDR)? 0 : 1;
				 			inizializza(tmp);
				 			
				 			buffer_send_udp = malloc(MSG_UDP);
				 			*(int*)buffer_send_udp = htonl(UDP_CMD_START);
				 			FD_SET(socket_UDP,&master_rd);
				 			FD_SET(socket_UDP,&master_wr);
				 			
				 			printf("Il tuo simbolo: '%c'\n",simbolo_client);
				 		break;
				 		
				 	case MSG_RESP_CLIENT_NEG:
				 			printf("L'utente '%s' ha rifiutato di giocare con te\n\n",altro_giocatore_nome);
				 			free(altro_giocatore_nome);
				 			simbolo_scritto = 0;
				 }
			break;
			
			
			
		case CMD_FINE:
				if(partita_avviata == 1)
				{
					cmd = ntohl(*(int*)(buffer + 4));
					if(cmd == CODICE_VITTORIA)
						printf("HAI PERSO!\n");
					else
						printf("PAREGGIO!\n");
					partita_avviata = 0;
					simbolo_scritto = 0;
					free(altro_giocatore_nome);
					FD_CLR(socket_UDP,&master_rd);
					printf("\n");
				}
			break;
			
	}
}

int gestisci_cmd_UDP(void* buffer)
{
	int cmd = ntohl(*(int*)buffer);
	int tmp;
	int ret;
	char altro_giocatore_simbolo;
	
	switch(cmd)
	{
		case UDP_CMD_START:
				if(turno_client==1)
					printf("\nE' il tuo turno");
				else
				{
					printf("\nE' il turno di '%s'\n",altro_giocatore_nome);
					return 0;
				}
					
				simbolo_scritto = 0;
				printf("\n");
			break;
		
		
		
		case UDP_CMD_HIT:
				turno_client = 1;
				altro_giocatore_simbolo = (simbolo_client == 'X')? 'O': 'X';
				tmp = ntohl(*(int*)(buffer + 4));
				
				ret = marca(tmp,altro_giocatore_simbolo);
				if(ret != 0)
					return -1;
				
				printf("'%s' ha marcato la casella #%d\n",altro_giocatore_nome,tmp);
				ret = controlla_vittoria(altro_giocatore_simbolo);
				if(ret != -1)
				{
					if(ret == 0)
						printf("HAI PERSO!\n");
					else
						printf("PAREGGIO!\n");
					partita_avviata = 0;
					free(altro_giocatore_nome);
					FD_CLR(socket_UDP,&master_rd);
				}
				else
					printf("\nE' il tuo turno");
				
				
				simbolo_scritto = 0;
				printf("\n");				
			break;
			
			
				
		default:
			printf("Comando udp sconosciuto\n");
			return -1;
	}
	return 0;
}
