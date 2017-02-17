OPT = main.o 
CPLEX_OBJ = ModeloCplexI.o ModeloCplexR.o
GRAFO_OBJ = Grafo.o
ERCSPP = XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
ALL_OBJS = $(CPLEX_OBJ) $(GRAFO_OBJ) $(OPT)
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

fluxos: $(ALL_OBJS)
	$(CC) $(CFLAGS) $(ALL_OBJS) $(LIBS) $(FLAGS_JOAO) -o fluxo.e

Grafo.o: Grafo.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Grafo.cpp

ModeloCplexI.o: ModeloCplexI.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloCplexI.cpp
	
ModeloCplexR.o: ModeloCplexR.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ModeloCplexR.cpp

main.o: main.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g main.cpp

temp:
	rm *.o -f
	rm *~ -f

clean:
	rm *.o -f
	rm *~ -f
	rm fluxo.e -f
