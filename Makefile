Main: main.o 
	gcc main.o -o Main -lreadline

main.o: main.c
	gcc -c -g main.c -lreadline

clean:
	rm -f *.o Main
