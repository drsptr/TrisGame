#include "tris_server.h"

int invia_header(int socket,int msg)
{
	int tmp;
	
	tmp = htonl(msg);
	return send(socket,(void*)&tmp,HEADER,0);
}

void stampa()
{
	struct client* p;
	for( p=lista; p!=NULL; p=p->pointer)
			printf("%s\t%d\n",p->nome,p->id);
			printf("\n\n");
}

void aggiungi(int id,struct sockaddr_in addr)
{
	struct client* nuovo = (struct client*)malloc(sizeof(struct client));
	nuovo->recv = 0;
	nuovo->send = 0;
	nuovo->reg = 0;
	nuovo->id = id;
	nuovo->chiudi = 0;
	nuovo->libero = 1;
	nuovo->porta_udp = 0;
	nuovo->buffer_invio = NULL;
	nuovo->client_addr = addr;
	nuovo->pointer = lista;
	lista = nuovo;
}

int elimina(int id)
{
	struct client* p,*q;
	for( q=lista; q!=NULL && q->id!=id; q=q->pointer)
		p=q;
	if(q!=NULL)
	{
		if(q==lista)
			lista = q->pointer;
		else
			p->pointer = q->pointer;
		if(q->nome != NULL)
			free(q->nome);
		if(q->buffer_invio != NULL)
			free(q->buffer_invio);
		free(q);
		return 1;
	}
	return -1;
}

struct client* cerca(int id)
{	
	struct client* p;
	for( p=lista; p!=NULL; p=p->pointer)
		if(p->id == id)
			break;
	return p;
}

struct client* cerca_per_nome(char* nome)
{
	struct client* p;
	for( p=lista; p!=NULL; p=p->pointer)
		if( p->nome!=NULL && strcmp(p->nome,nome)==0 )
			break;
	return p;
}

int avverti(int i)
{
	struct client* p = cerca(i);
	
	if(p==NULL) return 0;
	if(p->buffer_invio!=NULL) return -1;
	
	p->libero = 1;	
	p->buffer_invio = malloc(2 * sizeof(int));
	*(int*)p->buffer_invio = sizeof(int);
	*(int*)(p->buffer_invio + 4) = htonl(CMD_CLIENT_DISCONNECTED);
	FD_SET(p->id,&master_wr);
	printf("'%s' e' libero\n",p->nome);
	return 0;
}

