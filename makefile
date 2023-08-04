web: web.o network.o file.o tool.o
	gcc -o web web.o network.o file.o tool.o

web.o: web.c network.h file.h tool.h
	gcc -c web.c

file.o: file.c file.h
	gcc -c file.c

network.o: network.c network.h
	gcc -c network.c


tool.o: tool.c tool.h
	gcc -c tool.c

