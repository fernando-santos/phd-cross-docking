#include "ModeloCplex.h"
#include <sys/time.h>
using namespace std;

float ModeloCplex::limitePrimal;
Rota** retornaColuna(Grafo* G, bool camForn, bool pricingFull, float valSub, char op, int& retornoPricing){
	Rota** r;
	Elementar camElem(G, camForn, valSub);
	retornoPricing = camElem.calculaCaminhoElementar(G, pricingFull);
	if (retornoPricing > 0)
	{
		r = camElem.getRotaCustoMinimo(G, 0.90);
		if ( r == NULL )
		{
			if ( pricingFull ) retornoPricing = -1;
			else retornoPricing = 0;
		}
	}
	else
	{
		r = NULL;
	}
	return r;
}


ModeloCplex::~ModeloCplex(){
	for (int k = 0; k < maxVeic; ++k)
	{
		for (int r = 0; r < qRotasForn[k]; ++r)
		{
			delete [] A_qr[k][r];
			if (ptrRotasForn[k][r]->decrNumApontadores()) delete ptrRotasForn[k][r];
		}		
		for (int r = 0; r < qRotasCons[k]; ++r)
		{
			delete [] B_qr[k][r];
			if (ptrRotasCons[k][r]->decrNumApontadores()) delete ptrRotasCons[k][r];
		}
		A_qr[k].clear();
		B_qr[k].clear();
		ptrRotasForn[k].clear();
		ptrRotasCons[k].clear();
	}
	
	delete [] A_qr;
	delete [] B_qr;
	delete [] ptrRotasForn;
	delete [] ptrRotasCons;
	delete [] qRotasForn;
	delete [] qRotasCons;
	env.end();
}


ModeloCplex::ModeloCplex(Grafo* g, Rota** rForn, Rota** rCons, int nRotasF, int nRotasC, int maxVeiculos, int numCommodities, int custoTroca)
: env(), modelPrimal(env), cplexPrimal(modelPrimal), maxVeic(maxVeiculos), nCommodities(numCommodities), custoTrocaCD(custoTroca) {
	//Devido a alteracao na insercao das colunas (que agora serao inseridas apenas para o veiculo que a encontrou)
	//a quantidade de rotas de cada veiculo tanto nos consumidores quanto fornecedores poderá ser diferente,
	//sendo necessario que cada posicao dos vetores abaixo armazene a quantidade associada ao respectivo veiculo
	//Na inicializacao, todas as rotas vindas da heuristica sao associadas a todos os veiculos
	qRotasForn = new int[maxVeic];
	qRotasCons = new int[maxVeic];
	for (int k = 0; k < maxVeic; ++k)
	{
		qRotasForn[k] = 1; //cada veiculo eh inicializado com apenas aquela rota gerada em geraRotas
		qRotasCons[k] = 1;
	}

	cplexPrimal.setOut(env.getNullStream());

	//armazena como atributos da classe as matrizes ptrRotasForn, ptrRotasCons, A_qr e B_qr
	setA_qr(rForn);
	setB_qr(rCons);

	//Montagem do modelo Primal
	initVarsPrimal();
	setObjectiveFunctionPrimal();
	setConstraintsPrimal1();
	setConstraintsPrimal2();
	setConstraintsPrimal3();
	setConstraintsPrimal4();
	setConstraintsPrimal5();
	setConstraintsPrimal6e7();
}


void ModeloCplex::setA_qr(Rota** rForn){
	A_qr = new vector<short int*>[maxVeic];
	ptrRotasForn = new vector<Rota*>[maxVeic];
	vector<int> v;
	short int* rota;
	int tam;

	//Cada linha de A_qr apontara para um vetor de inteiros de tamanho numCommodities
	//Cada posicao do vetor sera 1 caso o vertice i esteja na rota r, ou 0 caso contrario
	for (int k = 0; k < maxVeic; ++k)
	{
		rota = new short int[nCommodities+1];
		memset(rota, 0, (nCommodities+1) * sizeof(short int));
		A_qr[k].push_back(rota);
		v = rForn[k]->getVertices();
		tam = v.size()-1;
		for(int j = 1; j < tam; ++j)
		{
			A_qr[k][0][v[j]] = 1; //coloca-se 1 apenas naquelas posicoes em que o vertice esteja na rota
		}
		ptrRotasForn[k].push_back(rForn[k]);
		rForn[k]->incrNumApontadores();
	}
}


