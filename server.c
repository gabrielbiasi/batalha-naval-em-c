/*
UEMS - Universidade Estadual de Mato Grosso do Sul
Bacharelado em Ciência da Computação
Sistemas Operacionais
Trabalho RPC - Servidor

Feito por Gabriel de Biasi, RGM 024785.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int socket_desc, sock1, sock2;

/*
FUNÇÃO envia(int socket, void* dados, size_t tam)
Esta função é para evitar a fadiga de testar o retorno da função write toda hora.
*/
int envia(int socket, void* dados, size_t tam) {
	int ret;
	if((ret = write(socket, dados, tam)) < 0) {
		perror("Erro no Write!");
	} 
	return ret;
}

/*
FUNÇÃO recebe(int socket, void* dados, size_t tam)
Mesmo caso da função envia, para testar o retorno da função.
*/
int recebe(int socket, void* dados, size_t tam) {
	int ret;
	if((ret = read(socket, dados, tam)) < 0) {
		perror("Erro no Read!");
	}
	if(!ret){
		printf("Alguem saiu!\n");
		close(sock1);
		close(sock2);
		close(socket_desc);
		exit(1);
	}
	return ret;
}

/*
FUNÇÃO Zerado(int* vet)
Esta função verifica se o jogo acabou.
*/
int Zerado(int* vet) {
	int i;
	for(i = 0; i < 4; i++) {
		if(vet[i] > 0) return 0;
	}
	return 1;
}

/*
ROTINA PRINCIPAL
*/
int main(int argc, char *argv[]) {
	char *game_vez, game1[100], game2[100];
	int c, id, tam, socket_vez, socket_ad, *tam_vez, pos;
	struct sockaddr_in server, cliente1, cliente2;
	int tam1[4] = {1, 2, 3, 5};
	int tam2[4] = {1, 2, 3, 5};
	
	if(argc != 2) { /* Validação de entrada com argumentos. */
		printf("\n[Game Server] Erro! Inicie o programa com o argumento: [porta].\n\n");
		exit(1);
	} else {
		printf("[Game Server] Iniciado!");
	}

	socket_desc = socket(AF_INET, SOCK_STREAM, 0); /* Criação do socket principal. */
	if (socket_desc == -1) {
		printf("\n[Game Server] Erro no Socket.\n");
		exit(1);
	}
	 
	/* Atribui as variáveis para a estrutura do servidor. */ 
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1])); /* Captura a porta pelo argumento de entrada. */
	
	/* Faz a ligação da porta com o socket. (Bind) */
	if(bind(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
		printf("\n[Game Server] Erro no Bind.\n");
		exit(1);
	}
	 
	/* Coloca o servidor para ouvir o canal de comunicação. */
	listen(socket_desc, 2);
	 
	c = sizeof(struct sockaddr_in);


	printf("\n[Game Server] Esperando pelos jogadores...\n\n");	

	/* Área de conexão dos jogadores. */

	/* Avisa ao cliente que ele é o jogador 1 e deve aguardar pela conexão do jogador 2. */
	sock1 = accept(socket_desc, (struct sockaddr *)&cliente1, (socklen_t*)&c);
	printf("Jogador [1]: Conectado!\n");
	id = 1;
	envia(sock1, &id, sizeof(id));

	/* Avisa ao cliente que ele é o jogador 2 e que ele deve esperar o jogador 1 entrar com o tabuleiro. */
	sock2 = accept(socket_desc, (struct sockaddr *)&cliente2, (socklen_t*)&c);
	printf("Jogador [2]: Conectado!\n");
	id = 2;
	envia(sock2, &id, sizeof(id));

	close(socket_desc); /* Fecha o socket do servidor. */

	/* Avisa o jogador 1 para mandar seu tabuleiro. */
	id = 3;
	envia(sock1, &id, sizeof(id));

	/* Tabuleiro do Jogador 1. */
	printf("Jogador [1]: Aguardando pelo tabuleiro...\n");
	recebe(sock1, &game1, sizeof(game1));
	printf("Jogador [1]: Tabuleiro recebido.\n");

	/* Avisa o jogador 2 para mandar seu tabuleiro. */
	id = 3;
	envia(sock2, &id, sizeof(id));

	/* Tabuleiro do Jogador 2. */
	printf("Jogador [2]: Aguardando pelo tabuleiro...\n");
	recebe(sock2, &game2, sizeof(game2));
	printf("Jogador [2]: Tabuleiro recebido.\n");

	/* Inicia o jogo para o Jogador 1. */
	id = 4;
	envia(sock1, &id, sizeof(id));

	/* Coloca o Jogador 2 na espera. */
	id = 6;
	envia(sock2, &id, sizeof(id));


	socket_vez = sock1;
	socket_ad = sock2;
	game_vez = game2;
	tam_vez = tam2;

	printf("\n[Game Server] Inicio do Jogo!\n");
	/* Inicio do jogo! */
	while(1) {
		recebe(socket_vez, &id, sizeof(id)); /* Recebe a jogada. */
		pos = id; /* Guarda a posição da jogada. */

		id = 5; /* Envia o código 5, que é a resposta de seu ataque. */
		envia(socket_vez, &id, sizeof(id));

		if(game_vez[pos] == 0) {
			game_vez[pos] = 1;
			id = 1; /* AGUA!! */
		} else {
			tam = --tam_vez[game_vez[pos]-3];
			if(tam == 0) {
				id = game_vez[pos]; /* Destruição total de um navio. */
			} else {
				id = 2; /* Destruição parcial de um navio. */
			}
			game_vez[pos] = 2; /* Marca como bloco destruído. */
		}

		envia(socket_vez, &id, sizeof(id)); /* Envia para o jogador atual a situação de seu ataque. */

		envia(socket_ad, &id, sizeof(id)); /* Envia situação para o inimigo. */
		envia(socket_ad, &pos, sizeof(pos)); /* Envia a posição de ataque para o inimigo. */

		/* Acabou o jogo com esta jogada? */
		if(Zerado(tam_vez)) {
			/* Acabou o jogo! */
			id = 7;
			envia(socket_vez, &id, sizeof(id));

			id = 8;
			envia(socket_ad, &id, sizeof(id));

			/* Envia o tabuleiro de um inimigo para o outro. */
			envia(sock1, &game2, sizeof(game2));
			envia(sock2, &game1, sizeof(game1));

			break;
		} else { 
			id = 6; /* Coloca o jogador atual em espera. */
			envia(socket_vez, &id, sizeof(id));

			id = 4; /* Faz o próximo jogador enviar sua jogada. */
			envia(socket_ad, &id, sizeof(id)); 
		}

		/* Muda a vez do jogador. */
		socket_vez = (socket_vez == sock1) ? sock2:sock1;
		socket_ad = (socket_ad == sock1) ? sock2:sock1;
		game_vez = (game_vez == game1) ? game2:game1;
		tam_vez = (tam_vez == tam1) ? tam2:tam1;
	}
	close(sock1);
	close(sock2);
	printf("[Game Server] Fim de Jogo!\n");
	return 0;
}
