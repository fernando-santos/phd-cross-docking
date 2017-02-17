#ifndef ELEMENTARCPLEX_H_
#define ELEMENTARCPLEX_H_

#include <ilcplex/ilocplex.h>
#include "Grafo.h"
#include "Rota.h"

class ElementarCplex{
	private:
		int nRequests;
		int nVertices;
		IloEnv env;
		IloModel model;
		IloCplex cplex;
		IloArray <IloNumVarArray> f;
		IloArray <IloIntVarArray> x;
		IloIntVarArray y;

	public:
		~ElementarCplex();
		ElementarCplex(Grafo*);
		void initVars();
		void setObjectiveFunction(Grafo*);		
		void setConstraints1(Grafo*);
		void setConstraints2(Grafo*);
		void setConstraints3(Grafo*);
		void setConstraints4(Grafo*);
		void setConstraints5(Grafo*);
		void setConstraints6(Grafo*);
		void setConstraints7(Grafo*);
		void setConstraints8();

		void calculaCaminhoElementar();
		Rota* getRotaCustoMinimo(Grafo*);
};

#endif
