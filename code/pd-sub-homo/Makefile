ElementarM2: Rota.o Grafo.o ElementarM2.o main.o
	g++ Rota.o Grafo.o ElementarM2.o main.o -o elementarM2.e
	rm *.o

Rota.o: Rota.cpp
	g++ -c -g Rota.cpp

Grafo.o: Grafo.cpp
	g++ -c -g Grafo.cpp

ElementarM2.o: ElementarM2.cpp
	g++ -c -g ElementarM2.cpp

main.o: main.cpp
	g++ -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm GRASP.e -f

