#include "ModeloCplex.h"
#include <sys/time.h>
using namespace std;

float ModeloCplex::limitePrimal;
Rota** retornaColuna(Grafo* G, int k, float valSub, char op, bool camForn, bool pricingFull, int& retornoPricing){
	Rota** r;	
	if (op == 'S')
	{
		StateSpaceRelax camElemSSR(G, camForn, valSub);
		retornoPricing = camElemSSR.calculaCaminhoElementarSSR(G, k);
		if (retornoPricing > 0)
		{
			r = camElemSSR.getRotaCustoMinimo(G, k, 0.85);
			if ( r == NULL ) retornoPricing = -1;
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
		retornoPricing = camElem.calculaCaminhoElementar(G, pricingFull, k);
		if (retornoPricing > 0)
		{
			r = camElem.getRotaCustoMinimo(G, k, 0.85);
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
}

Rota** retornaColuna(Grafo* G, int k, float valSub, char op, int tempo){
	Rota** r;
	if (op == 'P')
	{
		r = NULL;
		ElementarRD camElemRD(G, valSub);
		if (camElemRD.calculaCaminhoElementar(G, k, tempo) > 0) r = camElemRD.getRotaCustoMinimo(G, k, 0.85);
		return r;
	}
	else if (op == 'B') //primeiro tenta a heuristica de PD, caso nao encontre rota negativa, chama o BC
	{
		r = NULL;
		ElementarRDH camElemRDH(G, valSub);
		if (camElemRDH.calculaCaminhoElementar(G, k) > 0) r = camElemRDH.getRotaCustoMinimo(G, k, 0.85);
		if (r == NULL)
		{
			ModeloBC bcRD(G, k, valSub);
			bcRD.calculaCaminhoElementar(G, tempo);
			int aux = bcRD.rotasNegativas.size();
			if ( aux > 0 )
			{
				r = new Rota*[aux+1];
				for ( int i = 0; i < aux; ++i ) r[i] = bcRD.rotasNegativas[i];
				r[aux] = NULL;
			}
		}
		return r;
	}
}

ModeloCplex::~ModeloCplex(){
	for (int k = 0; k < maxVeic; ++k){
		for (int r = 0; r < qRotasForn[k]; ++r){
			delete [] A_qr[k][r];
			if (ptrRotasForn[k][r]->decrNumApontadores()) delete ptrRotasForn[k][r];
		}		
		for (int r = 0; r < qRotasCons[k]; ++r){
			delete [] B_qr[k][r];
			if (ptrRotasCons[k][r]->decrNumApontadores()) delete ptrRotasCons[k][r];
		}
		for (int r = 0; r < qRotasDiretas[k]; ++r){
			delete [] D_qr[k][r];
			if (ptrRotasDiretas[k][r]->decrNumApontadores()) delete ptrRotasDiretas[k][r];
		}

		A_qr[k].clear();
		B_qr[k].clear();
		D_qr[k].clear();
		ptrRotasForn[k].clear();
		ptrRotasCons[k].clear();
		ptrRotasDiretas[k].clear();
	}
	
	delete [] A_qr;
	delete [] B_qr;
	delete [] D_qr;
	delete [] ptrRotasForn;
	delete [] ptrRotasCons;
	delete [] ptrRotasDiretas;
	delete [] qRotasForn;
	delete [] qRotasCons;
	delete [] qRotasDiretas;
	env.end();
}

ModeloCplex::ModeloCplex(Grafo* g, Rota** rForn, Rota** rCons, int nRotasF, int nRotasC, int maxVeiculos, int numCommodities, int custoTroca)
: env(), modelPrimal(env), cplexPrimal(modelPrimal), maxVeic(maxVeiculos), nCommodities(numCommodities), custoTrocaCD(custoTroca) {

	qRotasForn = new int[maxVeic];
	qRotasCons = new int[maxVeic];
	qRotasDiretas = new int [maxVeic];
	for (int k = 0; k < maxVeic; ++k)
	{
		//Cada veiculo eh associado a uma rota para fornecedores e consumidores (nenhuma rota direta na inicializacao)
		qRotasForn[k] = 1;
		qRotasCons[k] = 1;
		qRotasDiretas[k] = 0;
	}

	cplexPrimal.setOut(env.getNullStream());

	//armazena como atributos da classe as matrizes A_qr, B_qr e D_qr, alem das matrizes de ponteiros para rotas
	setA_qr(rForn);
	setB_qr(rCons);
	D_qr = new vector<short int*>[maxVeic];
	ptrRotasDiretas = new vector<Rota*>[maxVeic];

	//Montagem do modelo Primal
	initVarsPrimal();
	setObjectiveFunctionPrimal();
	setConstraintsPrimal1();
	setConstraintsPrimal2();
	setConstraintsPrimal3();
	setConstraintsPrimal4();
	setConstraintsPrimal5();
	setConstraintsPrimal6();
}


//Este construtor eh necessario para construir um modelo para realizar um branching de variaveis
//Neste modelo, serao inseridas todas as colunas encontradas (as iniciais encontradas para se alcançar 
//a raiz do master, e aquelas inseridas em um no que terminou o primeiro branching sem solucoes inteiras)
//Alem de todas estas colunas, as restricoes de arvore (do primeiro branching) tambem sao inseridas
ModeloCplex::ModeloCplex(ModeloCplex& modeloCopia, vector<short int*>* matrizA_no, vector<short int*>* matrizB_no, vector<short int*>* matrizD_no,
					 vector<Rota*>* ptrForn_no, vector<Rota*>* ptrCons_no, vector<Rota*>* ptrDirect_no, char** varComRestr) : 
					 env(), modelPrimal(env), cplexPrimal(modelPrimal) {

	cplexPrimal.setOut(env.getNullStream());
	maxVeic = modeloCopia.maxVeic;
	nCommodities = modeloCopia.nCommodities;
	custoTrocaCD = modeloCopia.custoTrocaCD;
	
	short int* rota;
	qRotasForn = new int[maxVeic];
	qRotasCons = new int[maxVeic];
	qRotasDiretas = new int[maxVeic];
	A_qr = new vector<short int*>[maxVeic];
	B_qr = new vector<short int*>[maxVeic];
	D_qr = new vector<short int*>[maxVeic];
	ptrRotasForn = new vector<Rota*>[maxVeic];
	ptrRotasCons = new vector<Rota*>[maxVeic];
	ptrRotasDiretas = new vector<Rota*>[maxVeic];

	for (int k = 0; k < maxVeic; ++k)
	{
		qRotasForn[k] = modeloCopia.qRotasForn[k];
		qRotasCons[k] = modeloCopia.qRotasCons[k];
		qRotasDiretas[k] = modeloCopia.qRotasDiretas[k];
		for (int r = 0; r < qRotasForn[k]; ++r)
		{
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i)
			{
				rota[i] = modeloCopia.A_qr[k][r][i];
			}
			A_qr[k].push_back(rota);
			ptrRotasForn[k].push_back(modeloCopia.ptrRotasForn[k][r]);
			ptrRotasForn[k][r]->incrNumApontadores();
		}
		for (int r = 0; r < matrizA_no[k].size(); ++r)
		{
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i)
			{
				rota[i] = matrizA_no[k][r][i];
			}
			A_qr[k].push_back(rota);
			ptrRotasForn[k].push_back(ptrForn_no[k][r]);
			ptrRotasForn[k][qRotasForn[k]]->incrNumApontadores();
			++qRotasForn[k];
		}

		for (int r = 0; r < qRotasCons[k]; ++r)
		{
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i)
			{
				rota[i] = modeloCopia.B_qr[k][r][i];
			}
			B_qr[k].push_back(rota);	
			ptrRotasCons[k].push_back(modeloCopia.ptrRotasCons[k][r]);
			ptrRotasCons[k][r]->incrNumApontadores();
		}
		for (int r = 0; r < matrizB_no[k].size(); ++r)
		{
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i)
			{
				rota[i] = matrizB_no[k][r][i];
			}
			B_qr[k].push_back(rota);
			ptrRotasCons[k].push_back(ptrCons_no[k][r]);
			ptrRotasCons[k][qRotasCons[k]]->incrNumApontadores();
			++qRotasCons[k];
		}

		int nVertices = 2*nCommodities;
		for (int r = 0; r < qRotasDiretas[k]; ++r)
		{
			rota = new short int[nVertices+1];
			for (int i = 1; i <= nVertices; ++i)
			{
				rota[i] = modeloCopia.D_qr[k][r][i];
			}
			D_qr[k].push_back(rota);	
			ptrRotasDiretas[k].push_back(modeloCopia.ptrRotasDiretas[k][r]);
			ptrRotasDiretas[k][r]->incrNumApontadores();
		}
		for (int r = 0; r < matrizD_no[k].size(); ++r)
		{
			rota = new short int[nVertices+1];
			for (int i = 1; i <= nVertices; ++i)
			{
				rota[i] = matrizD_no[k][r][i];
			}
			D_qr[k].push_back(rota);
			ptrRotasDiretas[k].push_back(ptrDirect_no[k][r]);
			ptrRotasDiretas[k][qRotasDiretas[k]]->incrNumApontadores();
			++qRotasDiretas[k];
		}
	}
	
	//Constroi o modelo, exceto as restricoes da arvore
	initVarsPrimal();
	setObjectiveFunctionPrimal();
	setConstraintsPrimal1();
	setConstraintsPrimal2();
	setConstraintsPrimal3();
	setConstraintsPrimal4();
	setConstraintsPrimal5();
	setConstraintsPrimal6();

	//inclui as restricoes de branching
	//Nao precisa de variaveis artificiais, pois se chegou neste ponto, 
	//significa que estas restricoes sao possiveis de serem atendidas 
	IloExpr exp0(env);
	IloExpr exp1(env);
	int numRestrIgual1 = 0;
	for (int k = 0; k < maxVeic; ++k)
	{
		for (int i = 1; i <= nCommodities; ++i)
		{
			if ((varComRestr[k][i] == 1) || (varComRestr[k][i] == 2))
			{
				exp1 += tauPrimal[k][i];
				++numRestrIgual1;
			}
			else if (varComRestr[k][i] > 2)
			{
				exp0 += tauPrimal[k][i];
			}
		}
	}
	constraintArvoreTau0 = (exp0 == 0);
	constraintArvoreTau1 = (exp1 == numRestrIgual1);
	modelPrimal.add(constraintArvoreTau0);
	modelPrimal.add(constraintArvoreTau1);
	exp0.end(); exp1.end();
	
	//RESTRICOES QUE FIXAM AS VARIAVEIS LAMBDA, GAMMA e DELTA PARA ALGUNS NOS (que tiverem variaveisComRestricao[k][i] > 0)
	constraintsArvoreLambda = IloRangeArray(env);
	constraintsArvoreGamma = IloRangeArray(env);
	constraintsArvoreDelta = IloRangeArray(env);
	int numRestrArvore = 0;
	for (int k = 0; k < maxVeic; ++k)
	{
		for (int i = 1; i <= nCommodities; ++i)
		{
			if (varComRestr[k][i] > 0) 
			{
				//Insere a restricao fixando LAMBDA
				IloExpr expL(env);
				for (int r = 0; r < qRotasForn[k]; ++r)
				{
					if (A_qr[k][r][i] > 0)
					{
						expL += A_qr[k][r][i] * lambdaPrimal[k][r];
					}
				}
				if ((varComRestr[k][i] == 1) || (varComRestr[k][i] == 3))
				{
					constraintsArvoreLambda.add(expL == 1);
				}
				else
				{
					constraintsArvoreLambda.add(expL == 0);
				}
				modelPrimal.add(constraintsArvoreLambda[numRestrArvore]);

				//Insere a restricao fixando GAMMA
				IloExpr expG(env);
				for (int r = 0; r < qRotasCons[k]; ++r)
				{
					if (B_qr[k][r][i] > 0)
					{
						expG += B_qr[k][r][i] * gammaPrimal[k][r];
					}
				}
				if ((varComRestr[k][i] == 2) || (varComRestr[k][i] == 3))
				{
					constraintsArvoreGamma.add(expG == 1);
				}
				else
				{
					constraintsArvoreGamma.add(expG == 0);
				}
				modelPrimal.add(constraintsArvoreGamma[numRestrArvore]);

				//Insere a restricao fixando DELTA
				IloExpr expD(env);
				for (int r = 0; r < qRotasDiretas[k]; ++r)
				{
					if (D_qr[k][r][i] > 0)
					{
						expG += D_qr[k][r][i] * deltaPrimal[k][r];
					}
				}
				if (varComRestr[k][i] == 5) 
				{
					constraintsArvoreDelta.add(expD == 1);
				}
				else
				{
					constraintsArvoreDelta.add(expD == 0);
				}
				modelPrimal.add(constraintsArvoreDelta[numRestrArvore]);

				expL.end(); expG.end(); expD.end();
				++numRestrArvore;
			}
		}
	}
}


