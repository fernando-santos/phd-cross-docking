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
	
	for (int k = 0; k < maxVeiculos; ++k)
	{
		lambdaInt[k].endElements();
		gammaInt[k].endElements();
		deltaInt[k].endElements();
		tauInt[k].endElements();
	}
	lambdaInt.end();
	gammaInt.end();
	deltaInt.end();
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
	int totalRotas = 0;
	int totalBasicas = 0;
	int maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;

	for (int k = 0; k < maxV; ++k)
	{
		totalRotas += mCplex.qRotasForn[k];
		totalRotas += mCplex.qRotasCons[k];
		totalRotas += mCplex.qRotasDiretas[k];
	}

	//Caso o problema tenha um numero de rotas suficiente para colocar todas, faz isto
	if (totalRotas < 8000)
	{
		for (int k = 0; k < maxV; ++k)
		{
			for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
			{
				mCplex.ptrRotasForn[k][r]->setBasica();
			}
			for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
			{
				mCplex.ptrRotasCons[k][r]->setBasica();
			}
			for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
			{
				mCplex.ptrRotasDiretas[k][r]->setBasica();
			}
		}
	}
	else
	{
		totalBasicas = 0;
		for (int k = 0; k < maxV; ++k)
		{
			for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
			{
				if (mCplex.ptrRotasForn[k][r]->getBasica()) ++totalBasicas;
			}
			for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
			{
				if (mCplex.ptrRotasCons[k][r]->getBasica()) ++totalBasicas;
			}
			for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
			{
				if (mCplex.ptrRotasDiretas[k][r]->getBasica()) ++totalBasicas;
			}
		}

		int aleat, k = 0;
		while(totalBasicas < 8000)
		{
			aleat = rand() % mCplex.qRotasForn[k];
			mCplex.ptrRotasForn[k][aleat]->setBasica();
			totalBasicas += mCplex.ptrRotasForn[k][aleat]->getNumApontadores();

			aleat = rand() % mCplex.qRotasCons[k];
			mCplex.ptrRotasCons[k][aleat]->setBasica();
			totalBasicas += mCplex.ptrRotasCons[k][aleat]->getNumApontadores();
			
			aleat = rand() % mCplex.qRotasDiretas[k];
			mCplex.ptrRotasDiretas[k][aleat]->setBasica();
			totalBasicas += mCplex.ptrRotasDiretas[k][aleat]->getNumApontadores();

			if (++k == maxV) k = 0;
		}
	}

	lambdaInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++)
	{
		totalBasicas = 0;
		for (int r = 0; r < mCplex.qRotasForn[i]; ++r)
		{
			if (mCplex.ptrRotasForn[i][r]->getBasica()) ++totalBasicas;
		}
		lambdaInt[i] = IloIntVarArray(mCplex.env, totalBasicas, 0, 1);
	}

	gammaInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++)
	{
		totalBasicas = 0;
		for (int r = 0; r < mCplex.qRotasCons[i]; ++r)
		{
			if (mCplex.ptrRotasCons[i][r]->getBasica()) ++totalBasicas;
		}
		gammaInt[i] = IloIntVarArray(mCplex.env, totalBasicas, 0, 1);
	}

	deltaInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++)
	{
		totalBasicas = 0;
		for (int r = 0; r < mCplex.qRotasDiretas[i]; ++r)
		{
			if (mCplex.ptrRotasDiretas[i][r]->getBasica()) ++totalBasicas;
		}
		deltaInt[i] = IloIntVarArray(mCplex.env, totalBasicas, 0, 1);
	}

	tauInt = IloArray<IloIntVarArray>(mCplex.env, maxV);
	for (int i = 0; i < maxV; i++)
	{
		tauInt[i] = IloIntVarArray(mCplex.env, (numCommod+1), 0, 1);
	}
}

