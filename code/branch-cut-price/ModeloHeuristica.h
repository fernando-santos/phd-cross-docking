#ifndef MODELOHR_H_
#define MODELOHR_H_

#include "ModeloCplex.h"

class ModeloHeuristica{
	private:
		IloModel model;
		IloCplex cplex;

		IloIntVarArray lambdaInt;
		IloIntVarArray gammaInt;
		IloIntVarArray tauInt;

		IloObjective objective;
		IloRange constraints1;
		IloRange constraints2;
		IloRangeArray constraints3;
		IloRangeArray constraints4;
		
	public:
		ModeloHeuristica(ModeloCplex&);
		~ModeloHeuristica();
		void initVars(ModeloCplex&);
		void setObjectiveFunction(ModeloCplex&);
		void setConstraints1e2(ModeloCplex&);
		void setConstraints3e4(ModeloCplex&);
		float optimize();
};

#endif
