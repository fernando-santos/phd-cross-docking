OPT = main.o
GRAFO_OBJ = Grafo.o
ROTAS_OBJ = geraRotas.o Rota.o
CPLEX_OBJ = ModeloCplex.o ModeloHeuristica.o
SUBPROBLEMA_OBJ = Elementar.o StateSpaceRelax.o
ALL_OBJS = $(CPLEX_OBJ) $(ROTAS_OBJ) $(GRAFO_OBJ) $(SUBPROBLEMA_OBJ) $(OPT)
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

geraColskk: $(ALL_OBJS)
	$(CC) $(CFLAGS) $(ALL_OBJS) $(LIBS) $(FLAGS_JOAO) -o geraColskk.e
	rm *.o

Grafo.o: Grafo.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Grafo.cpp

ModeloCplex.o: ModeloCplex.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloCplex.cpp

geraRotas.o: geraRotas.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g geraRotas.cpp

Rota.o: Rota.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Rota.cpp

Elementar.o: Elementar.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Elementar.cpp

StateSpaceRelax.o: StateSpaceRelax.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g StateSpaceRelax.cpp

ModeloHeuristica.o: ModeloHeuristica.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloHeuristica.cpp
 
main.o: main.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm geraColskk.e -f
