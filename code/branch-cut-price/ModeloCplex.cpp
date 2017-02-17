#include "ModeloCplex.h"
#include <sys/time.h>
using namespace std;

float ModeloCplex::limitePrimal;
Rota** retornaColuna(Grafo* G, bool camForn, float valSub, char op ){
	Rota** r;
	if (op == 'S')
	{
		StateSpaceRelax camElemSSR(G, camForn, valSub);
		if ( camElemSSR.calculaCaminhoElementarSSR( G ) > 0 )
		{
			r = camElemSSR.getRotaCustoMinimo( G, 0.99 );
		}
		else
		{
			r = NULL;
		}
		return r;
	}
	else if (op == 'E')
	{
		Elementar camElem(G, camForn, valSub);
		if ( camElem.calculaCaminhoElementar(G, true) > 0 )
		{
			r = camElem.getRotaCustoMinimo(G, 0.99 );
		}
		else
		{
			r = NULL;
		}
		return r;
	}
}


ModeloCplex::~ModeloCplex(){
	for (int r = 0; r < qRotasForn; ++r)
	{
		delete [] A_qr[r];
		if (ptrRotasForn[r]->decrNumApontadores()) delete ptrRotasForn[r];
	}
	for (int r = 0; r < qRotasCons; ++r)
	{
		delete [] B_qr[r];
		if (ptrRotasCons[r]->decrNumApontadores()) delete ptrRotasCons[r];
	}
	A_qr.clear();
	B_qr.clear();
	ptrRotasForn.clear();
	ptrRotasCons.clear();
	env.end();
}


ModeloCplex::ModeloCplex(Grafo* g, Rota** rForn, Rota** rCons, int nRotasF, int nRotasC, int maxVeiculos, int numCommodities, int custoTroca)
: env(), modelPrimal(env), cplexPrimal(modelPrimal), maxVeic(maxVeiculos), nCommodities(numCommodities), custoTrocaCD(custoTroca) {

	cplexPrimal.setOut(env.getNullStream());
	qRotasForn = nRotasF;
	qRotasCons = nRotasC;

	setA_qr(rForn);
	setB_qr(rCons);
	
	initVarsPrimal();
	setObjectiveFunctionPrimal();
	setConstraintsPrimal1();
	setConstraintsPrimal2();
	setConstraintsPrimal3();
	setConstraintsPrimal4();
	setConstraintsPrimal5_6_7_8();
}


void ModeloCplex::exportModel(const char* arqExport){
	cplexPrimal.exportModel(arqExport);
}


float ModeloCplex::solveMaster(){
	cplexPrimal.solve();
	return cplexPrimal.getObjValue();
}


void ModeloCplex::updateDualCostsForn(Grafo* g){
        int numVertAtualizar;
        vector<int> verticesRota;
        IloNumArray thetaValues(env), piValues(env);
        float *rotasValues = new float[nCommodities+1];
        memset(rotasValues, 0, (nCommodities+1) * sizeof(float));

        cplexPrimal.getDuals(thetaValues, constraintsPrimal3);
//	cplexPrimal.getDuals(piValues, constraintsPrimal7);
        for (int i = 1; i <= nCommodities; ++i)
        {
//		printf("piValuesF[%d] = %f\n", i, piValues[i-1]);
                g->setCustoVerticeDual(i, -thetaValues[i-1]);//+piValues[i-1]);
        }
        g->setCustoArestasDual('F');

        thetaValues.end(); piValues.end();
        delete [] rotasValues;
}


void ModeloCplex::updateDualCostsCons(Grafo* g){
        int numVertAtualizar;
        vector<int> verticesRota;
        IloNumArray muValues(env), piValues(env);
        float *rotasValues = new float[nCommodities+1];
        memset(rotasValues, 0, (nCommodities+1) * sizeof(float));

        cplexPrimal.getDuals(muValues, constraintsPrimal4);
//	cplexPrimal.getDuals(piValues, constraintsPrimal7);
        for (int i = 1; i <= nCommodities; ++i)
        {
//		printf("piValuesC[%d] = %f\n", i, piValues[i-1]);
                g->setCustoVerticeDual(nCommodities+i, -muValues[i-1]);//+piValues[i-1]);
        }
        g->setCustoArestasDual('C');

        muValues.end(); piValues.end();
        delete [] rotasValues;
}


