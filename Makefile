all: bullscows

bullscows: main.o
	g++ -o $@ -Wall -Wextra -Werror -g $+

main.o: number.h