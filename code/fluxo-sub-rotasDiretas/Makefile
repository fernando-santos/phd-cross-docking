ALL_OBJS = main.o Rota.o Grafo.o ElementarCplex.o
CFLAGS = -Wall -W -g -DNOTE
LIBS = -lm -lpthread
CC = g++
CCC = gcc

#---------------------- CPLEX12 LAPO ---------------------------
#SYSTEM = x86-64_debian4.0_4.1
#CPLEX = cplex121
#CONCERT = concert29
#CONCERTDIR=/home/lapo/cplex/cplex121_0.x64linux/$(CONCERT)
#CPLEXDIR=/home/lapo/cplex/cplex121_0.x64linux/$(CPLEX)
#INCDIR=$(CONCERTDIR)/include
#LIBDIR=$(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
#---------------------------------------------------------------

#---------------------- CPLEX12 PC -----------------------------
SYSTEM = x86_sles10_4.1
CPLEX = cplex
CONCERT = concert
CONCERTDIR=/usr/ilog/$(CONCERT)
CPLEXDIR=/usr/ilog/$(CPLEX)/include
INCDIR=$(CONCERTDIR)/include
LIBDIR=$(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
#---------------------------------------------------------------

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

# OPTIONS and LDOPTIONS provide a way for the user to add his own
# specific compilation and linking options.
CFLAGS_CPLEX = $(CSYSFLAGS) $(DEBUG) -I$(INCDIR) $(OPTIONS) -I$(CPLEXDIR) 
LDFLAGS = -L$(LIBDIR) $(LDSYSFLAGS) $(LDOPTIONS)
FLAGS_JOAO = -L/usr/ilog/$(CPLEX)/lib/$(SYSTEM)/static_pic -lilocplex -lcplex -L/usr/ilog/$(CONCERT)/lib/$(SYSTEM)/static_pic -lconcert

elemCplex: $(ALL_OBJS)
	$(CC) $(CFLAGS) $(ALL_OBJS) $(LIBS) $(FLAGS_JOAO) -o elemCplex.e
	rm *.o

Grafo.o: Grafo.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Grafo.cpp

ElementarCplex.o: ElementarCplex.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g ElementarCplex.cpp

Rota.o: Rota.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g Rota.cpp

main.o: main.cpp
	$(CC) $(CFLAGS_CPLEX) -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm elemCplex.e -f