float ModeloCplex::getAlfaDual(){
	return cplexPrimal.getDual(constraintsPrimal1);
}


float ModeloCplex::getBetaDual(){
	return cplexPrimal.getDual(constraintsPrimal2);
}


void ModeloCplex::insertColumnForn(Rota* r){
	char nome[20];
	vector<int> vGamma;
	bool coletaEntrega;
	int numColetaEntrega;
	short int* rota = new short int[nCommodities+1];
	memset(rota, 0, (nCommodities+1) * sizeof(short int));
	A_qr.push_back(rota);
	ptrRotasForn.push_back(r);
	r->incrNumApontadores();
	
	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j)
	{
		++A_qr[qRotasForn][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//inicializacao do vetor para armazenar o numero de visitas de cada vertice da coluna
	int* vetVisitRota = new int [nCommodities];
	memset (vetVisitRota, 0, nCommodities*sizeof(int));

	//coloca os indices (> 0) nas respectivas posicoes da coluna
	for (int i = 1; i <= numVertAtualizar; ++i)
	{
		++vetVisitRota[vertRota[i]-1];
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal1(1);
	for (int i = 0; i < nCommodities; i++)
	{
		if (vetVisitRota[i] > 0)
		{
			col += constraintsPrimal3[i](vetVisitRota[i]);
		}
	}
	sprintf(nome, "lbd_%d", qRotasForn);
	lambdaPrimal.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
	col.end();

	//insere os atributos de troca relacionados a rota que acaba de ser inserida
	tauPrimal.add(IloNumVarArray(env));
	IloExpr exp5 = lambdaPrimal[qRotasForn];
	for ( int j = 0; j < qRotasCons; ++j )
	{
		IloNumColumn colTau = costPrimal(0);
		colTau += constraintsPrimal6[j](1);
		for ( int i = 1; i <= nCommodities; ++i )
		{
			if ( ( A_qr[qRotasForn][i] > 0 ) && ( B_qr[j][i] > 0 ) )
			{
				colTau += constraintsPrimal7[i-1](1);
			}
		}
//		colTau += constraintPrimal8(1);
		sprintf(nome, "tau_%d_%d", qRotasForn, j);
		tauPrimal[qRotasForn].add(IloNumVar(colTau, 0, 1, ILOFLOAT, nome));
		exp5 -= tauPrimal[qRotasForn][j];
		colTau.end();
	}
	constraintsPrimal5.add(exp5 == 0);
	sprintf(nome, "rest5_%d", qRotasForn);
	constraintsPrimal5[qRotasForn].setName(nome);
	modelPrimal.add(constraintsPrimal5[qRotasForn]);

	++qRotasForn;
	delete [] vetVisitRota;
}

void ModeloCplex::insertColumnCons(Rota* r){
	char nome[20];
	vector<int> vLambda;
	bool coletaEntrega;
	int numColetaEntrega;
	short int* rota = new short int[nCommodities+1];
	memset(rota, 0, (nCommodities+1) * sizeof(short int));
	B_qr.push_back(rota);
	ptrRotasCons.push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j)
	{
		++B_qr[qRotasCons][vertRota[j]-nCommodities]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//inicializacao do vetor para armazenar o numero de visitas de cada vertice da coluna
	int* vetVisitRota = new int [nCommodities];
	memset (vetVisitRota, 0, nCommodities*sizeof(int));

	//coloca os indices (> 0) nas respectivas posicoes da coluna
	for (int i = 1; i <= numVertAtualizar; ++i)
	{
		++vetVisitRota[vertRota[i]-nCommodities-1];
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal2(1);
	for (int i = 0; i < nCommodities; i++)
	{
		if (vetVisitRota[i] > 0)
		{
			col += constraintsPrimal4[i](vetVisitRota[i]);
		}
	}
	sprintf(nome, "gam_%d", qRotasCons);
	gammaPrimal.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
	col.end();

	IloExpr exp6 = gammaPrimal[qRotasCons];
	for ( int j = 0; j < qRotasForn; ++j )
	{
		IloNumColumn colTau = costPrimal(0);
		colTau += constraintsPrimal5[j](1);
		for ( int i = 1; i <= nCommodities; ++i )
		{
			if ( ( A_qr[j][i] > 0 ) && ( B_qr[qRotasCons][i] > 0 ) )
			{
				colTau += constraintsPrimal7[i-1](1);
			}
		}
//		colTau += constraintPrimal8(1);
		sprintf(nome, "tau_%d_%d", j, qRotasCons);
		tauPrimal[j].add(IloNumVar(colTau, 0, 1, ILOFLOAT, nome));
		exp6 -= tauPrimal[j][qRotasCons];
		colTau.end();
	}
	constraintsPrimal6.add(exp6 == 0);
	sprintf(nome, "rest6_%d", qRotasCons);
	constraintsPrimal6[qRotasCons].setName(nome);
	modelPrimal.add(constraintsPrimal6[qRotasCons]);

	++qRotasCons;
	delete [] vetVisitRota;
}


void ModeloCplex::initVarsPrimal(){
	lambdaPrimal = IloNumVarArray(env, qRotasForn, 0, 1, ILOFLOAT);
	for (int i = 0; i < qRotasForn; ++i)
	{
		char nome[20];
		sprintf(nome, "lbd_%d", i);
		lambdaPrimal[i].setName(nome);
	}

	gammaPrimal = IloNumVarArray(env, qRotasCons, 0, 1, ILOFLOAT);
	for (int i = 0; i < qRotasCons; ++i)
	{
		char nome[20];
		sprintf(nome, "gam_%d", i);
		gammaPrimal[i].setName(nome);
	}

	tauPrimal = IloArray < IloNumVarArray > (env, qRotasForn);
	for (int i = 0; i < qRotasForn; i++)
	{
		tauPrimal[i] = IloNumVarArray(env, qRotasCons, 0, 1, ILOFLOAT);
		for (int j = 0; j < qRotasCons; j++)
		{
			char nome[20];
			sprintf(nome, "tau_%d_%d", i, j);
			tauPrimal[i][j].setName(nome);
		}
	}

	tau = IloNumVarArray(env, nCommodities+1, 0, 1, ILOFLOAT);
	for (int i = 0; i <= nCommodities; ++i)
	{
		char nome[20];
		sprintf(nome, "tau_%d", i);
		tau[i].setName(nome);
	}

}

void ModeloCplex::setObjectiveFunctionPrimal(){
	IloExpr obj(env);
	bool coletaEntrega;
	int numColetaEntrega, total;
	vector<int> vLambda, vGamma;
	
	//Adiciona-se os coeficientes relacionados as variaveis lambdaPrimal
	for (int i = 0; i < qRotasForn; ++i)
	{
		obj += ptrRotasForn[i]->getCusto() * lambdaPrimal[i];
	}
		
	//E também adiciona-se os coeficientes relacionados as variaveis gammaPrimal
	for (int i = 0; i < qRotasCons; ++i)
	{
		obj += ptrRotasCons[i]->getCusto() * gammaPrimal[i];
	}

	obj += custoTrocaCD * nCommodities;
	for (int i = 1; i <= nCommodities; ++i)
	{
		obj -= custoTrocaCD * tau[i];
	}

/*	for ( int i = 0; i < qRotasForn; ++i )
	{
		vLambda = ptrRotasForn[i]->getVertices();
		for ( int j = 0; j < qRotasCons; ++j )
		{
			vGamma = ptrRotasCons[j]->getVertices();
			numColetaEntrega = 2; //vertices 0 e 0' sao 'coletados e entregues' por todas as rotas
			for ( int l = 1; l < (vGamma.size()-1); ++l )
			{
				//a troca sera computada quando a rota dos consumidores tiver um vertice que nao esteja na rota associada aos consumidores
				coletaEntrega = false;
				for ( int k = 1; k < (vLambda.size()-1); ++k )
				{
					if ( vLambda[k] == ( vGamma[l] - nCommodities ) )
					{
						coletaEntrega = true;
						break;
					}
				}
				if ( coletaEntrega ) ++numColetaEntrega;
			}
			total = custoTrocaCD * ( vLambda.size() + vGamma.size() - 2*numColetaEntrega );
			obj += total * tauPrimal[i][j];
		}
	}
*/
	//Cria o objeto para armazenar a funcao objetivo e atribui a expressao a ele
	costPrimal = IloObjective(env, obj);
	modelPrimal.add(costPrimal);
}


void ModeloCplex::setA_qr(Rota** rForn){
	int tam;
	vector<int> v;
	short int* rota;

	//Cada linha de A_qr apontara para um vetor de inteiros de tamanho numCommodities
	//Cada posicao do vetor sera 1 caso o vertice i esteja na rota r, ou 0 caso contrario
	for (int i = 0; i < qRotasForn; ++i)
	{
		rota = new short int[nCommodities+1];
		memset(rota, 0, (nCommodities+1) * sizeof(short int));
		A_qr.push_back(rota);
		v = rForn[i]->getVertices();
		tam = v.size()-1;
		for(int j = 1; j < tam; ++j)
		{
			A_qr[i][v[j]] = 1; //coloca-se 1 apenas naquelas posicoes em que o vertice esteja na rota
		}
		ptrRotasForn.push_back(rForn[i]);
		rForn[i]->incrNumApontadores();
	}
}


void ModeloCplex::setB_qr(Rota** rCons){
	int tam;
	vector<int> v;
	short int* rota;

	//Cada linha de B_qr apontara para um vetor de booleanos de tamanho numCommodities
	//Cada posicao do vetor sera 1 caso o vertice i esteja na rota r, ou 0 caso contrario
	for (int i = 0; i < qRotasCons; ++i)
	{
		rota = new short int[nCommodities+1];
		memset(rota, 0, (nCommodities+1) * sizeof(short int));
		B_qr.push_back(rota);
		v = rCons[i]->getVertices();
		tam = v.size()-1;
		for(int j = 1; j < tam; ++j)
		{
			B_qr[i][v[j]-nCommodities] = 1; //coloca-se 1 apenas naquelas posicoes em que o vertice esteja na rota
		}
		ptrRotasCons.push_back(rCons[i]);
		rCons[i]->incrNumApontadores();
	}
}


void ModeloCplex::setConstraintsPrimal1(){
	IloExpr exp1 = IloExpr(env);
	for ( int j = 0; j < qRotasForn; ++j )
	{
		exp1 += lambdaPrimal[j];
	}
	//Insere a restrição no modelo
	constraintsPrimal1 = ( exp1 == maxVeic );
	constraintsPrimal1.setName("rest1");
	modelPrimal.add(constraintsPrimal1);
	exp1.end();
}


void ModeloCplex::setConstraintsPrimal2(){
	IloExpr exp2 = IloExpr(env);
	for (int j = 0; j < qRotasCons; ++j)
	{
		exp2 += gammaPrimal[j];
	}
	//Insere a restrição no modelo
	constraintsPrimal2 = ( exp2 ==  maxVeic );
	constraintsPrimal2.setName("rest2");
	modelPrimal.add(constraintsPrimal2);
	exp2.end();
}


void ModeloCplex::setConstraintsPrimal3(){
	char nome[20];
	constraintsPrimal3 = IloRangeArray(env, nCommodities);
	for (int i = 1; i <= nCommodities; ++i)
	{
		IloExpr exp3 = IloExpr(env);
		for (int l = 0; l < qRotasForn; ++l)
		{
			if (A_qr[l][i] > 0)
			{
				exp3 += A_qr[l][i] * lambdaPrimal[l];
			}
		}
		//Insere a restrição no modelo
		constraintsPrimal3[i-1] = ( exp3 == 1 );
		sprintf(nome, "rest3_i%d", i);
		constraintsPrimal3[i-1].setName(nome);
		modelPrimal.add(constraintsPrimal3[i-1]);
		exp3.end();
	}
}


void ModeloCplex::setConstraintsPrimal4(){
	char nome[20];
	constraintsPrimal4 = IloRangeArray(env, nCommodities);
	for (int i = 1; i <= nCommodities; ++i)
	{
		IloExpr exp4 = IloExpr(env);
		for (int l = 0; l < qRotasCons; ++l)
		{
			if (B_qr[l][i] > 0)
			{
				exp4 += B_qr[l][i] * gammaPrimal[l];
			}
		}
		//Insere a restrição no modelo
		constraintsPrimal4[i-1] = ( exp4 == 1 );
		sprintf(nome, "rest4_i%d", i);
		constraintsPrimal4[i-1].setName(nome);
		modelPrimal.add(constraintsPrimal4[i-1]);
		exp4.end();	
	}
}


void ModeloCplex::setConstraintsPrimal5_6_7_8(){
	char nome[20];
	//para cada par r, r' cria uma CONSTRAINT 5, para cada r cria-se uma CONSTRAINT 6 e para cada r' cria-se uma CONSTRAINT 7
	
	//CONSTRAINTS 5
	constraintsPrimal5 = IloRangeArray(env, qRotasForn);
	for ( int i = 0; i < qRotasForn; ++i )
	{
		IloExpr exp5 = lambdaPrimal[i];
		for ( int j = 0; j < qRotasCons; ++j )
		{
			exp5 -= tauPrimal[i][j];
		}
		constraintsPrimal5[i] = ( exp5 == 0 );
		sprintf(nome, "rest5_%d", i);
		constraintsPrimal5[i].setName(nome);
		modelPrimal.add(constraintsPrimal5[i]);
		exp5.end();
	}

	//CONSTRAINTS 6
	constraintsPrimal6 = IloRangeArray(env, qRotasCons);
	for ( int j = 0; j < qRotasCons; ++j )
	{
		IloExpr exp6 = gammaPrimal[j];
		for ( int i = 0; i < qRotasForn; ++i )
		{
			exp6 -= tauPrimal[i][j];
		}
		constraintsPrimal6[j] = ( exp6 == 0 );
		sprintf(nome, "rest6_%d", j);
		constraintsPrimal6[j].setName(nome);
		modelPrimal.add(constraintsPrimal6[j]);
	}

	//CONSTRAINT 7
	constraintsPrimal7 = IloRangeArray(env, nCommodities);
	for ( int i = 1; i <= nCommodities; ++i )
	{
		IloExpr exp7(env);
		for ( int x = 0; x < qRotasForn; ++x )
		{
			for ( int y = 0; y < qRotasCons; ++y )
			{
				if ( ( A_qr[x][i] > 0 ) && ( B_qr[y][i] > 0 ) )
				{
					exp7 += tauPrimal[x][y];
				}
			}
		}
		constraintsPrimal7[i-1] = ( ( exp7 - tau[i] ) >= 0 );
		sprintf(nome, "rest7_%d", i);
		constraintsPrimal7[i-1].setName(nome);
		modelPrimal.add(constraintsPrimal7[i-1]);
		exp7.end();
	}

	//CONSTRAINT 8
/*	IloExpr exp8(env);
	for ( int i = 0; i < qRotasForn; ++i )
	{
		for ( int j = 0; j < qRotasCons; ++j )
		{
			exp8 += tauPrimal[i][j];
		}
	}
	constraintPrimal8 = ( exp8 == maxVeic );
	constraintPrimal8.setName("rest8");
	modelPrimal.add(constraintPrimal8);
*/
}


void ModeloCplex::setLimitePrimal(float lP){
	limitePrimal = lP;
}


float ModeloCplex::getLimitePrimal(){
	return limitePrimal;
}