void gestisci_cmd(struct client* cl,void* buffer)
{
	int cmd;
	int dim;
	int quanti;
	char c;
	struct client* p;
	
	cmd = ntohl(*(int*)buffer);
	
	switch(cmd)
	{
		// il client 'cl' chiede di registrare il nome
		case CMD_REG_NOME:
				p = cerca_per_nome((char*)(buffer+4));
				cl->buffer_invio = malloc(2 * sizeof(int));
				*(int*)cl->buffer_invio = sizeof(int);
				
				if(p==NULL)
				{
					cl->nome = malloc(strlen((char*)(buffer+4))+1);
					strcpy(cl->nome,(char*)(buffer+4));
					*(int*)(cl->buffer_invio + 4) = htonl(REG_NOME_OK);
				}
				else
					*(int*)(cl->buffer_invio + 4) = htonl(REG_NOME_ERR);

				cl->send = 1;
				FD_SET(cl->id,&master_wr);
			break;
		
		
		
		// il client 'cl' chiede di registrare la porta udp
		case CMD_REG_PORTA:
				cl->porta_udp = ntohl(*(int*)(buffer+4));
				cl->reg = 1;
				printf("'%s' si e' connesso\n",cl->nome);
				printf("'%s' e' libero\n",cl->nome);
			break;
		
			
			
		// il client 'cl' chiede la lista di utenti connessi al server
		case CMD_WHO:
				dim = 0;
				quanti = 0;
				
				for( p=lista; p!=NULL; p=p->pointer)
				{	
					if(p->reg == 1)
					{
						dim += strlen(p->nome) + 5;
						quanti++;
					}
				}
					
				cl->buffer_invio = malloc(12 + dim);
				*(int*)cl->buffer_invio = 8 + dim;
				*(int*)(cl->buffer_invio + 4) = htonl(CMD_WHO);
				*(int*)(cl->buffer_invio + 8) = htonl(quanti);
				
				dim = 0;
				for( p=lista; p!=NULL; p=p->pointer)
				{
					if(p->reg==1)
					{
						strcpy((char*)(cl->buffer_invio + 12 + dim),p->nome);
						dim += strlen(p->nome);
						if(p->libero == 1)
							strcat((char*)(cl->buffer_invio + 12 + dim)," (L)");
						else
							strcat((char*)(cl->buffer_invio + 12 + dim)," (O)");
						dim += 5;
					}
				}
				FD_SET(cl->id,&master_wr);
				printf("'%s' ha richiesto la lista degli utenti connessi al server\n",cl->nome);
			break;
			
		

		// il client 'cl' chiede di avviare una partita con un altro utente
		case CMD_CONNECT:
				p = cerca_per_nome((char*)(buffer + 4));
				cl->buffer_invio = malloc(3 * sizeof(int));
				*(int*)cl->buffer_invio = 2 * sizeof(int);
				*(int*)(cl->buffer_invio + 4) = htonl(CMD_CONNECT);
				
				// client cercato inesistente o non ancora registrato
				if(p==NULL || p->reg==0)
					*(int*)(cl->buffer_invio + 8) = htonl(ERR_CONNECT_CLIENT_INESISTENTE);
				
				else
				{
					// partita con sè stesso
					if(strcmp(cl->nome,p->nome)==0)
						*(int*)(cl->buffer_invio + 8) = htonl(ERR_CONNECT_CLIENT_STESSO);
					
					// altro client già occupato
					else if(p->libero==0)
						*(int*)(cl->buffer_invio + 8) = htonl(ERR_CONNECT_CLIENT_OCCUPATO);
						
					// invio richiesta all'altro client
					else
					{
						cl->libero = 0;
						p->libero = 0;
						
						cl->id_altro_giocatore = p->id;
						p->id_altro_giocatore = cl->id;
						
						*(int*)(cl->buffer_invio + 8) = htonl(REQ_CONNECT_CLIENT_SENT);
						
						p->buffer_invio = malloc(2 * sizeof(int) + strlen(cl->nome) + 1);
						*(int*)p->buffer_invio = sizeof(int) + strlen(cl->nome) + 1;
						*(int*)(p->buffer_invio + 4) = htonl(CMD_REQUEST);
						strcpy((char*)(p->buffer_invio + 8),cl->nome);
						FD_SET(p->id,&master_wr);
						printf("'%s' ha chiesto di connettersi ad un altro utente\n",cl->nome);
					}
				}
				
				FD_SET(cl->id,&master_wr);
			break;



		// il client 'cl' ha risposto alla richiesta di avvio di una partita
		case CMD_RESPONSE:
				p = cerca(cl->id_altro_giocatore);
				c = *(char*)(buffer + 4);
				
				if(p!=NULL)
				{
					if(c=='y')
					{
						cl->buffer_invio = malloc(5 * sizeof(int));
						*(int*)cl->buffer_invio = 4 * sizeof(int);
						*(int*)(cl->buffer_invio + 4) = htonl(CMD_RESPONSE);
						*(int*)(cl->buffer_invio + 8) = htonl(MSG_RESP_CLIENT_ADDR);
						*(int*)(cl->buffer_invio + 12) = htonl(p->client_addr.sin_addr.s_addr);
						*(int*)(cl->buffer_invio + 16) = htonl(p->porta_udp);
						FD_SET(cl->id,&master_wr);
						
						p->buffer_invio = malloc(5 * sizeof(int));
						*(int*)p->buffer_invio = 4 * sizeof(int);
						*(int*)(p->buffer_invio + 4) = htonl(CMD_RESPONSE);
						*(int*)(p->buffer_invio + 8) = htonl(MSG_RESP_CLIENT_POS);
						*(int*)(p->buffer_invio + 12) = htonl(cl->client_addr.sin_addr.s_addr);
						*(int*)(p->buffer_invio + 16) = htonl(cl->porta_udp);
						FD_SET(p->id,&master_wr);
						
						printf("'%s' si e' connesso con '%s'\n",p->nome,cl->nome);
					}
					
					else
					{
						cl->libero = 1;
						p->libero = 1;						
						
						p->buffer_invio = malloc(3 * sizeof(int));
						*(int*)p->buffer_invio = 2 * sizeof(int);
						*(int*)(p->buffer_invio + 4) = htonl(CMD_RESPONSE);
						*(int*)(p->buffer_invio + 8) = htonl(MSG_RESP_CLIENT_NEG);
						FD_SET(p->id,&master_wr);
						printf("'%s' ha rifiutato la richiesta di '%s'\n",cl->nome,p->nome);
						printf("'%s' e' libero\n",cl->nome);
						printf("'%s' e' libero\n",p->nome);
					}
				}
			break;
			
			
		
		// il client 'cl' interrompe la partita in corso con il comando '!disconnect'
		case CMD_DISCONNECT:
				p = cerca(cl->id_altro_giocatore);
				cl->libero = 1;
				
				printf("'%s' si e' disconnesso da '%s'\n",cl->nome,p->nome);
				printf("'%s' e' libero\n",cl->nome);
				avverti(cl->id_altro_giocatore);
			break;
			
			
			
		// il client 'cl' ha finito la partita in corso con il client 'p'
		case CMD_FINE:
				p = cerca(cl->id_altro_giocatore);
				cl->libero = 1;
				p->libero = 1;
				
				p->buffer_invio = malloc(3 * sizeof(int));
				*(int*)p->buffer_invio = 2 * sizeof(int);
				*(int*)(p->buffer_invio + 4) = htonl(CMD_FINE);
				*(int*)(p->buffer_invio + 8) = *(int*)(buffer + 4);
				FD_SET(p->id,&master_wr);
				
				cmd = *(int*)(buffer + 4);
				if(cmd == CODICE_VITTORIA)
					printf("'%s' ha vinto la partita contro '%s'\n",cl->nome,p->nome);
				else
					printf("'%s' ha pareggiato la partita contro '%s'\n",cl->nome,p->nome);
				printf("'%s' e' libero\n",cl->nome);
				printf("'%s' e' libero\n",p->nome);
			break;
			
			
			
		// client inattivo
		case CMD_TIMER:
				cl->libero = 1;
				printf("'%s' e' inattivo\n",cl->nome);
				printf("'%s' e' libero\n",cl->nome);
			break;
			
			
		default:
			printf("Comando sconosciuto al server\n");
	}
}