void ModeloHeuristica::setObjectiveFunction(ModeloCplex& mCplex){
	int count, maxV = mCplex.maxVeic;
	int numCommod = mCplex.nCommodities;
	IloExpr expObj = IloExpr(mCplex.env);

	for (int k = 0; k < maxV; ++k)
	{
		count = -1;
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
		{
			if (mCplex.ptrRotasForn[k][r]->getBasica()) expObj += (mCplex.ptrRotasForn[k][r]->getCusto() * lambdaInt[k][++count]);
		}
	}
	for (int k = 0; k < maxV; ++k)
	{
		count = -1;
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
		{
			if (mCplex.ptrRotasCons[k][r]->getBasica())	expObj += (mCplex.ptrRotasCons[k][r]->getCusto() * gammaInt[k][++count]);
		}
	}
	for (int k = 0; k < maxV; ++k)
	{
		count = -1;
		for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
		{
			if (mCplex.ptrRotasDiretas[k][r]->getBasica()) expObj += mCplex.ptrRotasDiretas[k][r]->getCusto() * deltaInt[k][++count];
		}
	}
	for (int k = 0; k < maxV; ++k)
	{
		for (int i = 1; i <= numCommod; ++i)
		{
			expObj += (mCplex.custoTrocaCD * tauInt[k][i]);
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
		count = -1;
		IloExpr exp1 = IloExpr(mCplex.env);
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
		{
			if (mCplex.ptrRotasForn[k][r]->getBasica()) exp1 += lambdaInt[k][++count];
		}
		count = -1;
		for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
		{
			if (mCplex.ptrRotasDiretas[k][r]->getBasica()) exp1 += deltaInt[k][++count];
		}
		constraints1[k] = (exp1 == 1);
		model.add(constraints1[k]);
		exp1.end();

		count =-1;
		IloExpr exp2 = IloExpr(mCplex.env);
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
		{
			if (mCplex.ptrRotasCons[k][r]->getBasica()) exp2 += gammaInt[k][++count];
		}
		count = -1;
		for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
		{
			if (mCplex.ptrRotasDiretas[k][r]->getBasica()) exp2 += deltaInt[k][++count];
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
	vector<short int*>* matrizD_qr = mCplex.D_qr;
	constraints3 = IloRangeArray(mCplex.env, numCommod);
	constraints4 = IloRangeArray(mCplex.env, numCommod);

	for (int i = 1; i <= numCommod; ++i)
	{
		IloExpr exp3 = IloExpr(mCplex.env);
		for (int k = 0; k < maxV; ++k)
		{
			count = -1;
			for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
			{
				if (mCplex.ptrRotasForn[k][r]->getBasica())
				{
					++count;
					if(matrizA_qr[k][r][i] > 0)
					{
						exp3 += lambdaInt[k][count];
					}
				}
			}
			count = -1;
			for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
			{
				if (mCplex.ptrRotasDiretas[k][r]->getBasica())
				{
					++count;
					if(matrizD_qr[k][r][i] > 0)
					{
						exp3 += deltaInt[k][count];
					}
				}
			}
		}
		constraints3[i-1] = (exp3 == 1);
		model.add(constraints3[i-1]);

		IloExpr exp4 = IloExpr(mCplex.env);
		for (int k = 0; k < maxV; ++k)
		{
			count = -1;
			for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
			{
				if (mCplex.ptrRotasCons[k][r]->getBasica())
				{
					++count;
					if (matrizB_qr[k][r][i] > 0)
					{
						exp4 += gammaInt[k][count];
					}
				}
			}
			count = -1;
			for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
			{
				if (mCplex.ptrRotasDiretas[k][r]->getBasica())
				{
					++count;
					if(matrizD_qr[k][r][numCommod + i] > 0)
					{
						exp4 += deltaInt[k][count];
					}
				}
			}
		}
		constraints4[i-1] = (exp4 == 1);
		model.add(constraints4[i-1]);
	}
}

void ModeloHeuristica::setConstraints5e6(ModeloCplex& mCplex){
	int count, maxV = mCplex.maxVeic;
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
			count = -1;
			for (int r = 0; r < mCplex.qRotasForn[k]; ++r){
				if (mCplex.ptrRotasForn[k][r]->getBasica()){
					++count;
					if (matrizA_qr[k][r][i] > 0){
						exp5 += lambdaInt[k][count];
						exp6 -= lambdaInt[k][count];
					}
				}
			}

			//CONSUMIDORES
			count = -1;
			for (int r = 0; r < mCplex.qRotasCons[k]; ++r){
				if (mCplex.ptrRotasCons[k][r]->getBasica()){
					++count;
					if (matrizB_qr[k][r][i] > 0){
						exp5 -= gammaInt[k][count];
						exp6 += gammaInt[k][count];
					}
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
	int limite = 8 * lambdaInt.getSize() + pow(3, tauInt[0].getSize()/10);
	cplex.setParam(IloCplex::TiLim, limite);
	cplex.setParam(IloCplex::Threads, 2);
	cplex.solve();
	if ((cplex.getStatus() == IloAlgorithm::Feasible) || (cplex.getStatus() == IloAlgorithm::Optimal)){
		return cplex.getObjValue();
	}else{
		return MAIS_INFINITO;
	}
}
