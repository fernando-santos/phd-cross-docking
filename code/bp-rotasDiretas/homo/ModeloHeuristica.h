#ifndef MODELOHR_H_
#define MODELOHR_H_

#include "ModeloCplex.h"

class ModeloHeuristica{
	private:
		IloModel model;
		IloCplex cplex;
		IloIntVarArray lambdaInt;
		IloIntVarArray deltaInt;
		IloIntVarArray tauInt;

	public:
		ModeloHeuristica(ModeloCplex&);
		~ModeloHeuristica();
		void buildModel(ModeloCplex&);
		float optimize(ModeloCplex&);
};

#endif
