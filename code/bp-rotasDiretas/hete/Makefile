OPT = main.o
GRAFO_OBJ = Grafo.o
ROTAS_OBJ = geraRotas.o Rota.o
ARVORE_OBJ = NoArvore.o ListaNosAtivos.o
CPLEX_OBJ = ModeloCplex.o ModeloHeuristica.o
SUBPROBLEMA_OBJ = Elementar.o ElementarRD.o ElementarRDH.o StateSpaceRelax.o ModeloBC-RD.o Fluxo.o dinic.o
ALL_OBJS = $(CPLEX_OBJ) $(ROTAS_OBJ) $(GRAFO_OBJ) $(SUBPROBLEMA_OBJ) $(ARVORE_OBJ) $(OPT)
CFLAGS = -Wall -W -g -DNOTE
LIBS = -lm -lpthread
CC = g++
CCC = gcc

SYSTEM = x86_sles10_4.1
CPLEX = cplex
CONCERT = concert
LIBFORMAT = static_pic
# System/lib-format specific compile/link flags
CSYSFLAGS  = -fPIC -fexceptions -DNDEBUG -DIL_STD 
LDSYSFLAGS = 
# Compilation and linking flags enabling Multi-threading
CMTFLAGS = -DILOUSEMT -D_REENTRANT
LDMTFLAGS = -lilocplex -lcplex -lpthread
# Code optimization/debugging options
DEBUG = -O1 -DNDEBUG
SUFFIX=cc
CONCERTDIR=/usr/ilog/$(CONCERT)
CPLEXDIR=/usr/ilog/$(CPLEX)/include
INCDIR=$(CONCERTDIR)/include
LIBDIR=$(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
# OPTIONS and LDOPTIONS provide a way for the user to add his own
# specific compilation and linking options.
CFLAGS_CPLEX = $(CSYSFLAGS) $(DEBUG) -I$(INCDIR) $(OPTIONS) -I$(CPLEXDIR) 
LDFLAGS = -L$(LIBDIR) $(LDSYSFLAGS) $(LDOPTIONS)
FLAGS_JOAO = -L/usr/ilog/$(CPLEX)/lib/$(SYSTEM)/static_pic -lilocplex -lcplex -L/usr/ilog/$(CONCERT)/lib/$(SYSTEM)/static_pic -lconcert

geraCols: $(ALL_OBJS)
	$(CC) $(CFLAGS) $(ALL_OBJS) $(LIBS) $(FLAGS_JOAO) -o geraColsRD.e

Grafo.o: Grafo.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Grafo.cpp

ModeloCplex.o: ModeloCplex.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloCplex.cpp

ModeloHeuristica.o: ModeloHeuristica.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloHeuristica.cpp

geraRotas.o: geraRotas.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g geraRotas.cpp

Rota.o: Rota.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Rota.cpp
	
NoArvore.o: NoArvore.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g NoArvore.cpp

ListaNosAtivos.o: ListaNosAtivos.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ListaNosAtivos.cpp

Elementar.o: Elementar.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Elementar.cpp

ElementarRD.o: ElementarRD.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ElementarRD.cpp

ElementarRDH.o: ElementarRDH.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ElementarRDH.cpp

StateSpaceRelax.o: StateSpaceRelax.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g StateSpaceRelax.cpp

ModeloBC-RD.o: ModeloBC-RD.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloBC-RD.cpp

Fluxo.o: Fluxo.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Fluxo.cpp

dinic.o: dinic.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g dinic.cpp

main.o: main.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm geraColsRD.e -f
