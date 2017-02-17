OPT = geraRotas.o main.o
GRAFO_OBJ = Grafo.o Rota.o
CPLEX_OBJ = ModeloCplex.o ModeloHeuristica.o
ARVORE_OBJ = NoArvore.o ListaNosAtivos.o
SUBPROBLEMA_OBJ = GRASP.o ESPPRC.o ModeloBC.o Fluxo.o dinic.o
ALL_OBJS = $(CPLEX_OBJ) $(GRAFO_OBJ) $(SUBPROBLEMA_OBJ) $(ARVORE_OBJ) $(OPT)

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

geraColsHomo: $(ALL_OBJS)
	$(CCC) -Wall -W -g -DNOTE $(ALL_OBJS) -o geraColsHomo.e $(CCLNFLAGS)

Rota.o: Rota.cpp
	$(CCC) $(CCFLAGS) -c -g Rota.cpp

geraRotas.o: geraRotas.cpp
	$(CCC) $(CCFLAGS) -c -g geraRotas.cpp

Grafo.o: Grafo.cpp
	$(CCC) $(CCFLAGS) -c -g Grafo.cpp

ModeloCplex.o: ModeloCplex.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloCplex.cpp

ModeloHeuristica.o: ModeloHeuristica.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloHeuristica.cpp

GRASP.o: GRASP.cpp
	$(CCC) $(CCFLAGS) -c -g GRASP.cpp

ESPPRC.o: ESPPRC.cpp
	$(CCC) $(CCFLAGS) -c -g ESPPRC.cpp

ModeloBC.o: ModeloBC.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloBC.cpp

Fluxo.o: Fluxo.cpp
	$(CCC) $(CCFLAGS) -c -g Fluxo.cpp

dinic.o: dinic.cpp
	$(CCC) $(CCFLAGS) -c -g dinic.cpp

NoArvore.o: NoArvore.cpp
	$(CCC) $(CCFLAGS) -c -g NoArvore.cpp

ListaNosAtivos.o: ListaNosAtivos.cpp
	$(CCC) $(CCFLAGS) -c -g ListaNosAtivos.cpp
 
main.o: main.cpp
	$(CCC) $(CCFLAGS) -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm geraColsHomo.e -f
