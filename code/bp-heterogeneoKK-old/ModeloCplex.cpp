#include "ModeloCplex.h"
#include <sys/time.h>
using namespace std;

float ModeloCplex::limitePrimal;
Rota** retornaColuna(Grafo* G, int k, bool camForn, bool pricingFull, float valSub, char op, int& retornoPricing){
	Rota** r;
	if (op == 'S'){
		StateSpaceRelax camElemSSR(G, camForn, valSub);
		retornoPricing = camElemSSR.calculaCaminhoElementarSSR(G, Grafo::classesVeic[k]);
		if (retornoPricing > 0){
			r = camElemSSR.getRotaCustoMinimo(G, Grafo::classesVeic[k], 0.99);
			if ( r == NULL ) retornoPricing = -1;
		}else{
			r = NULL;
		}
		return r;

	}else if (op == 'E'){
		Elementar camElem(G, camForn, valSub);
		retornoPricing = camElem.calculaCaminhoElementar(G, pricingFull, Grafo::classesVeic[k]);
		if (retornoPricing > 0){
			r = camElem.getRotaCustoMinimo(G, Grafo::classesVeic[k], 0.99);
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
	for (int k = 0; k < maxVeic; ++k){
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
	for (int kForn = 0; kForn < maxVeic; kForn++)
	{
		for (int kCons = 0; kCons < maxVeic; kCons++)
		{
			obj += MAIS_INFINITO * tauPrimal[kForn][kCons][0];
			for (int i = 1; i <= nCommodities; i++)
			{
				obj += custoTrocaCD * tauPrimal[kForn][kCons][i];
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
					if (A_qr[kForn][j][i] > 0){
						exp5 += A_qr[kForn][j][i] * lambdaPrimal[kForn][j];
					}
				}
				for (int j = 0; j < qRotasCons[kCons]; ++j){
					if (B_qr[kCons][j][i] > 0){
						exp5 += B_qr[kCons][j][i] * gammaPrimal[kCons][j];
					}
				}
				exp5 -= tauPrimal[kForn][kCons][i];

				//Insere a restrição no modelo
				if ( kForn != kCons ) constraintsPrimal5[count] = (exp5 <= 1);
				else constraintsPrimal5[count] = (exp5 <= 2);
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
	constraintsPrimal6 = IloArray < IloRangeArray > (env, maxVeic);
	constraintsPrimal7 = IloRangeArray (env);
	aPrimal = IloNumVarArray(env);

	for ( int k = 0; k < maxVeic; ++k )
	{
		char nome[20];
		constraintsPrimal6[k] = IloRangeArray(env);
		constraintsPrimal6[k].add( lambdaPrimal[k][0] <= 0);
		sprintf(nome, "rest6_k%d_r%d", k, (int)constraintsPrimal6[k].getSize()-1);
		constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1].setName(nome);
		modelPrimal.add(constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1]);

		constraintsPrimal7.add( lambdaPrimal[k][0] <= 0 );
		sprintf(nome, "rest7_r%d", (int)constraintsPrimal7.getSize()-1);
		constraintsPrimal7[constraintsPrimal7.getSize()-1].setName(nome);
		modelPrimal.add(constraintsPrimal7[constraintsPrimal7.getSize()-1]);
		
		IloNumColumn col = costPrimal(0);
		col += constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1](-1);
		col += constraintsPrimal7[constraintsPrimal7.getSize()-1](-1);		
		sprintf(nome, "a_%d", (int)aPrimal.getSize());
		aPrimal.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
		col.end();
		
		strHash* aux = new strHash();
		aux->veicsRota = new int[maxVeic];
		for ( int i = 0; i < maxVeic; ++i ) aux->veicsRota[i] = -1;
		aux->veicsRota[k] = constraintsPrimal6[k].getSize()-1;
		aux->ptrRota = ptrRotasForn[k][0];
		aux->ptrRota->incrNumApontadores();
		vetorRotas.push_back(aux);
	}
}


void ModeloCplex::updateDualCostsForn(Grafo* g, int k){
	float custoDual, somaXi;
	int ajuste = k*maxVeic*nCommodities;
	IloNumArray thetaDuals(env), xiDuals(env);
	cplexPrimal.getDuals(thetaDuals, constraintsPrimal3);
	cplexPrimal.getDuals(xiDuals, constraintsPrimal5);

	for (int i = 0; i < nCommodities; ++i)
	{
		somaXi = 0;
		for ( int kCons = 0; kCons < maxVeic; ++kCons )
		{
			somaXi += xiDuals[ajuste + kCons*nCommodities + i];
		}
		custoDual = -thetaDuals[i] - somaXi;
		g->setCustoVerticeDual(i+1, custoDual);
	}
	thetaDuals.end(); xiDuals.end();
	g->setCustoArestasDual(Grafo::classesVeic[k], 'F');
}


void ModeloCplex::updateDualCostsCons(Grafo* g, int k){
	float custoDual, somaXi;
	int ajuste = k*nCommodities;
	IloNumArray muDuals(env), xiDuals(env);
	cplexPrimal.getDuals(muDuals, constraintsPrimal4);
	cplexPrimal.getDuals(xiDuals, constraintsPrimal5);

	for (int i = 0; i < nCommodities; ++i)
	{
		somaXi = 0;
		for ( int kForn = 0; kForn < maxVeic; ++kForn )
		{
			somaXi += xiDuals[ajuste + kForn*maxVeic*nCommodities + i];
		}
		custoDual = -muDuals[i] - somaXi;
		g->setCustoVerticeDual(nCommodities+i+1, custoDual);
	}
	muDuals.end(); xiDuals.end();
	g->setCustoArestasDual(Grafo::classesVeic[k], 'C');
}


float ModeloCplex::getValorSubtrairForn( int k ){
	float valSub = cplexPrimal.getDual(constraintsPrimal1[k]);
	IloNumArray phiDuals(env), psiDuals(env), aValues(env);
	cplexPrimal.getDuals(phiDuals, constraintsPrimal6[k]);
	cplexPrimal.getDuals(psiDuals, constraintsPrimal7);
	cplexPrimal.getValues(aValues, aPrimal);

	float somaPrimal = 0;
	float somaDualPhi = 0;
	float somaDualPsi = 0;
	for ( int i = 0; i < vetorRotas.size(); ++i )
	{
		if ( vetorRotas[i]->veicsRota[k] >= 0 ) somaDualPhi += ( phiDuals[vetorRotas[i]->veicsRota[k]] );
		if ( vetorRotas[i]->veicsRota[k] >= 0 ) somaDualPsi += ( psiDuals[i] );
//		if ( vetorRotas[i]->veicsRota[k] >= 0 ) somaPrimal += aValues[i] * ( nCommodities * custoTrocaCD );
	}
	
	phiDuals.end(); psiDuals.end(); aValues.end();
	
//	printf("[%d] valSub(%f) + somaDualPhi(%f) + somaDualPsi(%f) + somaPrimal(%f) = %f\n", k, valSub, somaDualPhi, somaDualPsi, somaPrimal, valSub + somaDualPhi + somaDualPsi + somaPrimal);
	
	return valSub + somaDualPhi + somaDualPsi + somaPrimal;
}


float ModeloCplex::getValorSubtrairCons(int k){
	float valSub = cplexPrimal.getDual(constraintsPrimal2[k]);
	return valSub;
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
	int ajuste = k*maxVeic*nCommodities;
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal1[k](1);
	for (int i = 0; i < nCommodities; i++){
		if (vetVisitRota[i] > 0){
			col += constraintsPrimal3[i](vetVisitRota[i]);
			for ( int kCons = 0; kCons < maxVeic; ++kCons )
			{
				col += constraintsPrimal5[ajuste + kCons*nCommodities + i](vetVisitRota[i]);
			}
		}
	}

	char nome[20];
	int indice_a_r;
	strHash* aux = getStrHashRota(k, indice_a_r); //indice_a_r passado como referencia
	if ( aux != NULL ) //a rota precificada para o veiculo k, ja havia sido precificada em iteracoes anteriores
	{
		if ( aux->veicsRota[k] < 0 )
		{
			col += constraintsPrimal7[indice_a_r](1);
			sprintf(nome, "lbd_%d_%d", k, qRotasForn[k]);
			lambdaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT, nome));

			aux->veicsRota[k] = constraintsPrimal6[k].getSize();
			constraintsPrimal6[k].add( ( lambdaPrimal[k][qRotasForn[k]] - aPrimal[indice_a_r] ) <= 0 );
			sprintf(nome, "rest6_k%d_r%d", k, (int)constraintsPrimal6[k].getSize()-1);
			constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1].setName(nome);
			modelPrimal.add (constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1]);
			col.end();
		}
		else
		{
			A_qr[k].pop_back();
			ptrRotasForn[k].pop_back();
			if ( r->decrNumApontadores() ) delete r;
			delete [] vetVisitRota;
			return;
			
//			sprintf(nome, "lbd_%d_%d", k, qRotasForn[k]);
//			lambdaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
//			col.end();
		}
	}
	else
	{
		sprintf(nome, "lbd_%d_%d", k, qRotasForn[k]);
		lambdaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
		col.end();

		//inclui a rota no vetor, criar a variavel a_r correspondente e as constraints 6 e 7 da rota/veiculo
		strHash* aux = new strHash();
		aux->veicsRota = new int[maxVeic];
		for ( int i = 0; i < maxVeic; ++i ) aux->veicsRota[i] = -1;
		aux->veicsRota[k] = constraintsPrimal6[k].getSize();
		aux->ptrRota = ptrRotasForn[k][qRotasForn[k]];
		aux->ptrRota->incrNumApontadores();
		vetorRotas.push_back(aux);
		
		constraintsPrimal6[k].add ( lambdaPrimal[k][qRotasForn[k]] <= 0 );
		sprintf(nome, "rest6_k%d_r%d", k, (int)constraintsPrimal6[k].getSize()-1);
		constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1].setName(nome);

		constraintsPrimal7.add ( lambdaPrimal[k][qRotasForn[k]] <= 0 );
		sprintf(nome, "rest7_r%d", (int)constraintsPrimal7.getSize()-1);
		constraintsPrimal7[constraintsPrimal7.getSize()-1].setName(nome);

		modelPrimal.add (constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1]);
		modelPrimal.add (constraintsPrimal7[constraintsPrimal7.getSize()-1]);
		
		IloNumColumn colA = costPrimal(0);
		colA += constraintsPrimal6[k][constraintsPrimal6[k].getSize()-1](-1);
		colA += constraintsPrimal7[constraintsPrimal7.getSize()-1](-1);		
		sprintf(nome, "a_%d", (int)aPrimal.getSize());
		aPrimal.add(IloNumVar(colA, 0, 1, ILOFLOAT, nome));
		colA.end();
	}

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
	int ajuste = k*nCommodities;
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintsPrimal2[k](1);
	for (int i = 0; i < nCommodities; i++){
		if (vetVisitRota[i] > 0){
			col += constraintsPrimal4[i](vetVisitRota[i]);
			for ( int kForn = 0; kForn < maxVeic; ++kForn )
			{
				col += constraintsPrimal5[ajuste + kForn*maxVeic*nCommodities + i](vetVisitRota[i]);
			}
		}
	}
	gammaPrimal[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRotasCons[k];
	delete [] vetVisitRota;
}


