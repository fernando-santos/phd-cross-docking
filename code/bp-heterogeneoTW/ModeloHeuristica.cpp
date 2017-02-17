#include "ModeloHeuristica.h"

ModeloHeuristica::~ModeloHeuristica(){
	int maxVeiculos = lambdaInt.getSize();
	objective.end();
	constraints1.endElements();
	constraints2.endElements();
	constraints3.endElements();
	constraints4.endElements();
	constraints5.endElements();
	constraints6.endElements();
	
	for (int k = 0; k < maxVeiculos; ++k){
		lambdaInt[k].endElements();
		gammaInt[k].endElements();
		tauInt[k].endElements();
	}
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
	setConstraints5e6(mCplex);
}

void ModeloHeuristica::initVars(ModeloCplex& mCplex){
	int maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;

	lambdaInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++){
		lambdaInt[i] = IloIntVarArray(mCplex.env, mCplex.qRotasForn[i], 0, 1);
	}

	gammaInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++){
		gammaInt[i] = IloIntVarArray(mCplex.env, mCplex.qRotasCons[i], 0, 1);
	}

	tauInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++){
		tauInt[i] = IloIntVarArray(mCplex.env, (numCommod+1), 0, 1);
	}
}

void ModeloHeuristica::setObjectiveFunction(ModeloCplex& mCplex){
	int maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;
	IloExpr expObj = IloExpr(mCplex.env);

	for (int k = 0; k < maxV; ++k){
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r){
			expObj += (mCplex.ptrRotasForn[k][r]->getCusto() * lambdaInt[k][r]);
		}
	}
	for (int k = 0; k < maxV; ++k){
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r){
			expObj += (mCplex.ptrRotasCons[k][r]->getCusto() * gammaInt[k][r]);
		}
	}
	for (int k = 0; k < maxV; ++k){
		for (int i = 1; i <= numCommod; ++i){
			expObj += (mCplex.custoTrocaCD * tauInt[k][i]);
		}
	}
	objective = IloObjective(mCplex.env, expObj);
	model.add(objective);
}

void ModeloHeuristica::setConstraints1e2(ModeloCplex& mCplex){
	int maxV = mCplex.maxVeic;
	constraints1 = IloRangeArray(mCplex.env, maxV);
	constraints2 = IloRangeArray(mCplex.env, maxV);

	for (int k = 0; k < maxV; ++k){
		IloExpr exp1 = IloExpr(mCplex.env);
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r){
			exp1 += lambdaInt[k][r];
		}
		constraints1[k] = (exp1 == 1);
		model.add(constraints1[k]);
		exp1.end();
		
		IloExpr exp2 = IloExpr(mCplex.env);
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r){
			exp2 += gammaInt[k][r];
		}
		constraints2[k] = (exp2 == 1);
		model.add(constraints2[k]);
		exp2.end();
	}
}

void ModeloHeuristica::setConstraints3e4(ModeloCplex& mCplex){
	int maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;
	vector<short int*>* matrizA_qr = mCplex.A_qr;
	vector<short int*>* matrizB_qr = mCplex.B_qr;
	constraints3 = IloRangeArray(mCplex.env, numCommod);
	constraints4 = IloRangeArray(mCplex.env, numCommod);

	for (int i = 1; i <= numCommod; ++i){
		IloExpr exp3 = IloExpr(mCplex.env);
		for (int k = 0; k < maxV; ++k){
			for (int r = 0; r < mCplex.qRotasForn[k]; ++r){
				if(matrizA_qr[k][r][i] > 0){
					exp3 += lambdaInt[k][r];
				}
			}
		}
		constraints3[i-1] = (exp3 == 1);
		model.add(constraints3[i-1]);

		IloExpr exp4 = IloExpr(mCplex.env);
		for (int k = 0; k < maxV; ++k){
			for (int r = 0; r < mCplex.qRotasCons[k]; ++r){
				if (matrizB_qr[k][r][i] > 0){
					exp4 += gammaInt[k][r];
				}
			}
		}
		constraints4[i-1] = (exp4 == 1);
		model.add(constraints4[i-1]);
	}
}

void ModeloHeuristica::setConstraints5e6(ModeloCplex& mCplex){
	int maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;
	constraints5 = IloRangeArray(mCplex.env, maxV*numCommod);
	constraints6 = IloRangeArray(mCplex.env, maxV*numCommod);
	vector<short int*>* matrizA_qr = mCplex.A_qr;
	vector<short int*>* matrizB_qr = mCplex.B_qr;
	int contadorRestr = 0;

	for (int k = 0; k < maxV; ++k){
		for (int i = 1; i <= numCommod; ++i){
			constraints5[contadorRestr] = IloRange(mCplex.env, 0, +IloInfinity);
			constraints6[contadorRestr] = IloRange(mCplex.env, 0, +IloInfinity);
			IloExpr exp5 = IloExpr(mCplex.env);
			IloExpr exp6 = IloExpr(mCplex.env);
			
			//FORNECEDORES
			for (int r = 0; r < mCplex.qRotasForn[k]; ++r){
				if (matrizA_qr[k][r][i] > 0){
					exp5 += lambdaInt[k][r];
					exp6 -= lambdaInt[k][r];
				}
			}

			//CONSUMIDORES
			for (int r = 0; r < mCplex.qRotasCons[k]; ++r){
				if (matrizB_qr[k][r][i] > 0){
					exp5 -= gammaInt[k][r];
					exp6 += gammaInt[k][r];
				}
			}
			exp5 += tauInt[k][i];
			exp6 += tauInt[k][i];
			
			constraints5[contadorRestr] = (exp5 >= 0);
			constraints6[contadorRestr] = (exp6 >= 0);
			model.add(constraints5[contadorRestr]);
			model.add(constraints6[contadorRestr]);
			++contadorRestr;
		}
	}
}

float ModeloHeuristica::optimize(){
	int limite = 10 * lambdaInt.getSize();; // 10x o numero de veiculos
	cplex.setParam(IloCplex::TiLim, limite);
	cplex.solve();
	if ((cplex.getStatus() == IloAlgorithm::Feasible) || (cplex.getStatus() == IloAlgorithm::Optimal)){
		return cplex.getObjValue();
	}else{
		return MAIS_INFINITO;
	}
}
