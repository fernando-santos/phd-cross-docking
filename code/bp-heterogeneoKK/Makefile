OPT = main.o
GRAFO_OBJ = Grafo.o
ROTAS_OBJ = geraRotas.o Rota.o
CPLEX_OBJ = ModeloCplex.o ModeloHeuristica.o
SUBPROBLEMA_OBJ = Elementar.o
ALL_OBJS = $(CPLEX_OBJ) $(ROTAS_OBJ) $(GRAFO_OBJ) $(SUBPROBLEMA_OBJ) $(OPT)
CFLAGS = -Wall -W -g -DNOTE
LIBS = -lm -lpthread
CC = g++
CCC = gcc

SYSTEM     = x86-64_debian4.0_4.1
LIBFORMAT  = static_pic
CPLEXDIR      = /home/lapo/cplex/cplex121_0.x64linux/cplex121
CONCERTDIR    = /home/lapo/cplex/cplex121_0.x64linux/concert29
CCC = g++
CCOPT = -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD
CPLEXBINDIR   = $(CPLEXDIR)/bin/$(BINDIST)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -pthread
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 

geraColskk: $(ALL_OBJS)
	$(CCC) -Wall -W -g -DNOTE $(ALL_OBJS) -o geraColskk.e $(CCLNFLAGS)

Grafo.o: Grafo.cpp
	$(CCC) $(CCFLAGS) -c -g Grafo.cpp

ModeloCplex.o: ModeloCplex.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloCplex.cpp

geraRotas.o: geraRotas.cpp
	$(CCC) $(CCFLAGS) -c -g geraRotas.cpp

Rota.o: Rota.cpp
	$(CCC) $(CCFLAGS) -c -g Rota.cpp

Elementar.o: Elementar.cpp
	$(CCC) $(CCFLAGS) -c -g Elementar.cpp

ModeloHeuristica.o: ModeloHeuristica.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloHeuristica.cpp
 
main.o: main.cpp
	$(CCC) $(CCFLAGS) -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm geraColskk.e -f
