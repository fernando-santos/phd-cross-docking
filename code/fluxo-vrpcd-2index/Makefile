ALL_OBJS = ModeloCplexI.o ModeloCplexR.o Grafo.o main.o

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

fluxos: $(ALL_OBJS)
	$(CCC) -Wall -W -g -DNOTE $(ALL_OBJS) -o fluxo.e $(CCLNFLAGS)

Grafo.o: Grafo.cpp
	$(CCC) $(CCFLAGS) -c -g Grafo.cpp

ModeloCplexI.o: ModeloCplexI.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloCplexI.cpp
	
ModeloCplexR.o: ModeloCplexR.cpp
	$(CCC) $(CCFLAGS) -c -g ModeloCplexR.cpp

main.o: main.cpp
	$(CCC) $(CCFLAGS) -c -g main.cpp

clean:
	rm *.o -f
	rm *~ -f
	rm fluxo.e -f
