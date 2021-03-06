#ifndef MODELOHR_H_
#define MODELOHR_H_

#include "ModeloCplex.h"

class ModeloHeuristica{
	private:
		IloModel model;
		IloCplex cplex;

		IloArray<IloIntVarArray> lambdaInt;
		IloArray<IloIntVarArray> gammaInt;
		IloArray<IloArray<IloIntVarArray> > tauInt;

		IloObjective objective;
		IloRangeArray constraints1;
		IloRangeArray constraints2;
		IloRangeArray constraints3;
		IloRangeArray constraints4;
		IloRangeArray constraints5;
		
	public:
		ModeloHeuristica(ModeloCplex&);
		~ModeloHeuristica();
		void initVars(ModeloCplex&);
		void setObjectiveFunction(ModeloCplex&);
		void setConstraints1e2(ModeloCplex&);
		void setConstraints3e4(ModeloCplex&);
		void setConstraints5(ModeloCplex&);
		float optimize();
};

#endif
