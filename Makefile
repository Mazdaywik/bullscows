all: bullscows

bullscows: main.cpp
	g++ -o $@ -Wall -Wextra -Werror -g $+