void ModeloCplex::exportModel(const char* arqExport){
	cplexPrimal.exportModel(arqExport);
}

float ModeloCplex::solveMaster(){
	cplexPrimal.solve();
	return cplexPrimal.getObjValue();
}

float ModeloCplex::getAlfaDual(int k){
	return cplexPrimal.getDual(constraintsPrimal1[k]);
}

float ModeloCplex::getBetaDual(int k){
	return cplexPrimal.getDual(constraintsPrimal2[k]);
}

void ModeloCplex::updateDualCostsForn(Grafo* g, int k){
	float custoDual;
	int saltoK = k*nCommodities;
	IloNumArray dualValuesP3(env), dualValuesP5(env), dualValuesP6(env);
	cplexPrimal.getDuals(dualValuesP3, constraintsPrimal3);
	cplexPrimal.getDuals(dualValuesP5, constraintsPrimal5);
	cplexPrimal.getDuals(dualValuesP6, constraintsPrimal6);

	for (int i = 0; i < nCommodities; ++i)
	{
		custoDual = -dualValuesP3[i] - dualValuesP5[saltoK + i] + dualValuesP6[saltoK + i];
		g->setCustoVerticeDual(i+1, custoDual);
	}
	g->setCustoArestasDual(k);
}


void ModeloCplex::updateDualCostsCons(Grafo* g, int k){
	float custoDual;
	int saltoK = k*nCommodities;
	IloNumArray dualValuesP4(env), dualValuesP5(env), dualValuesP6(env);
	cplexPrimal.getDuals(dualValuesP4, constraintsPrimal4);
	cplexPrimal.getDuals(dualValuesP5, constraintsPrimal5);
	cplexPrimal.getDuals(dualValuesP6, constraintsPrimal6);

	for (int i = 0; i < nCommodities; ++i)
	{
		custoDual = -dualValuesP4[i] + dualValuesP5[saltoK + i] - dualValuesP6[saltoK + i];
		g->setCustoVerticeDual(i+nCommodities+1, custoDual);
	}
	g->setCustoArestasDual(k);
}

