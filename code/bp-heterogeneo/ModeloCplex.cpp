#include "ModeloCplex.h"
#include <sys/time.h>
using namespace std;

float ModeloCplex::limitePrimal;
Rota** retornaColuna(Grafo* G, int k, bool camForn, bool pricingFull, float valSub, char op, int& retornoPricing){
	Rota** r;
	if (op == 'S'){
		StateSpaceRelax camElemSSR(G, camForn, valSub);
		retornoPricing = camElemSSR.calculaCaminhoElementarSSR(G, k);
		if (retornoPricing > 0){
			r = camElemSSR.getRotaCustoMinimo(G, k, 0.85);
			if ( r == NULL ) retornoPricing = -1;
		}else{
			r = NULL;
		}
		return r;

	}else if (op == 'E'){
		Elementar camElem(G, camForn, valSub);
		retornoPricing = camElem.calculaCaminhoElementar(G, pricingFull, k);
		if (retornoPricing > 0){
			r = camElem.getRotaCustoMinimo(G, k, 0.85);
			if ( r == NULL ) 
			{
				if ( pricingFull ) retornoPricing = -1;
				else retornoPricing = 0;
			}
		}else{
			r = NULL;
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
	delete [] pontoInterior;
	env.end();
}

ModeloCplex::ModeloCplex(Grafo* g, Rota** rForn, Rota** rCons, int nRotasF, int nRotasC, int maxVeiculos, int numCommodities, int custoTroca)
: env(), modelPrimal(env), cplexPrimal(modelPrimal),  modelDual(env), cplexDual(modelDual), costDual(env),
  maxVeic(maxVeiculos), nCommodities(numCommodities), custoTrocaCD(custoTroca) {
	//Devido a alteracao na insercao das colunas (que agora serao inseridas apenas para o veiculo que a encontrou)
	//a quantidade de rotas de cada veiculo tanto nos consumidores quanto fornecedores poderá ser diferente,
	//sendo necessario que cada posicao dos vetores abaixo armazene a quantidade associada ao respectivo veiculo
	//Na inicializacao, todas as rotas vindas da heuristica sao associadas a todos os veiculos
	qRotasForn = new int[maxVeic];
	qRotasCons = new int[maxVeic];
	for (int k = 0; k < maxVeic; ++k){
		qRotasForn[k] = 1; //cada veiculo eh inicializado com apenas aquela rota gerada em geraRotas
		qRotasCons[k] = 1;
	}

	//Nao precisa armazenar as variaveis OMEGA, DELTA, EPSILON e GAMMA no ponto interior
	//pois elas nao sao utilizadas para atualizar os custos duais do subproblema
	tamPontoInterior = (2*maxVeic) + (2*nCommodities) + (2*maxVeic*nCommodities);
	pontoInterior = new IloNum[tamPontoInterior];

	cplexPrimal.setOut(env.getNullStream());
	cplexDual.setOut(env.getNullStream());

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
	setConstraintsPrimal6();
	
	//Montagem do modelo Dual
	initVarsDual();
	setConstraintsDual1();
	setConstraintsDual2();
	setConstraintsDual3();
	modelDual.add(costDual);
}

//Este construtor eh necessario para construir um modelo para realizar um branching de variaveis
//Neste modelo, serao inseridas todas as colunas encontradas (as iniciais encontradas para se alcançar 
//a raiz do master, e aquelas inseridas em um no que terminou o primeiro branching sem solucoes inteiras)
//Alem de todas estas colunas, as restricoes de arvore (do primeiro branching) tambem sao inseridas
ModeloCplex::ModeloCplex(ModeloCplex& modeloCopia, vector<short int*>* matrizA_no, vector<short int*>* matrizB_no,
					 vector<Rota*>* ptrForn_no, vector<Rota*>* ptrCons_no, char** varComRestr) : 
					 env(), modelPrimal(env), cplexPrimal(modelPrimal),  modelDual(env), cplexDual(modelDual), costDual(env){

	cplexPrimal.setOut(env.getNullStream());
	maxVeic = modeloCopia.maxVeic;
	nCommodities = modeloCopia.nCommodities;
	custoTrocaCD = modeloCopia.custoTrocaCD;
	pontoInterior = new IloNum[1]; //APENAS PARA ELIMINAR A FALHA AO DESALOCAR ESTE PONTEIRO
	
	short int* rota;
	qRotasForn = new int[maxVeic];
	qRotasCons = new int[maxVeic];
	A_qr = new vector<short int*>[maxVeic];
	B_qr = new vector<short int*>[maxVeic];
	ptrRotasForn = new vector<Rota*>[maxVeic];
	ptrRotasCons = new vector<Rota*>[maxVeic];

	for (int k = 0; k < maxVeic; ++k){
		qRotasForn[k] = modeloCopia.qRotasForn[k];
		qRotasCons[k] = modeloCopia.qRotasCons[k];
		for (int r = 0; r < qRotasForn[k]; ++r){
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i){
				rota[i] = modeloCopia.A_qr[k][r][i];
			}
			A_qr[k].push_back(rota);
			ptrRotasForn[k].push_back(modeloCopia.ptrRotasForn[k][r]);
			ptrRotasForn[k][r]->incrNumApontadores();
		}
		for (int r = 0; r < matrizA_no[k].size(); ++r){
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i){
				rota[i] = matrizA_no[k][r][i];
			}
			A_qr[k].push_back(rota);
			ptrRotasForn[k].push_back(ptrForn_no[k][r]);
			ptrRotasForn[k][qRotasForn[k]]->incrNumApontadores();
			++qRotasForn[k];
		}
		for (int r = 0; r < qRotasCons[k]; ++r){
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i){
				rota[i] = modeloCopia.B_qr[k][r][i];
			}
			B_qr[k].push_back(rota);	
			ptrRotasCons[k].push_back(modeloCopia.ptrRotasCons[k][r]);
			ptrRotasCons[k][r]->incrNumApontadores();
		}
		for (int r = 0; r < matrizB_no[k].size(); ++r){
			rota = new short int[nCommodities+1];
			for (int i = 1; i <= nCommodities; ++i){
				rota[i] = matrizB_no[k][r][i];
			}
			B_qr[k].push_back(rota);
			ptrRotasCons[k].push_back(ptrCons_no[k][r]);
			ptrRotasCons[k][qRotasCons[k]]->incrNumApontadores();
			++qRotasCons[k];
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

	//inclui as restricoes da arvore do primeiro branching
	//Nao precisa de variaveis artificiais, pois se chegou neste ponto, 
	//significa que estas restricoes sao possiveis de serem atendidas 
	int numRestrIgual1 = 0;
	IloExpr exp0 = IloExpr(env);
	IloExpr exp1 = IloExpr(env);
	for (int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			if ((varComRestr[k][i] == 1) || (varComRestr[k][i] == 2)){
				exp0 += tauPrimal[k][i];
			}else if ((varComRestr[k][i] == 3) || (varComRestr[k][i] == 4)){
				exp1 += tauPrimal[k][i];
				++numRestrIgual1;
			}
		}
	}
	constraintArvoreTau0 = (exp0 == 0);
	constraintArvoreTau1 = (exp1 == numRestrIgual1);
	modelPrimal.add(constraintArvoreTau0);
	modelPrimal.add(constraintArvoreTau1);
	exp0.end();
	exp1.end();
	
	//RESTRICOES QUE FIXAM AS VARIAVEIS LAMBDA e GAMMA PARA ALGUNS NOS (que tiverem variaveisComRestricao[k][i] > 0)
	constraintsArvoreLambda = IloRangeArray(env);
	constraintsArvoreGamma = IloRangeArray(env);
	int numRestrLambda = 0, numRestrGamma = 0;
	for (int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			if (varComRestr[k][i] > 0){ //Insere a restricao fixando lambda
				IloExpr expL = IloExpr(env);
				for (int r = 0; r < qRotasForn[k]; ++r){
					if (A_qr[k][r][i] > 0){
						expL += A_qr[k][r][i] * lambdaPrimal[k][r];
					}
				}
				if ((varComRestr[k][i] == 1) || (varComRestr[k][i] == 3)){
					constraintsArvoreLambda.add(expL == 1);
				}else{
					constraintsArvoreLambda.add(expL == 0);
				}
				modelPrimal.add(constraintsArvoreLambda[numRestrLambda]);
				++numRestrLambda;
				expL.end();
			}			
			if (varComRestr[k][i] > 0){ //Insere a restricao fixando gamma
				IloExpr expG = IloExpr(env);
				for (int r = 0; r < qRotasCons[k]; ++r){
					if (B_qr[k][r][i] > 0){
						expG += B_qr[k][r][i] * gammaPrimal[k][r];
					}
				}
				if ((varComRestr[k][i] == 1) || (varComRestr[k][i] == 4)){
					constraintsArvoreGamma.add(expG == 1);
				}else{
					constraintsArvoreGamma.add(expG == 0);
				}
				modelPrimal.add(constraintsArvoreGamma[numRestrGamma]);
				++numRestrGamma;
				expG.end();
			}
		}
	}
}

