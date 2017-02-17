#include "ModeloHeuristica.h"

ModeloHeuristica::~ModeloHeuristica(){
	int maxVeiculos = lambdaInt.getSize();
	objective.end();
	constraints1.endElements();
	constraints2.endElements();
	constraints3.endElements();
	constraints4.endElements();
	constraints5.endElements();
	
	for (int k = 0; k < maxVeiculos; ++k)
	{
		lambdaInt[k].endElements();
		gammaInt[k].endElements();
		for (int kC = 0; kC < maxVeiculos; ++kC)
		{
			tauInt[k][kC].endElements();
		}
		tauInt[k].end();
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
	setConstraints5(mCplex);
}

void ModeloHeuristica::initVars(ModeloCplex& mCplex){
	int totalRotas = 0;
	int totalBasicas = 0;
	int maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;

	lambdaInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++)
	{
		lambdaInt[i] = IloIntVarArray(mCplex.env, mCplex.qRotasForn[i], 0, 1);
	}

	gammaInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++)
	{
		gammaInt[i] = IloIntVarArray(mCplex.env, mCplex.qRotasCons[i], 0, 1);
	}

	tauInt = IloArray < IloArray < IloIntVarArray > > (mCplex.env, maxV);
	for (int kForn = 0; kForn < maxV; kForn++)
	{
		tauInt[kForn] = IloArray < IloIntVarArray > (mCplex.env, maxV);
		for (int kCons = 0; kCons < maxV; kCons++)
		{
			tauInt[kForn][kCons] = IloIntVarArray(mCplex.env, (numCommod+1), 0, 1);
		}
	}
}

void ModeloHeuristica::setObjectiveFunction(ModeloCplex& mCplex){
	int count, maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;
	IloExpr expObj = IloExpr(mCplex.env);

	for (int k = 0; k < maxV; ++k)
	{
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
		{
			expObj += (mCplex.ptrRotasForn[k][r]->getCusto() * lambdaInt[k][r]);
		}
	}
	for (int k = 0; k < maxV; ++k)
	{
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
		{
			expObj += (mCplex.ptrRotasCons[k][r]->getCusto() * gammaInt[k][r]);
		}
	}

	float custoTau;
	for (int kForn = 0; kForn < maxV; kForn++)
	{
		for (int kCons = 0; kCons < maxV; kCons++)
		{
			expObj += MAIS_INFINITO * tauInt[kForn][kCons][0];
			if ( kForn == kCons ) custoTau = 0;
			else custoTau = mCplex.custoTrocaCD;
			for (int i = 1; i <= numCommod; i++)
			{
				if (kForn != kCons) expObj += custoTau * tauInt[kForn][kCons][i];
			}
		}
	}

	objective = IloObjective(mCplex.env, expObj);
	model.add(objective);
}

void ModeloHeuristica::setConstraints1e2(ModeloCplex& mCplex){
	int count, maxV = mCplex.maxVeic;
	constraints1 = IloRangeArray(mCplex.env, maxV);
	constraints2 = IloRangeArray(mCplex.env, maxV);

	for (int k = 0; k < maxV; ++k)
	{
		IloExpr exp1 = IloExpr(mCplex.env);
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
		{
			exp1 += lambdaInt[k][r];
		}
		constraints1[k] = (exp1 == 1);
		model.add(constraints1[k]);
		exp1.end();

		IloExpr exp2 = IloExpr(mCplex.env);
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
		{
			exp2 += gammaInt[k][r];
		}
		constraints2[k] = (exp2 == 1);
		model.add(constraints2[k]);
		exp2.end();
	}
}

void ModeloHeuristica::setConstraints3e4(ModeloCplex& mCplex){
	int count, maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;
	vector<short int*>* matrizA_qr = mCplex.A_qr;
	vector<short int*>* matrizB_qr = mCplex.B_qr;
	constraints3 = IloRangeArray(mCplex.env, numCommod);
	constraints4 = IloRangeArray(mCplex.env, numCommod);

	for (int i = 1; i <= numCommod; ++i)
	{
		IloExpr exp3 = IloExpr(mCplex.env);
		for (int k = 0; k < maxV; ++k)
		{
			for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
			{
				if (matrizA_qr[k][r][i] > 0)
				{
					exp3 += lambdaInt[k][r];
				}
			}
		}
		constraints3[i-1] = (exp3 == 1);
		model.add(constraints3[i-1]);

		IloExpr exp4 = IloExpr(mCplex.env);
		for (int k = 0; k < maxV; ++k)
		{
			for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
			{
				if (matrizB_qr[k][r][i] > 0)
				{
					exp4 += gammaInt[k][r];
				}
			}
		}
		constraints4[i-1] = (exp4 == 1);
		model.add(constraints4[i-1]);
	}
}

void ModeloHeuristica::setConstraints5(ModeloCplex& mCplex){
	int contadorRestr = 0;
	int count, maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;
	vector<short int*>* matrizA_qr = mCplex.A_qr;
	vector<short int*>* matrizB_qr = mCplex.B_qr;
	constraints5 = IloRangeArray(mCplex.env, maxV*maxV*numCommod);
	
	for (int kForn = 0; kForn < maxV; ++kForn)
	{
		for (int kCons = 0; kCons < maxV; ++kCons)
		{
			for (int i = 1; i <= numCommod; ++i)
			{
				IloExpr exp5 = IloExpr(mCplex.env);
	
				for (int r = 0; r < mCplex.qRotasForn[kForn]; ++r)
				{
					if (matrizA_qr[kForn][r][i] > 0)
					{
						exp5 += lambdaInt[kForn][r];
					}
				}

				for (int r = 0; r < mCplex.qRotasCons[kCons]; ++r)
				{
					if (matrizB_qr[kCons][r][i] > 0)
					{
						exp5 += gammaInt[kCons][r];
					}
				}	
				
				exp5 -= 2*tauInt[kForn][kCons][i];
				constraints5[contadorRestr] = (exp5 <= 1);
				model.add(constraints5[contadorRestr]);
				++contadorRestr;
				exp5.end();
			}
		}
	}
}

float ModeloHeuristica::optimize(){
	int limite = 10 * lambdaInt.getSize() + pow(3, tauInt[0].getSize()/10);
	cplex.setParam(IloCplex::TiLim, limite);
	cplex.solve();
	if ((cplex.getStatus() == IloAlgorithm::Feasible) || (cplex.getStatus() == IloAlgorithm::Optimal))
	{
		return cplex.getObjValue();
	}
	else
	{
		return MAIS_INFINITO;
	}
}