void ModeloCplex::updateDualCostsDirect(Grafo* g, int k){
	IloNumArray dualValuesP3(env), dualValuesP4(env);
	cplexPrimal.getDuals(dualValuesP3, constraintsPrimal3);
	cplexPrimal.getDuals(dualValuesP4, constraintsPrimal4);

	for (int i = 0; i < nCommodities; ++i)
	{
		g->setCustoVerticeDual(i+1, -dualValuesP3[i]-dualValuesP4[i]);
		g->setCustoVerticeDual(i+nCommodities+1, 0);
	}
	g->setCustoArestasDual(k);
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
		for(int j = 1; j < tam; ++j){
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
		for(int j = 1; j < tam; ++j){
			B_qr[k][0][v[j]-nCommodities] = 1; //coloca-se 1 apenas naquelas posicoes em que o vertice esteja na rota
		}
		ptrRotasCons[k].push_back(rCons[k]);
		rCons[k]->incrNumApontadores();
	}
}

void ModeloCplex::insert_ColumnPrimal_Forn(Rota* r, int k){
	//Armazena a rota na matriz de rotas, que sera usada posteriormente
	short int* rota = new short int[nCommodities+1];
	memset(rota, 0, (nCommodities+1) * sizeof(short int));
	A_qr[k].push_back(rota);
	ptrRotasForn[k].push_back(r);
	r->incrNumApontadores();
	
	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j){
		++A_qr[k][qRotasForn[k]][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal1[k](1);
	for (int i = 1; i <= nCommodities; i++){
		if (A_qr[k][qRotasForn[k]][i] > 0){
			col += constraintsPrimal3[i - 1](A_qr[k][qRotasForn[k]][i]);
			col += constraintsPrimal5[k*nCommodities + i - 1](A_qr[k][qRotasForn[k]][i]);
			col += constraintsPrimal6[k*nCommodities + i - 1](-A_qr[k][qRotasForn[k]][i]);
		}
	}
	char nomeF[20];
	sprintf(nomeF, "lam_%01d_%02d", k, qRotasForn[k]);
	lambdaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT, nomeF));
	col.end();

	++qRotasForn[k];
}