strHash* ModeloCplex::getStrHashRota( int k, int& indice_a_r ){
	//procura pela ultima rota inserida (ptrRotasForn[k][qRotasForn]) no hashRotas,
	//caso a rota (na mesma sequencia de vertices, ou na sequencia inversa) já 
	//tenha sido precificada (mesmo que por outro veiculo) a estrutura q armazena esta rota eh retornada

	float custoRotaHash, custoRotaAtual = ptrRotasForn[k][qRotasForn[k]]->getCusto();
	vector < int > vertRotaHash, vertRotaAtual = ptrRotasForn[k][qRotasForn[k]]->getVertices();
	int tamRotaAtual = vertRotaAtual.size()-1;

	for ( int i = 0; i < vetorRotas.size(); ++i )
	{
		custoRotaHash = vetorRotas[i]->ptrRota->getCusto();
		if ( ( custoRotaAtual < ( custoRotaHash + 0.0001 ) ) && ( custoRotaAtual > ( custoRotaHash - 0.0001 ) ) )
		{
			//caso o custo da rota atual seja igual a de alguma com a mesma chave no hash, verifica os vertices, para verificar igualdade
			vertRotaHash = vetorRotas[i]->ptrRota->getVertices();
			if ( tamRotaAtual == ( vertRotaHash.size()-1 ) )
			{
				bool rotaMesmoSentido = true, rotaSentidoContrario = true;
				for (int j = 1; j < tamRotaAtual; ++j )
				{
					if ( vertRotaAtual[j] != vertRotaHash[j] ) rotaMesmoSentido = false;
					if ( vertRotaAtual[j] != vertRotaHash[tamRotaAtual - j] ) rotaSentidoContrario = false;
					if ( ( !rotaMesmoSentido ) && ( !rotaSentidoContrario ) ) break;
				}
				if ( rotaMesmoSentido || rotaSentidoContrario ) //significa q a rota ja existe no vetor, portanto, devo atualizar a restricao 6 e criar uma 7
				{
					indice_a_r = i;
					return vetorRotas[i];
				}
			}
		}
	}
	return NULL;
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

void ModeloCplex::imprimeRotasBasicas(){
	IloNumArray valuesF(env);
	IloNumArray valuesC(env);
	IloNumArray valuesT(env);
	for (int k = 0; k < maxVeic; ++k)
	{
		cplexPrimal.getValues(valuesF, lambdaPrimal[k]);
		for (int r = 0; r < qRotasForn[k]; ++r)
		{
			if (valuesF[r] >= 0.00001)
			{
				printf("lambda[%d][%d](%f): ", k, r, valuesF[r]);
				ptrRotasForn[k][r]->imprimir();
			}
		}
		cplexPrimal.getValues(valuesC, gammaPrimal[k]);
		for (int r = 0; r < qRotasCons[k]; ++r)
		{
			if (valuesC[r] >= 0.00001)
			{
				printf("gamma[%d][%d](%f): ", k, r, valuesC[r]);
				ptrRotasCons[k][r]->imprimir();
			}
		}
	}
	
	for (int kF = 0; kF < maxVeic; ++kF)
	{
		for (int kC = 0; kC < maxVeic; ++kC)
		{
			for (int i = 1; i <= nCommodities; ++i)
			{
				if (cplexPrimal.getValue(tauPrimal[kF][kC][i]) >= 0.00001)
				{
					printf("tau[%d][%d][%d](%f)\n", kF, kC, i, cplexPrimal.getValue(tauPrimal[kF][kC][i]));
				}
			}
		}
	}

	float soma = 0;
	for ( int i = 0; i < vetorRotas.size(); ++i )
	{
		printf("[ %d (%f) ]  -  ", i, cplexPrimal.getValue(aPrimal[i])); vetorRotas[i]->imprimir(maxVeic);
		soma += cplexPrimal.getValue(aPrimal[i]);
	}
	printf("soma_a_r = %f\n", soma);
}
