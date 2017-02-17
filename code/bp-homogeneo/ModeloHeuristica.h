#ifndef MODELOHR_H_
#define MODELOHR_H_

#include "ModeloCplex.h"
#include "NoArvore.h"

class ModeloHeuristica{
	private:
		IloModel model;
		IloCplex cplex;
		IloIntVarArray lambdaInt;
		IloIntVarArray tauInt;

	public:
		ModeloHeuristica(ModeloCplex&, ptrNo noAtual = NULL);
		~ModeloHeuristica();
		void buildModel(ModeloCplex&, ptrNo);
		float optimize();
};

#endif
