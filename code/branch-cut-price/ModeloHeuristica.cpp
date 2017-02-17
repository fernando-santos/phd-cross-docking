#include "ModeloHeuristica.h"

ModeloHeuristica::~ModeloHeuristica(){
	objective.end();
	constraints1.end();
	constraints2.end();
	constraints3.end();
	constraints4.end();
	lambdaInt.end();
	gammaInt.end();
	tauInt.end();
	cplex.end();
	model.end();
}

ModeloHeuristica::ModeloHeuristica(ModeloCplex& mCplex){
	model = IloModel(mCplex.env);
	cplex = IloCplex(model);
	cplex.setOut(mCplex.env.getNullStream());

	initVars(mCplex);
	setObjectiveFunction(mCplex);
	setConstraints1e2(mCplex);
	setConstraints3e4(mCplex);
}

void ModeloHeuristica::initVars(ModeloCplex& mCplex){
	lambdaInt = IloIntVarArray(mCplex.env, mCplex.lambdaPrimal.getSize(), 0, 1);
	gammaInt = IloIntVarArray(mCplex.env, mCplex.gammaPrimal.getSize(), 0, 1);
}

void ModeloHeuristica::setObjectiveFunction(ModeloCplex& mCplex){
	IloExpr expObj(mCplex.env);
	for (int r = 0; r < mCplex.ptrRotasForn.size(); ++r) expObj += (mCplex.ptrRotasForn[r]->getCusto() * lambdaInt[r]);
	for (int r = 0; r < mCplex.ptrRotasCons.size(); ++r) expObj += (mCplex.ptrRotasCons[r]->getCusto() * gammaInt[r]);
	objective = IloObjective(mCplex.env, expObj);
	model.add(objective);
}

void ModeloHeuristica::setConstraints1e2(ModeloCplex& mCplex){
	IloExpr exp1(mCplex.env);
	for (int r = 0; r < lambdaInt.getSize(); ++r) exp1 += lambdaInt[r];
	constraints1 = ( exp1 == mCplex.maxVeic );
	model.add(constraints1);
	exp1.end();

	IloExpr exp2(mCplex.env);
	for (int r = 0; r < gammaInt.getSize(); ++r) exp2 += gammaInt[r];
	constraints2 = ( exp2 == mCplex.maxVeic );
	model.add(constraints2);
	exp2.end();
}

void ModeloHeuristica::setConstraints3e4(ModeloCplex& mCplex){
	int numCommod = mCplex.nCommodities;
	vector<short int*> matrizA_qr = mCplex.A_qr;
	vector<short int*> matrizB_qr = mCplex.B_qr;
	constraints3 = IloRangeArray(mCplex.env, numCommod);
	constraints4 = IloRangeArray(mCplex.env, numCommod);

	for (int i = 1; i <= numCommod; ++i)
	{
		IloExpr exp3(mCplex.env);
		for (int r = 0; r < mCplex.qRotasForn; ++r)
		{
			if(matrizA_qr[r][i] > 0)
			{
				exp3 += lambdaInt[r];
			}
		}
		constraints3[i-1] = (exp3 == 1);
		model.add(constraints3[i-1]);
		exp3.end();

		IloExpr exp4(mCplex.env);
		for (int r = 0; r < mCplex.qRotasCons; ++r)
		{
			if (matrizB_qr[r][i] > 0)
			{
				exp4 += gammaInt[r];
			}
		}
		constraints4[i-1] = (exp4 == 1);
		model.add(constraints4[i-1]);
		exp4.end();
	}
}


float ModeloHeuristica::optimize(){
	cplex.setParam(IloCplex::TiLim, 600);
	cplex.solve();
	if ((cplex.getStatus() == IloAlgorithm::Feasible) || (cplex.getStatus() == IloAlgorithm::Optimal)){
		return cplex.getObjValue();
	}else{
		return MAIS_INFINITO;
	}
}
