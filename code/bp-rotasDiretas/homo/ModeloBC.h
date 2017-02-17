#ifndef MODELOBC_H_
#define MODELOBC_H_

#include "Grafo.h"
#include "Rota.h"
#include "Fluxo.h"

class ModeloBC{
	public:
		int nRequests;
		int nVertices;
		float valorSubtrair;
		IloEnv env;
		IloModel model;
		IloCplex cplex;
		IloArray <IloIntVarArray> x;
		IloIntVarArray y;
		IloIntVarArray t;
		vector<Rota*> rotasNegativas;

		~ModeloBC();
		ModeloBC(Grafo*, float*, float);
		void initVars();
		void setObjectiveFunction(Grafo*, float*);
		void setConstraints1e2(Grafo*);
		void setConstraints3e4(Grafo*);
		void setConstraints5e6(Grafo*);
		void setConstraints7();

		void calculaCaminhoElementar(Grafo*, int);
		Rota* getRotaCustoMinimo(Grafo*);
};

#endif