void ModeloCplex::insert_ColumnPrimal_Cons(Rota* r, int k){
	//Armazena a rota na matriz de rotas dos Consumidores, que sera usada posteriormente
	short int* rota = new short int[nCommodities+1];
	memset(rota, 0, (nCommodities+1) * sizeof(short int));
	B_qr[k].push_back(rota);
	ptrRotasCons[k].push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j){
		++B_qr[k][qRotasCons[k]][vertRota[j]-nCommodities]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal2[k](1);
	for (int i = 1; i <= nCommodities; i++){
		if (B_qr[k][qRotasCons[k]][i] > 0){
			col += constraintsPrimal4[i-1](B_qr[k][qRotasCons[k]][i]);
			col += constraintsPrimal5[k*nCommodities + i - 1](-B_qr[k][qRotasCons[k]][i]);
			col += constraintsPrimal6[k*nCommodities + i - 1](B_qr[k][qRotasCons[k]][i]);
		}
	}
	char nomeC[20];
	sprintf(nomeC, "gam_%01d_%02d", k, qRotasCons[k]);
	gammaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT, nomeC));
	col.end();

	++qRotasCons[k];
}


void ModeloCplex::insert_ColumnPrimal_Direct(Rota* r, int k){
	//Armazena a rota na matriz de rotas, que sera usada posteriormente
	int nVertices = 2*nCommodities+1;
	short int* rota = new short int[nVertices];
	memset(rota, 0, nVertices * sizeof(short int));
	D_qr[k].push_back(rota);
	ptrRotasDiretas[k].push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j)
	{
		++D_qr[k][qRotasDiretas[k]][vertRota[j]]; //+1 apenas nas posicoes cujo vertice esteja na rota
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal1[k](1);
	col += constraintsPrimal2[k](1);
	for (int i = 1; i <= nCommodities; i++)
	{
		if ( D_qr[k][qRotasDiretas[k]][i] > 0 )
		{
			col += constraintsPrimal3[i-1](D_qr[k][qRotasDiretas[k]][i]);
			col += constraintsPrimal4[i-1](D_qr[k][qRotasDiretas[k]][i]);
		}
	}
	char nomeRD[20];
	sprintf(nomeRD, "del_%01d_%02d", k, qRotasDiretas[k]);
	deltaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT, nomeRD));
	col.end();

	++qRotasDiretas[k];
}