void ModeloCplex::setB_qr(Rota** rCons){
	B_qr = new vector<short int*>[maxVeic];
	ptrRotasCons = new vector<Rota*>[maxVeic];
	vector<int> v;
	short int* rota;
	int tam;

	//Cada linha de B_qr apontara para um vetor de booleanos de tamanho numCommodities
	//Cada posicao do vetor sera 1 caso o vertice i esteja na rota r, ou 0 caso contrario
	for (int k = 0; k < maxVeic; ++k)
	{
		rota = new short int[nCommodities+1];
		memset(rota, 0, (nCommodities+1) * sizeof(short int));
		B_qr[k].push_back(rota);
		v = rCons[k]->getVertices();
		tam = v.size()-1;
		for(int j = 1; j < tam; ++j)
		{
			B_qr[k][0][v[j]-nCommodities] = 1; //coloca-se 1 apenas naquelas posicoes em que o vertice esteja na rota
		}
		ptrRotasCons[k].push_back(rCons[k]);
		rCons[k]->incrNumApontadores();
	}
}

void ModeloCplex::exportModel(const char* arqExport){
	cplexPrimal.exportModel(arqExport);
}


float ModeloCplex::solveMaster(){
	cplexPrimal.solve();
	return cplexPrimal.getObjValue();
}


void ModeloCplex::initVarsPrimal(){

	lambdaPrimal = IloArray<IloNumVarArray>(env, maxVeic);
	for (int i = 0; i < maxVeic; i++)
	{
		lambdaPrimal[i] = IloNumVarArray(env, qRotasForn[i], 0, 1, ILOFLOAT);
		for (int j = 0; j < qRotasForn[i]; j++)
		{
			char nome[20];
			sprintf(nome, "lbd_%01d_%02d", i, j);
			lambdaPrimal[i][j].setName(nome);
		}
	}
	gammaPrimal = IloArray<IloNumVarArray>(env, maxVeic);
	for (int i = 0; i < maxVeic; i++)
	{
		gammaPrimal[i] = IloNumVarArray(env, qRotasCons[i], 0, 1, ILOFLOAT);
		for (int j = 0; j < qRotasCons[i]; j++)
		{
			char nome[20];
			sprintf(nome, "gam_%01d_%02d", i, j);
			gammaPrimal[i][j].setName(nome);
		}
	}
	tauPrimal = IloArray < IloArray < IloNumVarArray > > (env, maxVeic);
	for (int kForn = 0; kForn < maxVeic; kForn++)
	{
		tauPrimal[kForn] = IloArray < IloNumVarArray > (env, maxVeic);
		for (int kCons = 0; kCons < maxVeic; kCons++)
		{
			//commodity 0 sera desprezada
			tauPrimal[kForn][kCons] = IloNumVarArray(env, (nCommodities+1), 0, 1, ILOFLOAT);
			for (int i = 0; i <= nCommodities; i++)
			{
				char nome[20];
				sprintf(nome, "tau_%01d_%01d_%02d", kForn, kCons, i);
				tauPrimal[kForn][kCons][i].setName(nome);
			}
		}
	}
}


void ModeloCplex::setObjectiveFunctionPrimal(){
	IloExpr obj(env);
	
	//Coeficientes relacionados as variaveis lambdaPrimal
	for (int i = 0; i < maxVeic; ++i)
	{
		for (int j = 0; j < qRotasForn[i]; ++j)
		{
			obj += ptrRotasForn[i][j]->getCusto() * lambdaPrimal[i][j];
		}
	}
		
	//Coeficientes relacionados as variaveis gammaPrimal
	for (int i = 0; i < maxVeic; ++i)
	{
		for (int j = 0; j < qRotasCons[i]; ++j)
		{
			obj += ptrRotasCons[i][j]->getCusto() * gammaPrimal[i][j];
		}
	}

	//Coeficientes das trocas no Cross-Docking
	float custoTau;
	for (int kForn = 0; kForn < maxVeic; kForn++)
	{
		for (int kCons = 0; kCons < maxVeic; kCons++)
		{
			obj += MAIS_INFINITO * tauPrimal[kForn][kCons][0];
			if ( kForn == kCons ) custoTau = 0;
			else custoTau = custoTrocaCD;
			for (int i = 1; i <= nCommodities; i++)
			{
				obj += custoTau * tauPrimal[kForn][kCons][i];
			}
		}
	}

	//Cria o objeto para armazenar a funcao objetivo e atribui a expressao a ele
	costPrimal = IloObjective(env, obj);
	modelPrimal.add(costPrimal);
}


