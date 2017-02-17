#ifndef MODELOBC_H_
#define MODELOBC_H_

#include "Grafo.h"
#include "Rota.h"
#include "Fluxo.h"

class ModeloBC{
	public:
		int veic;
		int nRequests;
		int nVertices;
		float valorSubtrair;
		IloEnv env;
		IloModel model;
		IloCplex cplex;
		IloArray <IloIntVarArray> x;
		IloIntVarArray y;
		vector<Rota*> rotasNegativas;

		~ModeloBC();
		ModeloBC(Grafo*, int, float);
		void initVars();
		void setObjectiveFunction(Grafo*);
		void setConstraints1e2(Grafo*);
		void setConstraints3e4(Grafo*);
		void setConstraints6e7(Grafo*);

		void calculaCaminhoElementar(Grafo*, int);
		Rota* getRotaCustoMinimo(Grafo*);
};

#endif
