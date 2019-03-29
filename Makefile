batalhanaval: game.c server.c
	gcc -o game game.c -Wall -Wextra -pedantic -std=c99
	gcc -o server server.c -Wall -Wextra -pedantic
	
clean:
	rm game *.o *~
	rm server *.o *~