void ModeloCplex::exportModel(const char* arqExport, char opcao){
	if (opcao == 'P'){
		cplexPrimal.exportModel(arqExport);
	}else{
		cplexDual.exportModel(arqExport);
	}
}

float ModeloCplex::solveMaster(){
	cplexPrimal.solve();
	return cplexPrimal.getObjValue();
}

void ModeloCplex::updateDualCostsForn(Grafo* g, int k){
	float custoDual;
	int saltoK = k*nCommodities;
	int inicioTHETA = 2*maxVeic;
	int inicioPI = inicioTHETA + 2*nCommodities;
	int inicioXI = inicioPI + nCommodities*maxVeic;

	for (int i = 0; i < nCommodities; ++i){
		custoDual = -pontoInterior[inicioTHETA + i] - pontoInterior[inicioPI + saltoK + i] + pontoInterior[inicioXI + saltoK + i];
		g->setCustoVerticeDual(i+1, custoDual);
	}
	g->setCustoArestasDual(k, 'F');
}


void ModeloCplex::updateDualCostsCons(Grafo* g, int k){
	float custoDual;
	int saltoK = k*nCommodities;
	int inicioMU = 2*maxVeic + nCommodities;
	int inicioPI = inicioMU + nCommodities;
	int inicioXI = inicioPI + nCommodities*maxVeic;

	for (int i = 0; i < nCommodities; ++i){
		custoDual = -pontoInterior[inicioMU + i] + pontoInterior[inicioPI + saltoK + i] - pontoInterior[inicioXI + saltoK + i];
		g->setCustoVerticeDual(i+nCommodities+1, custoDual);
	}
	g->setCustoArestasDual(k, 'C');
}


float ModeloCplex::getAlfaDual(int k){
	return pontoInterior[k];
}

float ModeloCplex::getBetaDual(int k){
	return pontoInterior[maxVeic+k];
}

