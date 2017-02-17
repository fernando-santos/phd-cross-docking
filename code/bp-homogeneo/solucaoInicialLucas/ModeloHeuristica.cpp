#include "ModeloHeuristica.h"

ModeloHeuristica::~ModeloHeuristica(){
	lambdaInt.end();
	tauInt.end();
	cplex.end();
	model.end();
}

ModeloHeuristica::ModeloHeuristica(ModeloCplex& mCplex){
	model = IloModel(mCplex.env);
	cplex = IloCplex(model);
	cplex.setOut(mCplex.env.getNullStream());

	buildModel(mCplex);
}

void ModeloHeuristica::buildModel(ModeloCplex& mCplex){

	//inicializa as variaveis de decisao
	lambdaInt = IloIntVarArray(mCplex.env, mCplex.qRotas, 0, 1);
	tauInt = IloIntVarArray(mCplex.env, (mCplex.nRequests+1), 0, 1);

	//funcao objetivo
	IloExpr expObj = IloExpr(mCplex.env);
	for (int r = 0; r < mCplex.qRotas; ++r)
	{
		expObj += mCplex.ptrRotas[r]->getCusto() * lambdaInt[r];
	}

	for (int i = 1; i <= mCplex.nRequests; ++i)
	{
		expObj += (mCplex.custoTrocaCD * tauInt[i]);
	}
	model.add(IloObjective(mCplex.env, expObj));

	//Restricoes - Constraint 1
	IloExpr exp1 = IloExpr(mCplex.env);
	for (int r = 0; r < mCplex.qRotas; ++r)
	{
		exp1 += lambdaInt[r];
	}

	model.add(exp1 == mCplex.maxVeic);
	exp1.end();

	//Restricoes - Constraints 2
	for (int i = 1; i <= 2*mCplex.nRequests; ++i)
	{
		IloExpr exp2 = IloExpr(mCplex.env);
		for (int r = 0; r < mCplex.qRotas; ++r)
		{
			if (mCplex.a_ir[r][i] > 0)
			{
				exp2 += mCplex.a_ir[r][i] * lambdaInt[r];
			}
		}

		model.add(exp2 == 1);
		exp2.end();	
	}

	//Restricoes - Constraints 3
	for (int i = 1; i <= mCplex.nRequests; ++i)
	{
		IloExpr exp3 = IloExpr(mCplex.env);
		exp3 += tauInt[i];
		for (int r = 0; r < mCplex.qRotas; ++r)
		{
			if ((mCplex.a_ir[r][i] == 0) && (mCplex.a_ir[r][i+mCplex.nRequests] > 0))
			{
				exp3 -= lambdaInt[r];
			}
		}

		model.add(exp3 >= 0);
		exp3.end();
	}
}

void ModeloHeuristica::imprimeRotasOtimas(ModeloCplex& mCplex){
	IloNumArray lambdaValues(mCplex.env);
	cplex.getValues(lambdaValues, lambdaInt);
	for (int r = 0; r < mCplex.qRotas; ++r)
	{
		if (lambdaValues[r] > 0.0001) mCplex.ptrRotas[r]->imprimir();
	}
}

float ModeloHeuristica::optimize(){
	cplex.solve();
	if ((cplex.getStatus() == IloAlgorithm::Feasible) || (cplex.getStatus() == IloAlgorithm::Optimal)){
		return cplex.getObjValue();
	}else{
		return MAIS_INFINITO;
	}
}
