all : sample

sample: main.o mySimpleSim.o
	gcc main.o mySimpleSim.o -o sample

main.o: main.c
	gcc -c main.c
	
mySimpleSim.o: mySimpleSim.c
	gcc -c mySimpleSim.c

clean:
	rm *o sample
