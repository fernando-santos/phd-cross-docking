OPT = main.o
GRAFO_OBJ = Grafo.o
ROTAS_OBJ = Rota.o
CPLEX_OBJ = ModeloCplex.o ModeloHeuristica.o
ARVORE_OBJ = NoArvore.o NoArvoreArcos.o ListaNosAtivos.o
SUBPROBLEMA_OBJ = ElementarTW.o
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

geraColsTW: $(ALL_OBJS)
	$(CC) $(CFLAGS) $(ALL_OBJS) $(LIBS) $(FLAGS_JOAO) -o geraColsTW.e
	rm *.o

Grafo.o: Grafo.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Grafo.cpp

ModeloCplex.o: ModeloCplex.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloCplex.cpp

Rota.o: Rota.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Rota.cpp

ElementarTW.o: ElementarTW.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ElementarTW.cpp

NoArvore.o: NoArvore.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g NoArvore.cpp

NoArvoreArcos.o: NoArvoreArcos.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g NoArvoreArcos.cpp

ListaNosAtivos.o: ListaNosAtivos.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ListaNosAtivos.cpp

ModeloHeuristica.o: ModeloHeuristica.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloHeuristica.cpp
 
main.o: main.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm geraColsTW.e -f