void ModeloCplex::setConstraintsPrimal1(){
	constraintsPrimal1 = IloRangeArray(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k)
	{
		IloExpr exp1 = IloExpr(env);
		for (int j = 0; j < qRotasForn[k]; ++j)
		{
			exp1 += lambdaPrimal[k][j];
		}
		//Insere a restrição no modelo
		constraintsPrimal1[k] = (exp1 == 1);
		char nome[20];
		sprintf(nome, "rest1_k%01d", k);
		constraintsPrimal1[k].setName(nome);
		modelPrimal.add(constraintsPrimal1[k]);
		exp1.end();	
	}
}


void ModeloCplex::setConstraintsPrimal2(){
	constraintsPrimal2 = IloRangeArray(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k)
	{
		IloExpr exp2 = IloExpr(env);
		for (int j = 0; j < qRotasCons[k]; ++j)
		{
			exp2 += gammaPrimal[k][j];
		}
		//Insere a restrição no modelo
		constraintsPrimal2[k] = (exp2 == 1);
		char nome[20];
		sprintf(nome, "rest2_k%01d", k);
		constraintsPrimal2[k].setName(nome);
		modelPrimal.add(constraintsPrimal2[k]);
		exp2.end();	
	}
}


void ModeloCplex::setConstraintsPrimal3(){
	constraintsPrimal3 = IloRangeArray(env, nCommodities);

	//Neste ponto, A_qr já pode ser usada para gerar as constraints
	for (int i = 1; i <= nCommodities; ++i)
	{
		IloExpr exp3 = IloExpr(env);
		for (int k = 0; k < maxVeic; ++k)
		{
			for (int l = 0; l < qRotasForn[k]; ++l)
			{
				if (A_qr[k][l][i] > 0)
				{
					exp3 += A_qr[k][l][i] * lambdaPrimal[k][l];
				}
			}
		}
		//Insere a restrição no modelo
		constraintsPrimal3[i-1] = (exp3 == 1);
		char nome[20];
		sprintf(nome, "rest3_i%02d", i);
		constraintsPrimal3[i-1].setName(nome);
		modelPrimal.add(constraintsPrimal3[i-1]);
		exp3.end();
	}
}


void ModeloCplex::setConstraintsPrimal4(){
	constraintsPrimal4 = IloRangeArray(env, nCommodities); //constraints4[0] eh desprezada

	//Neste ponto, B_qr já pode ser usada para gerar as constraints
	for (int i = 1; i <= nCommodities; ++i)
	{
		IloExpr exp4 = IloExpr(env);
		for (int k = 0; k < maxVeic; ++k)
		{
			for (int l = 0; l < qRotasCons[k]; ++l)
			{
				if (B_qr[k][l][i] > 0)
				{
					exp4 += B_qr[k][l][i] * gammaPrimal[k][l];
				}
			}
		}
		//Insere a restrição no modelo
		constraintsPrimal4[i-1] = (exp4 == 1);
		char nome[20];
		sprintf(nome, "rest4_i%02d", i);
		constraintsPrimal4[i-1].setName(nome);
		modelPrimal.add(constraintsPrimal4[i-1]);
		exp4.end();	
	}
}


