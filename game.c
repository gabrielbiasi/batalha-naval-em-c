/*
UEMS - Universidade Estadual de Mato Grosso do Sul
Bacharelado em Ciência da Computação
Sistemas Operacionais
Trabalho RPC - Cliente

Feito por Gabriel de Biasi, RGM 024785.

*/

#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/*
Definição de UNICODES para desenhar o tabuleiro.
*/
#define AGUA "\u2591\u2591 "
#define AGUA_VISITADA "\u2593\u2593 "
#define DESTRUIDO "\u2691\u2691 "

#define SUBMARINO "\u25C1\u25B7 "
#define ENCORACADO "\u25FB\u25FB "
#define CRUZADOR "\u25FC\u25FC "
#define PORTAAVIOES "\u25E7\u25E8 "


/*
FUNÇÃO Pos2Cod(char* str)
Esta função retorna o índice no vetor de acordo com a posição inserida pelo cliente.
Ex: A1 -> 11
*/
int Pos2Cod(char* str) {
	return ((str[0]-65)*10)+(str[1]-48);
}

/*
FUNÇÃO ValidaPos(char* str)
Esta função valida uma entrada de posição de tabuleiro feita pelo cliente.
Ex: H3 -> 1 | K2 -> 0
*/
int ValidaPos(char* str) {
	if(str[0] >= 'A' && str[0] <= 'J' && str[1] >= '0' && str[1] <= '9' && str[2] == '\0') return 1;
	else return 0;
}

/*
FUNÇÃO Cod2Pos(int pos, char* str)
Esta função retorna uma string que representa a posição no tabuleiro de acordo com um determinado índice.
Ex: 11 -> A1
*/
char* Cod2Pos(int pos, char* str) {
	str[0] = ((pos/10)+65);
	str[1] = (pos%10)+48;
	str[2] = '\0';
	return str;
}

/*
PROCEDIMENTO printUNI(char code)
De acordo com o caracter representado, imprime o caracter UNICODE para desenhar o tabuleiro.
*/
void printUNI(char code) {
	switch(code) {
		case 0:
			printf(AGUA);
		break;
		case 1:
			printf(AGUA_VISITADA);
		break;
		case 2:
			printf(DESTRUIDO);
		break;
		case 3:
			printf(SUBMARINO);
		break;
		case 4:
			printf(ENCORACADO);
		break;
		case 5:
			printf(CRUZADOR);
		break;
		case 6:
			printf(PORTAAVIOES);
		break;
		default:
			printf("? ");
		break;
	}
}

/*
PROCEDIMENTO PrintGame(char* game1, char* game2, int vez)
Este procedimento limpa a tela, imprime um cabeçalho no topo e desenha os tabuleiros na tela.
*/
void PrintGame(char* game1, char* game2, int vez) {
	int i, j;
	printf("\33[H\33[2J");
	printf("\t\t\t===== BATALHA NAVAL ===== \n\t\t\t     == Jogador %d == ", vez);
	printf("\n\t     VOCE\t\t\t\t   ADVERSARIO\n  ");
	for(j = 0; j < 10; j++) printf("%d  ", j); /* 0, 1, 2, 3, ..., 9. */
	printf("\t  ");
	for(j = 0; j < 10; j++) printf("%d  ", j); /* 0, 1, 2, 3, ..., 9. */
	putchar('\n');
	for(i = 0; i < 10; i++) {
		printf("%c ", 65+i); /* A, B, C, D, ..., J. */
		for(j = i*10; j < (i*10)+10; j++) {;
			printUNI(game1[j]);
		}
		putchar('\t');
		printf("%c ", 65+i); /* A, B, C, D, ..., J. */
		for(j = i*10; j < (i*10)+10; j++) {;
			printUNI(game2[j]);
		}
		printf("\n\n");
	}
	//printf("\n%s- AGUA\t%s- VISITADO\t%s- SUBMARINO\t%s- ENCORACADO\n\n%s- CRUZADOR\t%s- PORTA-AVIOES\t%s- DESTRUIDO \n\n", AGUA, AGUA_VISITADA, SUBMARINO, ENCORACADO, CRUZADOR, PORTAAVIOES, DESTRUIDO);
}


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
	if(ret == 0) {
		printf("O Servidor foi fechado!\n\n");
		close(socket);
		exit(1);
	}
	return ret;
}

