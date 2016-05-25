all: oxcows
	./$<

oxcows: main.cpp
	g++ -o $@ -Wall -Wextra -Werror -g $+