void ModeloCplex::setConstraintsPrimal5(){
	constraintsPrimal5 = IloRangeArray(env, maxVeic * maxVeic * nCommodities);
	int count = 0;

	for (int kForn = 0; kForn < maxVeic; ++kForn)
	{
		for (int kCons = 0; kCons < maxVeic; ++kCons)
		{
			for (int i = 1; i <= nCommodities; ++i)
			{
				IloExpr exp5 = IloExpr(env);
				for (int j = 0; j < qRotasForn[kForn]; ++j)
				{
					if (A_qr[kForn][j][i] > 0)
					{
						exp5 += A_qr[kForn][j][i] * lambdaPrimal[kForn][j];
					}
				}
				for (int j = 0; j < qRotasCons[kCons]; ++j)
				{
					if (B_qr[kCons][j][i] > 0)
					{
						exp5 += B_qr[kCons][j][i] * gammaPrimal[kCons][j];
					}
				}
				exp5 -= tauPrimal[kForn][kCons][i];

				//Insere a restrição no modelo
				constraintsPrimal5[count] = (exp5 <= 1);
				char nome[20];
				sprintf(nome, "rest5_k%01d_k%01d_i%02d", kForn, kCons, i);
				constraintsPrimal5[count].setName(nome);
				modelPrimal.add(constraintsPrimal5[count]);
				exp5.end();
				++count;
			}
		}
	}
}

void ModeloCplex::setConstraintsPrimal6e7(){
	int count = 0;
	constraintsPrimal6 = IloRangeArray(env, nCommodities);

	for (int i = 1; i <= nCommodities; ++i)
	{
		IloExpr exp6 = IloExpr(env);
		for (int kForn = 0; kForn < maxVeic; ++kForn)
		{
			for (int kCons = 0; kCons < maxVeic; ++kCons)
			{
				exp6 += tauPrimal[kForn][kCons][i];
			}
		}
		constraintsPrimal6[i-1] = (exp6 == 1);
		char nome[20];
		sprintf(nome, "rest6_i%02d", i);
		constraintsPrimal6[i-1].setName(nome);
		modelPrimal.add(constraintsPrimal6[i-1]);
		exp6.end();
	}

	constraintsPrimal7 = IloRangeArray(env, maxVeic * nCommodities);
	for (int k = 0; k < maxVeic; ++k)
	{
		for (int i = 1; i <= nCommodities; ++i)
		{
			IloExpr exp7 = IloExpr(env);
			for (int kForn = 0; kForn < maxVeic; ++kForn)
			{
				if ( kForn != k )
				{
					for ( int r = 0; r < qRotasForn[kForn]; ++r )
					{
						if (A_qr[kForn][r][i] > 0)
						{
							exp7 += A_qr[kForn][r][i] * lambdaPrimal[kForn][r];
						}
					}
				}
			}

			for (int kCons = 0; kCons < maxVeic; ++kCons)
			{
				if ( kCons != k )
				{
					for ( int r = 0; r < qRotasCons[kCons]; ++r )
					{
						if (B_qr[kCons][r][i] > 0)
						{
							exp7 += B_qr[kCons][r][i] * gammaPrimal[kCons][r];
						}
					}
				}
			}
			exp7 += 2*tauPrimal[k][k][i];
			constraintsPrimal7[count] = (exp7 <= 2);
			char nome[20];
			sprintf(nome, "rest7_k%01d_i%02d", k, i);
			constraintsPrimal7[count].setName(nome);
			modelPrimal.add(constraintsPrimal7[count]);
			exp7.end();
			++count;
		}
	}
}


void ModeloCplex::updateDualCostsForn(Grafo* g, int k){
	float custoDual, somaXi, somaPsi;
	int ajuste = k*maxVeic*nCommodities;
	IloNumArray thetaDuals(env), xiDuals(env), psiDuals(env);
	cplexPrimal.getDuals(thetaDuals, constraintsPrimal3);
	cplexPrimal.getDuals(xiDuals, constraintsPrimal5);
	cplexPrimal.getDuals(psiDuals, constraintsPrimal7);

	for (int i = 0; i < nCommodities; ++i)
	{
		somaXi = somaPsi = 0;
		for ( int kCons = 0; kCons < maxVeic; ++kCons )
		{
			somaXi += xiDuals[ajuste + kCons*nCommodities + i];
			if ( kCons != k ) somaPsi += psiDuals[kCons*nCommodities + i];
		}
		custoDual = - thetaDuals[i] - somaXi - somaPsi;
		g->setCustoVerticeDual(i+1, custoDual);
	}
	thetaDuals.end(); xiDuals.end(); psiDuals.end();
	g->setCustoArestasDual('F');
}


