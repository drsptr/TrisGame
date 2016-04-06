#include "tris_server.h"

struct client* lista = NULL;
fd_set master_rd,master_wr,rdset,wrset;
int fdmax;
	
int main(int argc,char* argv[])
{			
	int ret,i,len;
	int optval = 1;

	void* buffer;
	struct client* app_client;

	

	int list_sock,conn_sock;
	struct sockaddr_in my_addr,app_addr;



	printf("Avvio del server..\n");
	if(!argv[1] || !argv[2])
	{
		printf("Indirizzo IP o porta del server non valida\n");
		exit(-1);
	}	
	
	list_sock = socket(AF_INET,SOCK_STREAM,0);
	if(list_sock == -1)
	{
		printf("Errore nella creazione del socket\n");
		exit(-1);
	}
  
  // settaggio opzioni del socket
  	ret = setsockopt(list_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(ret == -1)
	{
		printf("Errore nell'esecuzione della setsockopt\n");
		exit(-1);
	}
  
  // inizializzazione sockaddr_in my_addr
	if(atoi(argv[2])<1024 || atoi(argv[2])>65535)
	{
		printf("Porta del server non valida\n");
		printf("Il server verrà terminato\n");
		exit(-1);
	}
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(argv[2]));
	ret = inet_pton(AF_INET,argv[1],&my_addr.sin_addr.s_addr);
	if(ret == -1)
	{
		printf("IP del server non valido\n");
		printf("Il server verrà terminato\n");
		exit(-1);
	}
  
  // bind
	ret = bind(list_sock,(struct sockaddr*)&my_addr,sizeof(my_addr));
	if(ret == -1)
	{
		printf("Errore nell'esecuzione della bind()\n");
		exit(-1);
	}
	printf("Indirizzo: %s (porta %s)\n",argv[1],argv[2]);
  
  //azzeramento fd_set
  	FD_ZERO(&master_rd);
  	FD_ZERO(&master_wr);
  
  // listen
	ret = listen(list_sock,10);
	if(ret == -1)
	{
		printf("Errore nell'esecuzione della listen()\n");
		exit(-1);
	}
	
  // inizializzazione fd_set
	FD_SET(list_sock,&master_rd);
	fdmax = list_sock;
while(1)
{
	rdset = master_rd;
	wrset = master_wr;
	ret = select(fdmax+1,&rdset,&wrset,NULL,NULL);
	if(ret == -1)
	{
		printf("Errore nell'esecuzione della select\n");
  		exit(-1);
	}
	for( i=0; i<=fdmax; i++)
	{
		app_client = cerca(i);
		
		if(FD_ISSET(i,&rdset))
		{
			// listener socket
			if( i == list_sock)
			{
				len = sizeof(app_addr);
				conn_sock = accept(list_sock,(struct sockaddr*)&app_addr,(void*)&len);
				if(conn_sock == -1)
				{
					printf("Errore nell'esecuzione dell'accept\n");
					exit(-1);
				}
				aggiungi(conn_sock,app_addr);
				printf("Connessione stabilita con il client\n");
				FD_SET(conn_sock,&master_rd);
				fdmax = (conn_sock > fdmax)? conn_sock : fdmax;
			}
			
			// connected socket
			else
			{	
				if(app_client->recv == 0)
				{
					buffer = malloc(HEADER);
					ret = recv(i,buffer,HEADER,0);
					if(ret == -1)
					{
							printf("Errore nella ricezione della dimensione\n");
  	 						exit(-1);
					}
					if(ret == 0)
						app_client->chiudi = 1;
					
					if(app_client->chiudi != 1)
					{
						app_client->dim_op = ntohl(*(int*)buffer);
						app_client->recv =1;
						free(buffer);
					}
				}
				else
				{
					buffer = malloc(app_client->dim_op);
					ret = recv(i,buffer,app_client->dim_op,0);
					if(ret == -1)
					{
							printf("Errore nella ricezione dei dati\n");
  	 						app_client->chiudi = 1;
					}
					if(ret == 0)
						app_client->chiudi = 1;
					
					if(app_client->chiudi != 1)
					{
						app_client->recv = 0;
						gestisci_cmd(app_client,buffer);
						free(buffer);
					}
				}
				
				if(app_client->chiudi == 1)
				{
					if(app_client->libero==1 || (app_client->libero==0 && avverti(app_client->id_altro_giocatore)==0))
					{
						if(app_client->nome != NULL)
							printf("'%s' si e' disconnesso\n",app_client->nome);
						else
							printf("Il client %d si e' disconnesso\n",i);
						
						elimina(i);
						FD_CLR(i,&master_rd);
						FD_CLR(i,&master_wr);
						FD_CLR(i,&rdset);
						FD_CLR(i,&wrset);
						while(FD_ISSET(fdmax,&master_rd) == 0) fdmax--;
						close(i);
					}
				}
			}
		}
		
		
		if(FD_ISSET(i,&wrset))
		{
			if(app_client->send == 0)
			{
				ret = invia_header(i,*(int*)app_client->buffer_invio);
				if(ret == -1)
				{
					printf("Errore nell'invio della dimensione dei dati\n");
					exit(-1);
				}
				app_client->send = 1;
			}
			else
			{
				ret = send(i,(app_client->buffer_invio +4),*(int*)app_client->buffer_invio,0);
				if(ret == -1)
				{
					printf("Errore nell'invio dei dati al client\n");
					exit(-1);
				}
				app_client->send = 0;
				free(app_client->buffer_invio);
				app_client->buffer_invio = NULL;
				FD_CLR(i,&master_wr);
			}
		}
	}
}

	close(list_sock);
	return 0;
}
