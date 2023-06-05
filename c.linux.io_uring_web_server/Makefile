all:
	gcc -g3 main.c -luring

bin:
	gcc -Ofast -flto -march=native -o server main.c -luring

clang:
	clang -g3 main.c -luring