void ModeloCplex::initVarsPrimal(){

	lambdaPrimal = IloArray<IloNumVarArray>(env, maxVeic); //variaveis de rotas dos fornecedores
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
	gammaPrimal = IloArray<IloNumVarArray>(env, maxVeic); //variaveis de rotas dos consumidores
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
	deltaPrimal = IloArray<IloNumVarArray>(env, maxVeic); //variaveis de rotas diretas
	for (int i = 0; i < maxVeic; i++)
	{
		deltaPrimal[i] = IloNumVarArray(env, qRotasDiretas[i], 0, 1, ILOFLOAT);
		for (int j = 0; j < qRotasDiretas[i]; j++)
		{
			char nome[20];
			sprintf(nome, "del_%01d_%02d", i, j);
			deltaPrimal[i][j].setName(nome);
		}
	}

	tauPrimal = IloArray<IloNumVarArray>(env, maxVeic); //variaveis de troca no CD
	for (int i = 0; i < maxVeic; i++){
		//cria-se uma variavel tau a mais para cada veiculo, pois a commodity 0 eh desprezada
		tauPrimal[i] = IloNumVarArray(env, (nCommodities+1), 0, 1, ILOFLOAT);
		for (int j = 0; j <= nCommodities; j++){
			char nome[20];
			sprintf(nome, "tau_%01d_%02d", i, j);
			tauPrimal[i][j].setName(nome);
		}
	}
}

