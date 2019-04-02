# lc3-vm: main.o operations.o utils.o
	# gcc -o lc3-vm main.o operations.o utils.o

lc3-vm: main.c operations.c utils.c
	gcc -o lc3-vm main.c operations.c utils.c

# main.o: main.c
# 	gcc -c main.c

# operations.o: operations.c
# 	gcc -c operations.c

# utils.o: utils.c
# 	gcc -c utils.c
