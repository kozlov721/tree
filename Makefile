# Project: simple clone of tree
# Author: Martin Kozlovský

FLAGS = -Wall -Wno-format -Werror -pedantic -Wextra -lm
NAME = tree

all:
	cc main.c tree.c -o $(NAME) $(FLAGS)
