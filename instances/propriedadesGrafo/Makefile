all: main

main: main.o Grafo.o
	g++ main.o Grafo.o -o propGrafo.e
	rm *.o -f
	rm *~ -f

main.o: main.cpp
	g++ -c main.cpp

Grafo.o: Grafo.cpp
	g++ -c -g Grafo.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm propGrafo.e -f
