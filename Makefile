minish:	minish.o
	gcc minish.o -o minish

minish.o: minish.c
	gcc -c minish.c
	
clean:
	rm *.o minish
