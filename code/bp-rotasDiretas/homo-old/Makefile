OPT = geraRotas.o main.o
GRAFO_OBJ = Grafo.o Rota.o
ARVORE_OBJ = NoArvore.o ListaNosAtivos.o
CPLEX_OBJ = ModeloCplex.o ModeloHeuristica.o
SUBPROBLEMA_OBJ = DSSRPDP.o ESPPRCbi.o ElementarRD.o GRASP.o GRASPRD.o ModeloBC.o ModeloBC-RD.o Fluxo.o dinic.o
ALL_OBJS = $(CPLEX_OBJ) $(GRAFO_OBJ) $(SUBPROBLEMA_OBJ) $(ARVORE_OBJ) $(OPT)

SYSTEM     = x86_sles10_4.1
LIBFORMAT  = static_pic
CPLEXDIR      = /usr/ilog/cplex
CONCERTDIR    = /usr/ilog/concert
CCC = g++
CCOPT = -O -fPIC -fexceptions -DNDEBUG -DIL_STD
CPLEXBINDIR   = $(CPLEXDIR)/bin/$(BINDIST)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -pthread
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 

geraColsHomo: $(ALL_OBJS)
	$(CCC) -Wall -W -g -DNOTE $(ALL_OBJS) -o geraColsHomoRD.e $(CCLNFLAGS)

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

ElementarRD.o: ElementarRD.cpp
	$(CCC) $(CCFLAGS) -c -g ElementarRD.cpp

DSSRPDP.o: DSSRPDP.cpp
	$(CCC) $(CCFLAGS) -c -g DSSRPDP.cpp

ESPPRCbi.o: ESPPRCbi.cpp
	$(CCC) $(CCFLAGS) -c -g ESPPRCbi.cpp

GRASP.o: GRASP.cpp
	$(CCC) $(CCFLAGS) -c -g GRASP.cpp

GRASPRD.o: GRASPRD.cpp
	$(CCC) $(CCFLAGS) -c -g GRASPRD.cpp

ModeloBC.o: ModeloBC.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloBC.cpp

ModeloBC-RD.o: ModeloBC-RD.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloBC-RD.cpp

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
	rm geraColsHomoRD.e -f