/*
ROTINA PRINCIPAL
*/
int main(int argc, char* argv[]) {
	int i, ok, sock, id, pos, dir, count, quit = 0, vez = 0;
	struct sockaddr_in server;
	char h[3], mygame[100], adgame[100];
	char nomes[4][15] = {"SUBMARINO", "ENCORACADO", "CRUZADOR", "PORTA-AVIOES"};
	int tam[4] = {1, 2, 3, 5};

	memset(mygame, 0, 100);
	memset(adgame, 0, 100);

	/* Verifica se os dados do servidor foram passados por argumento de entrada. */
	if(argc != 3) {
		printf("\n[Game Client] Erro! Inicie o programa com argumentos: [ip] [porta].\n\n");
		exit(1);
	}
	
	/* Criação do socket. */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("\n[Game Client] Erro no Socket.\n\n");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]); /* Pega o ip do servidor pelo argumento de entrada. */
	server.sin_port = htons(atoi(argv[2])); /* Mesma coisa com a porta. */

	/* Tenta fazer a conexão. */
	if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
		printf("\n[Game Client] Erro na conexao.\n\n");
		exit(1);
	}

	/* Recebe do servidor, qual jogador este cliente será. */
	printf("\n[Game Client] Conectado! IP: [%s] Porta: [%s]\n", argv[1], argv[2]);

	while(!quit) {
		recebe(sock, &id, sizeof(id));
		switch(id) {
			case 0: /* Caso para fechar o cliente. */
				quit = 0;
			break;

			case 1: /* Primeiro contato do servidor, informando se você é o Jogador 1 ou 2. */
			case 2:
				vez = id;
				if(vez == 1) {
					printf("[Game Client] Voce eh o Jogador 1! Aguarde a conexao de um novo jogador.\n");
				} else {
					printf("[Game Client] Voce eh o Jogador 2! Aguarde o Jogador 1 enviar seu tabuleiro.\n");
				}
			break;

			case 3: /* Entrada do Tabuleiro para enviar ao servidor. */
				for(i = 0; i < 4; i++) {
					ok = 0;
					do {
						PrintGame(mygame, adgame, vez);
						printf("[Game Client] Entrada do Tabuleiro\n\n");
						if(ok) printf("Local Invalido!\n\n");
						printf("Entre com o local para o %s, que possui o tamanho de %d unidade(s). (Ex: H3)\nOpcao: ", nomes[i], tam[i]);
						__fpurge(stdin);
						fgets(h, 3, stdin);
						printf("\nEm qual direcao? (0 - Vertical | 1 - Horizontal)\nOpcao: ");
						scanf("%d", &dir);
						if((dir == 0 || dir == 1) && ValidaPos(h)) {
							pos = Pos2Cod(h);
							count = 0;
							if(dir == 0) { /* Validar navio na vertical. */
								while(mygame[pos+(count*10)] == 0 && count < tam[i]) {
									count++;
									if(pos+(count*10) > 99 && count < tam[i]) break; /* Caso chegue no final do tabuleiro por baixo. */
								}
							} else { /* Validar navio na horizontal. */
								while(mygame[pos+count] == 0 && count < tam[i]) {
									count++;
									if(count != 0 && (pos+count)%10 == 0) break; /* Caso chegue no final do tauleiro pela direita. */
								}
							}
							ok = (count != tam[i]) ? 1:0;
						} else {
							ok = 1;
						}
					} while(ok);

					/* Escreve o navio no tabuleiro. */
					count = 0;
					if(dir == 0) { /* Escreve na vertical. */
						while(count < tam[i]) {
							mygame[pos+(count*10)] = i+3;
							count++;
						}
					} else { /* Escreve na horinzontal. */
						while(count < tam[i]) {
							mygame[pos+count] = i+3;
							count++;
						}
					}
				}
				/* Envia para o servidor o tabuleiro pronto. */
				envia(sock, &mygame, sizeof(mygame));
				PrintGame(mygame, adgame, vez);
				if(vez == 1) {
					printf("[Game Client] Fim da entrada.\n[Game Client] Aguarde o Jogador 2 realizar a entrada de seu tabuleiro.\n");
				} else {
					printf("[Game Client] Fim da entrada.\n");
				}
			break;

			case 4: /* Vez de jogada. */
				ok = 0;
				do {
					PrintGame(mygame, adgame, vez);
					printf("[Game Client] SUA VEZ!\n");
					if(ok) printf("\nLocal Invalido!\n");
					printf("\nEntre com o local para bombardear. (Ex: H3)\nOpcao: ");
					__fpurge(stdin);
					fgets(h, 3, stdin);
					if(ValidaPos(h)) {
						ok = (adgame[Pos2Cod(h)] != 0) ? 1:0;
					} else {
						ok = 1;
					}
				} while(ok);
				pos = Pos2Cod(h);
				envia(sock, &pos, sizeof(pos)); /* Envia a jogada para o servidor. */
			break;

			case 5: /* Recebe a resposta do ataque. */
				recebe(sock, &dir, sizeof(dir)); /* Recebe resposta do servidor. */

				if(dir >= 3 && dir <= 6) { /* Destruiu totalmente algo! */
					adgame[pos] = 2;
				} else if(dir == 2) { /* Destruiu parte de um navio! */
					adgame[pos] = 2;
				} else {
					adgame[pos] = 1; /* Água visitada! */
				}

				PrintGame(mygame, adgame, vez);

				if(dir >= 3 && dir <= 6) {
					printf("[Game Client] Voce destruiu totalmente o %s do inimigo!!!\n", nomes[dir-3]);
				} else if(dir == 2) {
					printf("[Game Client] Voce destruiu algo!!!\n");
				} else {
					printf("[Game Client] Agua!!! \n");
				}
				sleep(2);
			break;

			case 6: /* Aguarda ataque do inimigo. */
				PrintGame(mygame, adgame, vez);
				printf("[Game Client] Aguardando jogada do inimigo...\n\n");

				recebe(sock, &dir, sizeof(dir));
				recebe(sock, &pos, sizeof(pos));

				if(dir >= 3 && dir <= 6) { /* Destruiu totalmente algo! */
					mygame[pos] = 2;
				} else if(dir == 2) { /* Destruiu parte de um navio! */
					mygame[pos] = 2;
				} else {
					mygame[pos] = 1; /* Água visitada! */
				}
				
				PrintGame(mygame, adgame, vez);
				Cod2Pos(pos, h);
				printf("[Game Client] Inimigo atacou o local %s!\n", h);
				if(dir >= 3 && dir <= 6) {
					printf("[Game Client] O inimigo destruiu totalmente seu %s!!!\n", nomes[dir-3]);
				} else if(dir == 2) {
					printf("[Game Client] Seu inimigo destruiu algo!!!\n");
				} else {
					printf("[Game Client] Agua!!! \n");
				}
				sleep(2);
			break;

			case 7: /* Fim de Jogo!! */
			case 8:
				recebe(sock, &adgame, sizeof(adgame)); /* Recebe o tabuleiro do inimigo para mostrar no fim. */
				PrintGame(mygame, adgame, vez);
				if(id == 7) {
					printf("[Game Client] VOCE VENCEU!!! \n\n");
				} else {
					printf("[Game Client] VOCE PERDEU! :(\n\n");
				}
				quit = 1;
			break;
		}
	}
	close(sock);
	return 0;
}