void ModeloCplex::updateDualCostsCons(Grafo* g, int k){
	int ajuste = k*nCommodities;
	float custoDual, somaXi, somaPsi;
	IloNumArray muDuals(env), xiDuals(env), psiDuals(env);
	cplexPrimal.getDuals(muDuals, constraintsPrimal4);
	cplexPrimal.getDuals(xiDuals, constraintsPrimal5);
	cplexPrimal.getDuals(psiDuals, constraintsPrimal7);

	for (int i = 0; i < nCommodities; ++i)
	{
		somaXi = somaPsi = 0;
		for ( int kForn = 0; kForn < maxVeic; ++kForn )
		{
			somaXi += xiDuals[ajuste + kForn*maxVeic*nCommodities + i];
			if ( kForn != k ) somaPsi += psiDuals[kForn*nCommodities + i];
		}
		custoDual = - muDuals[i] - somaXi - somaPsi;
		g->setCustoVerticeDual(nCommodities+i+1, custoDual);
	}
	muDuals.end(); xiDuals.end(); psiDuals.end();
	g->setCustoArestasDual('C');
}


float ModeloCplex::getValorSubtrairForn( int k ){
	return cplexPrimal.getDual(constraintsPrimal1[k]);
}


float ModeloCplex::getValorSubtrairCons(int k){
	return cplexPrimal.getDual(constraintsPrimal2[k]);
}


void ModeloCplex::insertColumnPrimalForn(Rota* r, int k){
	//Armazena a rota na matriz de rotas, que sera usada posteriormente
	short int* rota = new short int[nCommodities+1];
	memset(rota, 0, (nCommodities+1) * sizeof(short int));
	A_qr[k].push_back(rota);
	ptrRotasForn[k].push_back(r);
	r->incrNumApontadores();
	
	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j)
	{
		++A_qr[k][qRotasForn[k]][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
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
	int ajuste = k*maxVeic*nCommodities;
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal1[k](1);
	for (int i = 0; i < nCommodities; i++)
	{
		if (vetVisitRota[i] > 0)
		{
			col += constraintsPrimal3[i](vetVisitRota[i]);
			for ( int kCons = 0; kCons < maxVeic; ++kCons )
			{
				col += constraintsPrimal5[ajuste + kCons*nCommodities + i](vetVisitRota[i]);
			}

			for ( int kForn = 0; kForn < maxVeic; ++kForn )
			{
				if ( kForn != k )
				{
					col += constraintsPrimal7[kForn*nCommodities + i](vetVisitRota[i]);
				}				
			}
		}
	}
	lambdaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRotasForn[k];
	delete [] vetVisitRota;
}


void ModeloCplex::insertColumnPrimalCons(Rota* r, int k){
	//Armazena a rota na matriz de rotas dos Consumidores, que sera usada posteriormente
	short int* rota = new short int[nCommodities+1];
	memset(rota, 0, (nCommodities+1) * sizeof(short int));
	B_qr[k].push_back(rota);
	ptrRotasCons[k].push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j)
	{
		++B_qr[k][qRotasCons[k]][vertRota[j]-nCommodities]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//inicializacao do vetor para armazenar o numero de visitas de cada vertice da coluna
	int* vetVisitRota = new int [nCommodities];
	memset (vetVisitRota, 0, nCommodities*sizeof(int));

	//coloca os indices (> 0) nas respectivas posicoes da coluna
	for (int i = 1; i <= numVertAtualizar; ++i) {
		++vetVisitRota[vertRota[i]-nCommodities-1];
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	int ajuste = k*nCommodities;
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal2[k](1);
	for (int i = 0; i < nCommodities; i++)
	{
		if (vetVisitRota[i] > 0)
		{
			col += constraintsPrimal4[i](vetVisitRota[i]);
			for ( int kForn = 0; kForn < maxVeic; ++kForn )
			{
				col += constraintsPrimal5[ajuste + kForn*maxVeic*nCommodities + i](vetVisitRota[i]);
			}

			for ( int kCons = 0; kCons < maxVeic; ++kCons )
			{
				if ( kCons != k )
				{
					col += constraintsPrimal7[kCons*nCommodities + i](vetVisitRota[i]);
				}				
			}
		}
	}
	gammaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRotasCons[k];
	delete [] vetVisitRota;
}


void ModeloCplex::setLimitePrimal(float lP){
	limitePrimal = lP;
}


float ModeloCplex::getLimitePrimal(){
	return limitePrimal;
}
