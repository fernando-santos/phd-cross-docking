OPT = main.o
GRAFO_OBJ = Grafo.o Rota.o
SUBPROBLEMA_OBJ = ESPPRCbi.o GRASP.o
ALL_OBJS = $(GRAFO_OBJ) $(SUBPROBLEMA_OBJ) $(OPT)


testeTTT: $(ALL_OBJS)
	g++ -Wall -W $(ALL_OBJS) -o testeTTT.e

Rota.o: Rota.cpp
	g++  -c -g Rota.cpp

Grafo.o: Grafo.cpp
	g++  -c -g Grafo.cpp

ESPPRCbi.o: ESPPRCbi.cpp
	g++  -c -g ESPPRCbi.cpp

GRASP.o: GRASP.cpp
	g++  -c -g GRASP.cpp
 
main.o: main.cpp
	g++  -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm testeTTT.e -f