void ModeloCplex::setObjectiveFunctionPrimal(){
	IloExpr obj(env);
	
	//lambdaPrimal
	for (int i = 0; i < maxVeic; ++i)
	{
		for (int j = 0; j < qRotasForn[i]; ++j)
		{
			obj += ptrRotasForn[i][j]->getCusto() * lambdaPrimal[i][j];
		}
	}
		
	//gammaPrimal
	for (int i = 0; i < maxVeic; ++i)
	{
		for (int j = 0; j < qRotasCons[i]; ++j)
		{
			obj += ptrRotasCons[i][j]->getCusto() * gammaPrimal[i][j];
		}
	}
	
	//deltaPrimal
	for (int i = 0; i < maxVeic; ++i)
	{
		for (int j = 0; j < qRotasDiretas[i]; ++j)
		{
			obj += ptrRotasDiretas[i][j]->getCusto() * deltaPrimal[i][j];
		}
	}

	//tauPrimal
	for (int i = 0; i < maxVeic; ++i)
	{
		for (int j = 1; j <= nCommodities; ++j)
		{
			obj += custoTrocaCD * tauPrimal[i][j];
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
		for (int j = 0; j < qRotasDiretas[k]; ++j)
		{
			exp1 += deltaPrimal[k][j];
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
		for (int j = 0; j < qRotasDiretas[k]; ++j)
		{
			exp2 += deltaPrimal[k][j];
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
			for (int l = 0; l < qRotasDiretas[k]; ++l)
			{
				if (D_qr[k][l][i] > 0)
				{
					exp3 += D_qr[k][l][i] * deltaPrimal[k][l];
				}
			}
		}
		//Insere a restrição no modelo
		constraintsPrimal3[i-1] = (exp3 == 1);
		char nome[20];
		sprintf(nome, "rest3_q%02d", i);
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
			for (int l = 0; l < qRotasDiretas[k]; ++l)
			{
				if (D_qr[k][l][nCommodities+i] > 0)
				{
					exp4 += D_qr[k][l][nCommodities+i] * deltaPrimal[k][l];
				}
			}
		}
		//Insere a restrição no modelo
		constraintsPrimal4[i-1] = (exp4 == 1);
		char nome[20];
		sprintf(nome, "rest4_q%02d", i);
		constraintsPrimal4[i-1].setName(nome);
		modelPrimal.add(constraintsPrimal4[i-1]);
		exp4.end();	
	}
}


void ModeloCplex::setConstraintsPrimal5(){
	constraintsPrimal5 = IloRangeArray(env, maxVeic * nCommodities);

	for (int k = 0; k < maxVeic; ++k)
	{
		for (int i = 1; i <= nCommodities; ++i)
		{
			IloExpr exp5 = IloExpr(env);
			for (int j = 0; j < qRotasForn[k]; ++j)
			{
				if (A_qr[k][j][i] > 0)
				{
					exp5 += A_qr[k][j][i] * lambdaPrimal[k][j];
				}
			}
			for (int j = 0; j < qRotasCons[k]; ++j)
			{
				if (B_qr[k][j][i] > 0)
				{
					exp5 -= B_qr[k][j][i] * gammaPrimal[k][j];
				}
			}
			exp5 += tauPrimal[k][i];

			//Insere a restrição no modelo
			constraintsPrimal5[k*nCommodities + (i-1)] = (exp5 >= 0);
			char nome[20];
			sprintf(nome, "rest5_k%01d_q%02d", k, i);
			constraintsPrimal5[k*nCommodities + (i-1)].setName(nome);
			modelPrimal.add(constraintsPrimal5[k*nCommodities + (i-1)]);
			exp5.end();	
		}
	}
}

void ModeloCplex::setConstraintsPrimal6(){
	constraintsPrimal6 = IloRangeArray(env, maxVeic * nCommodities);

	for (int k = 0; k < maxVeic; ++k)
	{
		for (int i = 1; i <= nCommodities; ++i)
		{
			IloExpr exp6 = IloExpr(env);
			for (int j = 0; j < qRotasForn[k]; ++j)
			{
				if (A_qr[k][j][i] > 0)
				{
					exp6 -= A_qr[k][j][i] * lambdaPrimal[k][j];
				}
			}
			for (int j = 0; j < qRotasCons[k]; ++j)
			{
				if (B_qr[k][j][i] > 0)
				{
					exp6 += B_qr[k][j][i] * gammaPrimal[k][j];
				}
			}
			exp6 += tauPrimal[k][i];

			//Insere a restrição no modelo
			constraintsPrimal6[k*nCommodities + (i-1)] = (exp6 >= 0);
			char nome[20];
			sprintf(nome, "rest6_k%01d_q%02d", k, i);
			constraintsPrimal6[k*nCommodities + (i-1)].setName(nome);
			modelPrimal.add(constraintsPrimal6[k*nCommodities + (i-1)]);
			exp6.end();	
		}	
	}
}

void ModeloCplex::limpaVariaveisArvoreB1(){
	//limpa variaveis dos modelos primal e dual	
	for (int k = 0; k < maxVeic; ++k)
	{
		lambdaNo[k].endElements();
		gammaNo[k].endElements();
		deltaNo[k].endElements();
	}

	artificiais.endElements();
	constraintArvoreTau0.end();
	constraintArvoreTau1.end();
	constraintsArvoreLambda.endElements();
	constraintsArvoreGamma.endElements();
	constraintsArvoreDelta.endElements();
}

void ModeloCplex::setLimitePrimal(float lP){
	limitePrimal = lP;
}

float ModeloCplex::getLimitePrimal(){
	return limitePrimal;
}

void ModeloCplex::atualizaRotasBasicas(){
	IloNumArray valuesF(env);
	IloNumArray valuesC(env);
	IloNumArray valuesD(env);
	for (int k = 0; k < maxVeic; ++k)
	{
		cplexPrimal.getValues(valuesF, lambdaPrimal[k]);
		for (int r = 0; r < qRotasForn[k]; ++r)
		{
			if (valuesF[r] >= 0.00001) ptrRotasForn[k][r]->setBasica();
		}

		cplexPrimal.getValues(valuesC, gammaPrimal[k]);
		for (int r = 0; r < qRotasCons[k]; ++r)
		{
			if (valuesC[r] >= 0.00001) ptrRotasCons[k][r]->setBasica();
		}

		cplexPrimal.getValues(valuesD, deltaPrimal[k]);
		for (int r = 0; r < qRotasDiretas[k]; ++r)
		{
			if (valuesD[r] >= 0.00001) ptrRotasDiretas[k][r]->setBasica();
		}
	}
}

void ModeloCplex::imprimeRotasBasicas(){
	IloNumArray valuesF(env);
	IloNumArray valuesC(env);
	IloNumArray valuesD(env);
	for (int k = 0; k < maxVeic; ++k)
	{
		cplexPrimal.getValues(valuesF, lambdaPrimal[k]);
		for (int r = 0; r < qRotasForn[k]; ++r)
		{
			if (valuesF[r] >= 0.00001) ptrRotasForn[k][r]->imprimir();
		}

		cplexPrimal.getValues(valuesC, gammaPrimal[k]);
		for (int r = 0; r < qRotasCons[k]; ++r)
		{
			if (valuesC[r] >= 0.00001) ptrRotasCons[k][r]->imprimir();
		}

		cplexPrimal.getValues(valuesD, deltaPrimal[k]);
		for (int r = 0; r < qRotasDiretas[k]; ++r)
		{
			if (valuesD[r] >= 0.00001) ptrRotasDiretas[k][r]->imprimir();
		}
	}
}
