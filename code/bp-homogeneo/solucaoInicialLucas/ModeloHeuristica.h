#ifndef MODELOHR_H_
#define MODELOHR_H_

#include "ModeloCplex.h"

class ModeloHeuristica{
	private:
		IloModel model;
		IloCplex cplex;
		IloIntVarArray lambdaInt;
		IloIntVarArray tauInt;

	public:
		~ModeloHeuristica();
		ModeloHeuristica(ModeloCplex&);
		void buildModel(ModeloCplex&);
		void imprimeRotasOtimas(ModeloCplex&);
		float optimize();
};

#endif