void ModeloCplex::insert_ColumnPrimal_RowDual_Forn(Rota* r, int k){
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

	//inicializacao do vetor para armazenar o numero de visitas de cada vertice da coluna
	int* vetVisitRota = new int [nCommodities];
	memset (vetVisitRota, 0, nCommodities*sizeof(int));

	//coloca os indices (> 0) nas respectivas posicoes da coluna
	for (int i = 1; i <= numVertAtualizar; ++i) {
		++vetVisitRota[vertRota[i]-1];
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal1[k](1);
	for (int i = 0; i < nCommodities; i++){
		if (vetVisitRota[i] > 0){
			col += constraintsPrimal3[i](vetVisitRota[i]);
			col += constraintsPrimal5[k*nCommodities + i](vetVisitRota[i]);
			col += constraintsPrimal6[k*nCommodities + i](-vetVisitRota[i]);
		}
	}
	lambdaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	//DEPOIS CRIA-SE A RESTRICAO NO DUAL ASSOCIADA A ROTA
	IloExpr res = alfaDual[k];
	for (int i = 0; i < nCommodities; ++i){
		if (vetVisitRota[i] > 0){
			res += vetVisitRota[i] * thetaDual[i+1] + vetVisitRota[i] * piDual[k][i+1] - vetVisitRota[i] * xiDual[k][i+1];
		}
	}
	constraintsDual1[k].add(res <= r->getCusto());
	modelDual.add(constraintsDual1[k][qRotasForn[k]]);
	res.end();

	++qRotasForn[k];
	delete [] vetVisitRota;
}

void ModeloCplex::insert_ColumnPrimal_RowDual_Cons(Rota* r, int k){
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

	//inicializacao do vetor para armazenar o numero de visitas de cada vertice da coluna
	int* vetVisitRota = new int [nCommodities];
	memset (vetVisitRota, 0, nCommodities*sizeof(int));

	//coloca os indices (> 0) nas respectivas posicoes da coluna
	for (int i = 1; i <= numVertAtualizar; ++i) {
		++vetVisitRota[vertRota[i]-nCommodities-1];
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA PASSADA COMO PARAMETRO E INSERE NA VARIAVEL ASSOCIADA AO VEICULO K
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal2[k](1);
	for (int i = 0; i < nCommodities; i++){
		if (vetVisitRota[i] > 0){
			col += constraintsPrimal4[i](vetVisitRota[i]);
			col += constraintsPrimal5[k*nCommodities + i](-vetVisitRota[i]);
			col += constraintsPrimal6[k*nCommodities + i](vetVisitRota[i]);
		}
	}
	gammaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	//DEPOIS CRIA-SE A RESTRICAO NO DUAL ASSOCIADA A ROTA
	IloExpr res = betaDual[k];
	for (int i = 0; i < nCommodities; ++i){
		if (vetVisitRota[i] > 0){
			res += vetVisitRota[i] * muDual[i+1] - vetVisitRota[i] * piDual[k][i+1] + vetVisitRota[i] * xiDual[k][i+1];
		}
	}
	constraintsDual2[k].add(res <= r->getCusto());
	modelDual.add(constraintsDual2[k][qRotasCons[k]]);
	res.end();

	++qRotasCons[k];
	delete [] vetVisitRota;
}

void ModeloCplex::initVarsPrimal(){

	lambdaPrimal = IloArray<IloNumVarArray>(env, maxVeic);
	for (int i = 0; i < maxVeic; i++){
		lambdaPrimal[i] = IloNumVarArray(env, qRotasForn[i], 0, 1, ILOFLOAT);
		for (int j = 0; j < qRotasForn[i]; j++){
			char nome[20];
			sprintf(nome, "lbd_%01d_%02d", i, j);
			lambdaPrimal[i][j].setName(nome);
		}
	}
	gammaPrimal = IloArray<IloNumVarArray>(env, maxVeic);
	for (int i = 0; i < maxVeic; i++){
		gammaPrimal[i] = IloNumVarArray(env, qRotasCons[i], 0, 1, ILOFLOAT);
		for (int j = 0; j < qRotasCons[i]; j++){
			char nome[20];
			sprintf(nome, "gam_%01d_%02d", i, j);
			gammaPrimal[i][j].setName(nome);
		}
	}
	tauPrimal = IloArray<IloNumVarArray>(env, maxVeic);
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
	
	//Adiciona-se os coeficientes relacionados as variaveis lambdaPrimal
	for (int i = 0; i < maxVeic; ++i){
		for (int j = 0; j < qRotasForn[i]; ++j){
			obj += ptrRotasForn[i][j]->getCusto() * lambdaPrimal[i][j];
		}
	}
		
	//E também adiciona-se os coeficientes relacionados as variaveis gammaPrimal
	for (int i = 0; i < maxVeic; ++i){
		for (int j = 0; j < qRotasCons[i]; ++j){
			obj += ptrRotasCons[i][j]->getCusto() * gammaPrimal[i][j];
		}
	}

	//Por fim, os coeficientes das trocas no Cross-Docking 
	for (int i = 0; i < maxVeic; ++i){
		for (int j = 1; j <= nCommodities; ++j){
			obj += custoTrocaCD * tauPrimal[i][j];
		}
	}

	//Cria o objeto para armazenar a funcao objetivo e atribui a expressao a ele
	costPrimal = IloObjective(env, obj);
	modelPrimal.add(costPrimal);
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


void ModeloCplex::setConstraintsPrimal1(){
	constraintsPrimal1 = IloRangeArray(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k){
		IloExpr exp1 = IloExpr(env);
		for (int j = 0; j < qRotasForn[k]; ++j){
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

	for (int k = 0; k < maxVeic; ++k){
		IloExpr exp2 = IloExpr(env);
		for (int j = 0; j < qRotasCons[k]; ++j){
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
	for (int i = 1; i <= nCommodities; ++i){
		IloExpr exp3 = IloExpr(env);
		for (int k = 0; k < maxVeic; ++k){
			for (int l = 0; l < qRotasForn[k]; ++l){
				if (A_qr[k][l][i] > 0){
					exp3 += A_qr[k][l][i] * lambdaPrimal[k][l];
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
	for (int i = 1; i <= nCommodities; ++i){
		IloExpr exp4 = IloExpr(env);
		for (int k = 0; k < maxVeic; ++k){
			for (int l = 0; l < qRotasCons[k]; ++l){
				if (B_qr[k][l][i] > 0){
					exp4 += B_qr[k][l][i] * gammaPrimal[k][l];
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

	for (int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			IloExpr exp5 = IloExpr(env);
			for (int j = 0; j < qRotasForn[k]; ++j){
				if (A_qr[k][j][i] > 0){
					exp5 += A_qr[k][j][i] * lambdaPrimal[k][j];
				}
			}
			for (int j = 0; j < qRotasCons[k]; ++j){
				if (B_qr[k][j][i] > 0){
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

	for (int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			IloExpr exp6 = IloExpr(env);
			for (int j = 0; j < qRotasForn[k]; ++j){
				if (A_qr[k][j][i] > 0){
					exp6 -= A_qr[k][j][i] * lambdaPrimal[k][j];
				}
			}
			for (int j = 0; j < qRotasCons[k]; ++j){
				if (B_qr[k][j][i] > 0){
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

void ModeloCplex::initVarsDual(){

	alfaDual = IloNumVarArray(env, maxVeic, -IloInfinity, +IloInfinity, ILOFLOAT);
	for (int k = 0; k < maxVeic; ++k){
		char nome[20];
		sprintf(nome, "alfa_%01d", k);
		alfaDual[k].setName(nome);
	}

	betaDual = IloNumVarArray(env, maxVeic, -IloInfinity, +IloInfinity, ILOFLOAT);
	for (int k = 0; k < maxVeic; ++k){
		char nome[20];
		sprintf(nome, "beta_%01d", k);
		betaDual[k].setName(nome);
	}

	thetaDual = IloNumVarArray(env, (nCommodities+1), -IloInfinity, +IloInfinity, ILOFLOAT);
	for (int i = 0; i <= nCommodities; ++i){
		char nome[20];
		sprintf(nome, "teta_%02d", i);
		thetaDual[i].setName(nome);
	}

	muDual = IloNumVarArray(env, (nCommodities+1), -IloInfinity, +IloInfinity, ILOFLOAT);
	for (int i = 0; i <= nCommodities; ++i){
		char nome[20];
		sprintf(nome, "mu_%02d", i);
		muDual[i].setName(nome);
	}

	piDual = IloArray<IloNumVarArray>(env, maxVeic);
	for (int k = 0; k < maxVeic; ++k){
		piDual[k] = IloNumVarArray(env, (nCommodities+1), 0, +IloInfinity, ILOFLOAT);
		for (int i = 0; i <= nCommodities; ++i){
			char nome[20];
			sprintf(nome, "pi_%01d_%02d", k, i);
			piDual[k][i].setName(nome);
		}
	}

	xiDual = IloArray<IloNumVarArray>(env, maxVeic);
	for (int k = 0; k < maxVeic; ++k){
		xiDual[k] = IloNumVarArray(env, (nCommodities+1), 0, +IloInfinity, ILOFLOAT);
		for (int i = 0; i <= nCommodities; ++i){
			char nome[20];
			sprintf(nome, "xi%01d_%02d", k, i);
			xiDual[k][i].setName(nome);
		}
	}
}

void ModeloCplex::setObjectiveFunctionDual(char** varComRestr){
		IloExpr multiplicadoresObj(env);
		//Variaveis alfa
		for (int k = 0; k < maxVeic; ++k){
			multiplicadoresObj += ((rand()%101) / 100.0) * alfaDual[k];
		}
		//Variaveis beta
		for (int k = 0; k < maxVeic; ++k){
			multiplicadoresObj += ((rand()%101) / 100.0) * betaDual[k];
		}
		//Variaveis theta
		for (int i = 1; i <= nCommodities; ++i){
			multiplicadoresObj += ((rand()%101) / 100.0) * thetaDual[i];
		}
		//Variaveis mu
		for (int i = 1; i <= nCommodities; ++i){
			multiplicadoresObj += ((rand()%101) / 100.0) * muDual[i];
		}

		//Inclui os multiplicadores na funcao objetivo relacionados as variaveis da arvore no dual
		if (varComRestr != NULL){
			int numRestricoesArvoreK, numRestricoesArvoreTau1 = 0;
			for (int k = 0; k < maxVeic; ++k){
				numRestricoesArvoreK = 0;
				for (int i = 1; i <= nCommodities; ++i){
					if (varComRestr[k][i] > 0){
						if ((varComRestr[k][i] == 1) || (varComRestr[k][i] == 3)){
							multiplicadoresObj += ((rand()%101) / 100.0) * varDualArvoreLambda[k][numRestricoesArvoreK];
						}
						if ((varComRestr[k][i] == 1) || (varComRestr[k][i] == 4)){
							multiplicadoresObj += ((rand()%101) / 100.0) * varDualArvoreGamma[k][numRestricoesArvoreK];
						}
						//conta quantas restricoes na arvore fixam a variavel tau em 1 para ajustar na funcao objetivo
						if ((varComRestr[k][i] == 3) || (varComRestr[k][i] == 4)){
							++numRestricoesArvoreTau1;
						}
						++numRestricoesArvoreK;
					}
				}
			}
			multiplicadoresObj += ((rand()%101) / 100.0) * numRestricoesArvoreTau1 * varDualArvoreTau1;
		}

		costDual.setExpr(multiplicadoresObj);
		costDual.setSense(IloObjective::Maximize);
}

void ModeloCplex::setConstraintsDual1(){
	constraintsDual1 = IloArray<IloRangeArray>(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k){
		constraintsDual1[k] = IloRangeArray(env, qRotasForn[k]);

		for (int r = 0; r < qRotasForn[k]; ++r){
			IloExpr expD1 = IloExpr(env);

			expD1 += alfaDual[k];

			for (int i = 1; i <= nCommodities; ++i){
				if (A_qr[k][r][i] > 0){
					expD1 += thetaDual[i];
					expD1 += piDual[k][i];
					expD1 -= xiDual[k][i];
				}
			}

			//Insere a restricao no modelo
			constraintsDual1[k][r] = (expD1 <= ptrRotasForn[k][r]->getCusto()); //O sinal da desigualdade podera mudar depois que o primal for executado
			char nome[20];
			sprintf(nome, "rest1_k%01d_r%02d", k, r);
			constraintsDual1[k][r].setName(nome);
			modelDual.add(constraintsDual1[k][r]);
			expD1.end();
		}
	}
}


void ModeloCplex::setConstraintsDual2(){
	constraintsDual2 = IloArray<IloRangeArray>(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k){
		constraintsDual2[k] = IloRangeArray(env, qRotasCons[k]);

		for (int r = 0; r < qRotasCons[k]; ++r){
			IloExpr expD2 = IloExpr(env);

			expD2 += betaDual[k];

			for (int i = 1; i <= nCommodities; ++i){
				if (B_qr[k][r][i] > 0){
					expD2 += muDual[i];
					expD2 -= piDual[k][i];
					expD2 += xiDual[k][i];
				}
			}

			//Insere a restricao no modelo
			constraintsDual2[k][r] = (expD2 <= ptrRotasCons[k][r]->getCusto()); //O sinal da desigualdade podera mudar depois que o primal for executado
			char nome[20];
			sprintf(nome, "rest2_k%01d_r%02d", k, r);
			constraintsDual2[k][r].setName(nome);
			modelDual.add(constraintsDual2[k][r]);
			expD2.end();
		}
	}
}


void ModeloCplex::setConstraintsDual3(){
	constraintsDual3 = IloRangeArray(env, maxVeic*nCommodities);

	int count = 0;
	for (int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			IloExpr expD3 = IloExpr(env);
			expD3 = piDual[k][i] + xiDual[k][i];
			//Insere a restricao no modelo
			constraintsDual3[count] = (expD3 <= custoTrocaCD); //O sinal da desigualdade podera mudar depois que o primal for executado
			char nome[20];
			sprintf(nome, "rest3_k%01d_q%02d", k, i);
			constraintsDual3[count].setName(nome);
			modelDual.add(constraintsDual3[count]);
			expD3.end();
			++count;
		}
	}
}

void ModeloCplex::geraModeloDual(char** varComRestr){
	//Sera necessario obter os valores das variaveis de decisao obtidas na solucao primal
	//e entao verificar se o sinal da restricao no dual sera <= ou ==
	float ZERO = 0.00005;
	int count;
	IloNum rhs;

	IloNumArray lambdaValues(env);
	for(int k = 0; k < maxVeic; ++k){
		cplexPrimal.getValues(lambdaValues, lambdaPrimal[k]);

		for (int r = 0; r < qRotasForn[k]; ++r){
			rhs = constraintsDual1[k][r].getUb();
			if ((lambdaValues[r] >= -ZERO) && (lambdaValues[r] <= ZERO)){
				constraintsDual1[k][r].setBounds(-IloInfinity, rhs);
			}else{
				constraintsDual1[k][r].setBounds(rhs, rhs);
			}
		}
	}
	lambdaValues.end();

	IloNumArray gammaValues(env);
	for(int k = 0; k < maxVeic; ++k){
		cplexPrimal.getValues(gammaValues, gammaPrimal[k]);

		for (int r = 0; r < qRotasCons[k]; ++r){
			rhs = constraintsDual2[k][r].getUb();
			if ((gammaValues[r] >= -ZERO) && (gammaValues[r] <= ZERO)){
				constraintsDual2[k][r].setBounds(-IloInfinity, rhs);
			}else{
				constraintsDual2[k][r].setBounds(rhs, rhs);
			}
		}
	}
	gammaValues.end();

	count = 0;
	IloNum tauValue;
	for(int k = 0; k < maxVeic; ++k){
		for(int i = 1; i <= nCommodities; ++i){
			tauValue = cplexPrimal.getValue(tauPrimal[k][i]);
			if ((tauValue >= -ZERO) && (tauValue <= ZERO)){
				constraintsDual3[count].setBounds(-IloInfinity, custoTrocaCD);
			}else{
				constraintsDual3[count].setBounds(custoTrocaCD, custoTrocaCD);
			}
			++count;
		}
	}

	if (varComRestr != NULL){
		//Caso esteja gerando o modelo dual quando se tem colunas associadas aos nos do branch and price verifica
		//os valores das variaveis de decisao lambdaNo e gammaNo para fixar ou nao as respectivas restricoes
		IloNumArray lambdaNoValues(env);
		for(int k = 0; k < maxVeic; ++k){
			cplexPrimal.getValues(lambdaNoValues, lambdaNo[k]);

			for (int r = 0; r < lambdaNoValues.getSize(); ++r){
				rhs = constraintsDual1_no[k][r].getUb();
				if ((lambdaNoValues[r] >= -ZERO) && (lambdaNoValues[r] <= ZERO)){
					constraintsDual1_no[k][r].setBounds(-IloInfinity, rhs);
				}else{
					constraintsDual1_no[k][r].setBounds(rhs, rhs);
				}
			}
		}
		lambdaNoValues.end();

		IloNumArray gammaNoValues(env);
		for(int k = 0; k < maxVeic; ++k){
			cplexPrimal.getValues(gammaNoValues, gammaNo[k]);

			for (int r = 0; r < gammaNoValues.getSize(); ++r){
				rhs = constraintsDual2_no[k][r].getUb();
				if ((gammaNoValues[r] >= -ZERO) && (gammaNoValues[r] <= ZERO)){
					constraintsDual2_no[k][r].setBounds(-IloInfinity, rhs);
				}else{
					constraintsDual2_no[k][r].setBounds(rhs, rhs);
				}
			}
		}
		gammaNoValues.end();


		//Verifica os valores das variaveis de decisao artificiais, para saber
		//se suas respetivas restricoes no dual serao fixadas ou nao
		IloNumArray artificiaisValues(env);
		cplexPrimal.getValues(artificiaisValues, artificiais);
		for (int a = 0; a < artificiaisValues.getSize(); ++a){
			if ((artificiaisValues[a] >= -ZERO) && (artificiaisValues[a] <= ZERO)){
				constraintsDualArtificiais[a].setBounds(-IloInfinity, ModeloCplex::limitePrimal);
			}else{
				constraintsDualArtificiais[a].setBounds(ModeloCplex::limitePrimal, ModeloCplex::limitePrimal);
			}
		}
		artificiaisValues.end();
	}

	//Obtem os valores das variaveis de folga associadas a cada restricao primal e caso a restricao
	//esteja folgada, a respectiva variavel de decisao no primal deve ser 0;
	IloNumArray slackConstraints1(env);
	cplexPrimal.getSlacks(slackConstraints1, constraintsPrimal1);
	for (int k = 0; k < maxVeic; ++k){
		if  ((slackConstraints1[k] <= -ZERO) || (slackConstraints1[k] >= ZERO)){
			alfaDual[k].setBounds(0, 0);
		}else{
			alfaDual[k].setBounds(-IloInfinity, +IloInfinity);
		}
	}
	slackConstraints1.end();

	IloNumArray slackConstraints2(env);
	cplexPrimal.getSlacks(slackConstraints2, constraintsPrimal2);
	for (int k = 0; k < maxVeic; ++k){
		if  ((slackConstraints2[k] <= -ZERO) || (slackConstraints2[k] >= ZERO)){
			betaDual[k].setBounds(0, 0);
		}else{
			betaDual[k].setBounds(-IloInfinity, +IloInfinity);
		}
	}
	slackConstraints2.end();

	IloNumArray slackConstraints3(env);
	cplexPrimal.getSlacks(slackConstraints3, constraintsPrimal3);
	for (int i = 1; i <= nCommodities; ++i){
		if  ((slackConstraints3[i-1] <= -ZERO) || (slackConstraints3[i-1] >= ZERO)){
			thetaDual[i].setBounds(0, 0);
		}else{
			thetaDual[i].setBounds(-IloInfinity, +IloInfinity);
		}
	}
	slackConstraints3.end();

	IloNumArray slackConstraints4(env);
	cplexPrimal.getSlacks(slackConstraints4, constraintsPrimal4);
	for (int i = 1; i <= nCommodities; ++i){
		if  ((slackConstraints4[i-1] <= -ZERO) || (slackConstraints4[i-1] >= ZERO)){
			muDual[i].setBounds(0, 0);
		}else{
			muDual[i].setBounds(-IloInfinity, +IloInfinity);
		}
	}
	slackConstraints4.end();

	IloNumArray slackConstraints5(env);
	cplexPrimal.getSlacks(slackConstraints5, constraintsPrimal5);
	count = 0;
	for(int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			if  ((slackConstraints5[count] <= -ZERO) || (slackConstraints5[count] >= ZERO)){
				piDual[k][i].setBounds(0, 0);
			}else{
				piDual[k][i].setBounds(0, +IloInfinity);
			}
		}
		++count;
	}
	slackConstraints5.end();

	IloNumArray slackConstraints6(env);
	cplexPrimal.getSlacks(slackConstraints6, constraintsPrimal6);
	count = 0;
	for(int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			if  ((slackConstraints6[count] <= -ZERO) || (slackConstraints6[count] >= ZERO)){
				xiDual[k][i].setBounds(0, 0);
			}else{
				xiDual[k][i].setBounds(0, +IloInfinity);
			}
		}
		++count;
	}
	slackConstraints6.end();

	if (varComRestr != NULL){
		//Verifica as folgas de cada restricao do primal relacionadas as restricoes da arvore
		//e determina quais variaveis duais serao fixas ao executar o modelo dual
		IloNum slackArvore;
		int countK, countTotal = 0;

		//variaveis duais associadas as restricoes que fixam lambda e gamma na arvore do primal
		for(int k = 0; k < maxVeic; ++k){
			countK = 0;
			for(int i = 1; i <= nCommodities; ++i){
				if (varComRestr[k][i] > 0){
					slackArvore = cplexPrimal.getSlack(constraintsArvoreLambda[countTotal]);
					if  ((slackArvore <= -ZERO) || (slackArvore >= ZERO)){
						varDualArvoreLambda[k][countK].setBounds(0, 0);
					}else{
						varDualArvoreLambda[k][countK].setBounds(-IloInfinity, +IloInfinity);
					}

					slackArvore = cplexPrimal.getSlack(constraintsArvoreGamma[countTotal]);
					if  ((slackArvore <= -ZERO) || (slackArvore >= ZERO)){
						varDualArvoreGamma[k][countK].setBounds(0, 0);
					}else{
						varDualArvoreGamma[k][countK].setBounds(-IloInfinity, +IloInfinity);
					}

					++countTotal;
					++countK;
				}
			}
		}

		//variavel dual associada a restricao \sum tau^k_q = |numVariaveisFixas|
		slackArvore = cplexPrimal.getSlack(constraintArvoreTau0);
		if  ((slackArvore <= -ZERO) || (slackArvore >= ZERO)){
			varDualArvoreTau0.setBounds(0, 0);
		}else{
			varDualArvoreTau0.setBounds(-IloInfinity, +IloInfinity);
		}

		//variavel dual associada a restricao \sum tau^k_q = 0
		slackArvore = cplexPrimal.getSlack(constraintArvoreTau1);
		if  ((slackArvore <= -ZERO) || (slackArvore >= ZERO)){
			varDualArvoreTau1.setBounds(0, 0);
		}else{
			varDualArvoreTau1.setBounds(-IloInfinity, +IloInfinity);
		}
	}
}


void ModeloCplex::geraPontoInterior(int numPontosExtremos){
	if (numPontosExtremos > 0){
		int i = 0, pontosExtremosObtidos = 0;
		IloNum *pontoExtremo;
		memset(pontoInterior, 0, tamPontoInterior * sizeof(IloNum));

		while(i < numPontosExtremos){
			//atribui novos multiplicadores a funcao objetivo dual
			setObjectiveFunctionDual(NULL);

			//O metodo getPontoExtremo() pode retornar um valor NULL, caso o dual seja 'unbounded' ou 'infeasible'
			//caso isto aconteca, o valor retornado deve ser desconsiderado
			pontoExtremo = getPontoExtremo(NULL);

			if (pontoExtremo != NULL){
				for (int j = 0; j < tamPontoInterior; ++j){
					pontoInterior[j] += pontoExtremo[j];
				}
				delete [] pontoExtremo;
				++pontosExtremosObtidos;
			}
			++i;
		}

		if (pontosExtremosObtidos > 0){
			//obtem a media em cada dimensao do ponto interior obtido
			for (int j = 0; j < tamPontoInterior; ++j){
				pontoInterior[j] /= pontosExtremosObtidos;
			}
			return;
		}
		cout << "ENTROU EM PONTOS INTERIORES MAS NAO RETORNOU PONTO ALGUM!!" << endl;
	}

	//CASO NAO QUEIRA PONTOS INTERIORES, OU MESMO SE UM PONTO INTERIOR NAO FOR ENCONTRADO
	//UTILIZA-SE AS INFORMACOES DO PRIMAL PARA OBTER OS VALORES DAS VARIAVEIS DUAIS
	int count = 0;
	IloNumArray dualValues(env);
	//variavel dual ALFA
	cplexPrimal.getDuals(dualValues, constraintsPrimal1);
	for (int k = 0; k < maxVeic; ++k){
		pontoInterior[count++] = dualValues[k];
	}

	//variavel dual BETA
	cplexPrimal.getDuals(dualValues, constraintsPrimal2);
	for (int k = 0; k < maxVeic; ++k){
		pontoInterior[count++] = dualValues[k];
	}

	//variavel dual THETA
	cplexPrimal.getDuals(dualValues, constraintsPrimal3);
	for (int i = 0; i < nCommodities; ++i){
		pontoInterior[count++] = dualValues[i];
	}

	//variavel dual MU
	cplexPrimal.getDuals(dualValues, constraintsPrimal4);
	for (int i = 0; i < nCommodities; ++i){
		pontoInterior[count++] = dualValues[i];
	}

	//variavel dual PI
	cplexPrimal.getDuals(dualValues, constraintsPrimal5);
	for (int i = 0; i < maxVeic*nCommodities; ++i){
		pontoInterior[count++] = dualValues[i];
	}

	//variavel dual XI
	cplexPrimal.getDuals(dualValues, constraintsPrimal6);
	for (int i = 0; i < maxVeic*nCommodities; ++i){
		pontoInterior[count++] = dualValues[i];
	}
	dualValues.end();
}


IloNum* ModeloCplex::getPontoExtremo(char** varComRestr, int acresc){
	IloNum *result;

	if (cplexDual.solve()){
		int count = 0;
		result = new IloNum[tamPontoInterior + 2*acresc];

		//armazena no vetor correspondente ao ponto extremo os valores das variaveis alfaDual e betaDual
		for (int k = 0; k < maxVeic; ++k){
			result[count] = cplexDual.getValue(alfaDual[k]);
			result[count+maxVeic] = cplexDual.getValue(betaDual[k]);
			++count;
		}
		count += maxVeic;

		//armazena no vetor do ponto extremo os valores das variaveis thetaDual e muDual
		for (int i = 1; i <= nCommodities; ++i){
			result[count] = cplexDual.getValue(thetaDual[i]);
			result[count+nCommodities] = cplexDual.getValue(muDual[i]);
			++count;
		}
		count += nCommodities;

		//armazena no vetor do ponto extremo os valores das variaveis piDual e xiDual
		int salto = maxVeic * nCommodities;
		for (int k = 0; k < maxVeic; ++k){
			for (int i = 1; i <= nCommodities; ++i){
				result[count] = cplexDual.getValue(piDual[k][i]);
				result[count+salto] = cplexDual.getValue(xiDual[k][i]);
				++count;
			}
		}

		if (varComRestr != NULL){
			//armazena no vetor do ponto extremo os valores das variaveis piDual e xiDual
			count += salto;
			for (int k = 0; k < maxVeic; ++k){
				int numRestrArvoreK = 0;
				for (int i = 1; i <= nCommodities; ++i){
					if (varComRestr[k][i] > 0){
						result[count] = cplexDual.getValue(varDualArvoreLambda[k][numRestrArvoreK]);
						result[count + acresc] = cplexDual.getValue(varDualArvoreGamma[k][numRestrArvoreK]);
						++count;
						++numRestrArvoreK;
					}
				}
			}
		}

	}else{
		result = NULL;
	}

	return result;
}


void ModeloCplex::imprimeVariaveisDuais(){
	cout << "Obtidas do modelo primal (opt = " << cplexPrimal.getObjValue() << ")" << endl;
	IloNumArray dualValuesP(env);
	cplexPrimal.getDuals(dualValuesP, constraintsPrimal1);
	env.out() << "alfaDual\n" << dualValuesP << endl;
	cplexPrimal.getDuals(dualValuesP, constraintsPrimal2);
	env.out() << "betaDual\n" << dualValuesP << endl;
	cplexPrimal.getDuals(dualValuesP, constraintsPrimal3);
	env.out() << "thetaDual\n" << dualValuesP << endl;
	cplexPrimal.getDuals(dualValuesP, constraintsPrimal4);
	env.out() << "muDual\n" << dualValuesP << endl;
	cplexPrimal.getDuals(dualValuesP, constraintsPrimal5);
	env.out() << "piDual\n" << dualValuesP << endl;
	cplexPrimal.getDuals(dualValuesP, constraintsPrimal6);
	env.out() << "xiDual\n" << dualValuesP << endl;

	cout << "Obtidas do modelo dual (opt = " << cplexDual.getObjValue() << ")" << endl;
	IloNumArray dualValuesD(env);
	cplexDual.getValues(dualValuesD, alfaDual);
	env.out() << "alfaDual\n" << dualValuesD << endl;
	cplexDual.getValues(dualValuesD, betaDual);
	env.out() << "betaDual\n" << dualValuesD << endl;
	env.out() << "thetaDual" << endl;
	for (int i = 1; i <= nCommodities; ++i){
		env.out() << cplexDual.getValue(thetaDual[i]) << " ";
	}
	env.out() << "\nmuDual" << endl;
	for (int i = 1; i <= nCommodities; ++i){
		env.out() << cplexDual.getValue(muDual[i]) << " ";
	}
	env.out() << "\npiDual" << endl;
	for (int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			env.out() << cplexDual.getValue(piDual[k][i]) << " ";
		}
	}
	env.out() << "\nxiDual" << endl;
	for (int k = 0; k < maxVeic; ++k){
		for (int i = 1; i <= nCommodities; ++i){
			env.out() << cplexDual.getValue(xiDual[k][i]) << " ";
		}
	}
}

void ModeloCplex::limpaVariaveisArvoreB1(){
	//limpa variaveis dos modelos primal e dual	
	for (int k = 0; k < maxVeic; ++k){
		lambdaNo[k].endElements();
		constraintsDual1_no[k].endElements();

		gammaNo[k].endElements();
		constraintsDual2_no[k].endElements();

		varDualArvoreLambda[k].endElements();
		varDualArvoreGamma[k].endElements();
	}

	artificiais.endElements();
	constraintsDualArtificiais.endElements();

	varDualArvoreTau0.end();
	varDualArvoreTau1.end();
	constraintArvoreTau0.end();
	constraintArvoreTau1.end();
	constraintsArvoreLambda.endElements();
	constraintsArvoreGamma.endElements();
}

void ModeloCplex::limpaVariaveisArvoreB2(){
	for (int k = 0; k < maxVeic; ++k){
		lambdaNo[k].endElements();
		gammaNo[k].endElements();
	}
	artificiais.endElements();
	constraintsArvoreArcos.endElements();
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
	for (int k = 0; k < maxVeic; ++k){
		cplexPrimal.getValues(valuesF, lambdaPrimal[k]);
		for (int r = 0; r < qRotasForn[k]; ++r){
			if (valuesF[r] >= 0.00001){
				ptrRotasForn[k][r]->setBasica();
			}
		}

		cplexPrimal.getValues(valuesC, gammaPrimal[k]);
		for (int r = 0; r < qRotasCons[k]; ++r){
			if (valuesC[r] >= 0.00001){
				ptrRotasCons[k][r]->setBasica();
			}
		}
	}
}

