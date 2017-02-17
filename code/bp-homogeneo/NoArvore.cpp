#include "NoArvore.h"

Grafo* NoArvore::G;
ModeloCplex* NoArvore::mCplex;
int NoArvore::numRequests;
int NoArvore::numVertices;
int NoArvore::totalNosAtivos;
int NoArvore::totalNosCriados;

NoArvore::~NoArvore(){
	--totalNosAtivos;
	for (int r = 0; r < qRotas_no; ++r){
		delete [] a_ir_no[r];
		if (ptrRotas_no[r]->decrNumApontadores()) delete ptrRotas_no[r];
	}

	for (int i = 0; i < arcosBranching.size(); ++i){
		delete [] arcosBranching[i];
	}
}

NoArvore::NoArvore(ModeloCplex* modCplex, Grafo* g, vector < ptrNo >& nosCriados, float lDual){
	G = g;
	mCplex = modCplex;
	totalNosAtivos = 0;
	totalNosCriados = 0;
	numRequests = mCplex->nRequests;
	numVertices = 2*numRequests;
	limiteDual = lDual;
	qRotas_no = 0;
	indiceNo = 0;
	int i, j;

	//instancia os objetos do cplex para manipular as restricoes e variaveis do branching
	mCplex->lambda_no = IloNumVarArray(mCplex->env);
	mCplex->artificiais = IloNumVarArray(mCplex->env);
	mCplex->constraintsArvore = IloRangeArray(mCplex->env);

	//variaveis i e j passadas como referencia. armazenarao o arco a ser realizado o branching.
	if (defineVariavelBranching(i, j)) {
		short int* branch0 = new short int[3];
		branch0[0] = i; branch0[1] = j; branch0[2] = 0;
		nosCriados.push_back(new NoArvore(this, branch0));

		//nao precisa deletar branch0 e branch1, pois serao armazenadas pelos 
		//respectivos nos criados e serao deletadas em seus destrutores

		short int* branch1 = new short int[3];
		branch1[0] = i; branch1[1] = j; branch1[2] = 1;
		nosCriados.push_back(new NoArvore(this, branch1));
	}
}

NoArvore::NoArvore(ptrNo pai, short int* arcoBranching){
	short int *tmp;
	++totalNosAtivos;
	indiceNo = ++totalNosCriados;

	//copia (herda) as informacoes do pai
	qRotas_no = pai->qRotas_no;
	limiteDual = pai->limiteDual;
	for (int r = 0; r < qRotas_no; ++r) {
		tmp = new short int[numVertices+1];
		for (int i = 1; i <= numVertices; ++i) {
			tmp[i] = pai->a_ir_no[r][i];
		}
		a_ir_no.push_back(tmp);
		ptrRotas_no.push_back(pai->ptrRotas_no[r]);
		ptrRotas_no[r]->incrNumApontadores();
	}

	//herda os arcos fixos do veiculo k
	for (int i = 0; i < pai->arcosBranching.size(); i++) {
		tmp = new short int[3];
		tmp[0] = pai->arcosBranching[i][0];
		tmp[1] = pai->arcosBranching[i][1];
		tmp[2] = pai->arcosBranching[i][2];
		arcosBranching.push_back(tmp);
	}
	//por fim, inclue o arco de branching passado como parametro (o ultimo definido para executar o branching)
	arcosBranching.push_back(arcoBranching);
}

float NoArvore::executaBranching(vector<ptrNo>& nosCriados, char opSub){
	//limpa as informacoes do no processado anteriormente no ModeloCplex (variaveis de decisao, artificiais e restricoes da arvore)
	mCplex->lambda_no.endElements();
	mCplex->artificiais.endElements();
	mCplex->constraintsArvore.endElements();

	//insere no ModeloCplex as informacoes pertinentes ao no
	setVariaveisDecisao_no();
	setRestricoes_no();

	//Uma vez que o modelo deste no esta montado, eh possivel chegar na raiz atraves da geracao de colunas
	//ao fixar os arcos, eh possivel que o modelo se torne inviavel, neste caso, interrompe a geracao de filhos (poda por inviabilidade)
	if (alcancaRaiz_no(opSub))
	{
		//variaveis i e j passadas como referencia (arco a ser realizado o branching)
		int i, j;
		if (defineVariavelBranching(i, j))
		{
			//caso o limite dual do no seja maior ou igual ao limite o primal, nao gera os filhos (poda por limite dual)
			if (limiteDual < ModeloCplex::getLimitePrimal()){ 
				short int* branch0 = new short int[3];
				branch0[0] = i; branch0[1] = j; branch0[2] = 0;
				nosCriados.push_back(new NoArvore(this, branch0));

				//nao precisa deletar branch0 e branch1, pois serao armazenadas pelos 
				//respectivos nos criados e serao deletadas em seus destrutores

				short int* branch1 = new short int[3];
				branch1[0] = i; branch1[1] = j; branch1[2] = 1;
				nosCriados.push_back(new NoArvore(this, branch1));
			}
			return 999999;

		}else{ //caso nao existam variaveis a executar Branching (todas variaveis sao inteiras), nao cria-se filhos (poda por integralidade)
			return limiteDual;

		}
	}else{
		return 999999;
	}
}

void NoArvore::setVariaveisDecisao_no(){
	//para cada rota (matriz de vertices visitados) inclue uma rota no ModeloCplex
	for (int r = 0; r < qRotas_no; ++r) {
		IloNumColumn col = mCplex->costPrimal(ptrRotas_no[r]->getCusto());
		col += mCplex->constraintP1(1);

		for (int i = 1; i <= numVertices; i++){
			if (a_ir_no[r][i] > 0) col += mCplex->constraintsP2[i-1](a_ir_no[r][i]);
		}
		for (int i = 1; i <= numRequests; i++){
			if ((a_ir_no[r][i] == 0) && (a_ir_no[r][i+numRequests] > 0)) col += mCplex->constraintsP3[i-1](-1);
		}

		char nome[20];
		sprintf(nome, "lNo%d", r);
		mCplex->lambda_no.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
		col.end();
	}
}

void NoArvore::setRestricoes_no() {
	int vFixoI, vFixoJ, quantR, quantV, coefColuna, numVisitasArco;
	vector<int> verticesRota;
	int numRestrArcos = 0;

	for (int i = 0; i < arcosBranching.size(); ++i) {
		IloExpr exp = IloExpr(mCplex->env);
		vFixoI = arcosBranching[i][0];
		vFixoJ = arcosBranching[i][1];

		//insere na restricao as variaveis de decisao do no raiz (validas para qualquer no da arvore)
		quantR = mCplex->ptrRotas.size();
		for (int r = 0; r < quantR; ++r) {
			verticesRota = mCplex->ptrRotas[r]->getVertices();
			quantV = verticesRota.size()-2;
			numVisitasArco = 0;
			for (int i = 0; i < quantV; ++i) {
				if ((verticesRota[i] == vFixoI) && (verticesRota[i+1] == vFixoJ)) {
					++numVisitasArco;
				}
			}
			if (numVisitasArco > 0) exp += numVisitasArco * mCplex->lambda[r];
		}

		//insere na restricao as variaveis de decisao deste no
		quantR = ptrRotas_no.size();
		for (int r = 0; r < quantR; ++r) {
			verticesRota = ptrRotas_no[r]->getVertices();
			quantV = verticesRota.size()-2;
			numVisitasArco = 0;
			for (int i = 0; i < quantV; ++i) {
				if ((verticesRota[i] == vFixoI) && (verticesRota[i+1] == vFixoJ)) {
					++numVisitasArco;
				}
			}
			if (numVisitasArco > 0) exp += numVisitasArco * mCplex->lambda_no[r];
		}

		//a variavel coefColuna sera usada para controlar o indice da variavel artificial nesta restricao
		if (arcosBranching[i][2] == 1) {
			mCplex->constraintsArvore.add(exp == 1);
			coefColuna = 1;
		} else {
			mCplex->constraintsArvore.add(exp == 0);
			coefColuna = -1;
		}

		char nome[20];
		sprintf(nome, "a%d_%d", vFixoI, vFixoJ);
		mCplex->constraintsArvore[numRestrArcos].setName(nome);
		mCplex->modelPrimal.add(mCplex->constraintsArvore[numRestrArcos]);
		exp.end();

		//insere a variavel artificial associada a esta restricao (usando como custo o limite primal e o indice coefColuna)
		sprintf(nome, "artif%d", i);
		IloNumColumn col = mCplex->costPrimal(ModeloCplex::limitePrimal);
		col += mCplex->constraintsArvore[numRestrArcos](coefColuna);
		mCplex->artificiais.add(IloNumVar(col, 0, +IloInfinity, ILOFLOAT, nome));
		col.end();
		++numRestrArcos;
	}
}

bool NoArvore::alcancaRaiz_no(char opSub){
	int aux, tmp;
	bool alcancouRaiz;

	do{
		alcancouRaiz = true;
		mCplex->solveMaster();
		atualizaCustosDuais();

		if ( opSub == 'B' )
		{
			GRASP grasp(G, numRequests, numVertices+1, mCplex->getXi(), 0.5);
			aux = grasp.run(2000, mCplex->getAlfa());
			if (aux > 0)
			{
				alcancouRaiz = false;
				for (int h = 0; h < aux; ++h) inserirColuna_no(grasp.getRotaConvertida(h));
			}
			else
			{
				ModeloBC BCE(G, mCplex->getXi(), mCplex->getAlfa());
				BCE.calculaCaminhoElementar(G);
				aux = BCE.rotasNegativas.size();
				if (aux > 0) //significa que encontrou rota(s) de custo reduzido negativo
				{
					alcancouRaiz = false;
					for (int y = 0; y < aux; ++y) inserirColuna_no(BCE.rotasNegativas[y]);
				}
			}
		}
		else
		{
			ESPPRC *eee = new ESPPRC(G, mCplex->getXi(), mCplex->getAlfa());
			eee->calculaCaminhoElementar(G);
			Rota** rr = eee->getRotaCustoMinimo(G, 0.7);
			if ( rr != NULL )
			{
				alcancouRaiz = false;
				for ( int y = 0; rr[y] != NULL; ++y ) inserirColuna_no(rr[y]);
			}
			delete eee;
		}

	}while(!alcancouRaiz);

	limiteDual = mCplex->solveMaster();
	IloNumArray valoresVarArtif(mCplex->env);
	mCplex->cplexPrimal.getValues(valoresVarArtif, mCplex->artificiais);
	int numVarArtificiais = valoresVarArtif.getSize();
	for (int i = 0; i < numVarArtificiais; ++i)
	{
		if (valoresVarArtif[i] > 0.00001) return false;
	}
	return true;
}

void NoArvore::atualizaCustosDuais(){
	//insere nos vertices do grafo os valores das variaveis duais theta (como eh feito na raiz)
	IloNumArray dualValuesConstraintsP2(mCplex->env);
	mCplex->cplexPrimal.getDuals(dualValuesConstraintsP2, mCplex->constraintsP2);
	for (int i = 1; i <= numVertices; ++i){
		G->setCustoVerticeDual(i, -dualValuesConstraintsP2[i-1]);
	}
	G->setCustoArestasDual();

	//insere no grafo os custos duais associados aos arcos que foram fixados para este no
	float custoAresta;
	for (int i = 0; i < arcosBranching.size(); ++i){
		custoAresta = G->getCustoArestaDual(arcosBranching[i][0], arcosBranching[i][1]);
		G->setCustoArestaDual(arcosBranching[i][0], arcosBranching[i][1], (custoAresta - mCplex->cplexPrimal.getDual(mCplex->constraintsArvore[i])));
	}

	//armzena nas variaveis da classe os valores de alfa e xi, para serem retornados pelas funcoes 'get' (como eh feito na raiz)
	mCplex->alfaPtInterior = mCplex->cplexPrimal.getDual(mCplex->constraintP1);
	IloNumArray dualValuesConstraintsP3(mCplex->env);
	mCplex->cplexPrimal.getDuals(dualValuesConstraintsP3, mCplex->constraintsP3);
	for (int i = 0; i < numRequests; ++i) mCplex->xiPtInterior[i+1] = dualValuesConstraintsP3[i];
	mCplex->xiPtInterior[0] = 0;

	//libera memoria
	dualValuesConstraintsP2.end(); dualValuesConstraintsP3.end();
}

void NoArvore::inserirColuna_no(Rota* r){
	//Armazena a rota na matriz de rotas do no
	short int* rota = new short int[numVertices+1];
	memset(rota, 0, (numVertices+1) * sizeof(short int));
	a_ir_no.push_back(rota);
	ptrRotas_no.push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-1;
	for(int j = 1; j < numVertAtualizar; ++j){
		++a_ir_no[qRotas_no][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA NESTE NO
	IloNumColumn col = mCplex->costPrimal(r->getCusto());
	col += mCplex->constraintP1(1);
	for (int i = 1; i <= numVertices; i++){
		if (a_ir_no[qRotas_no][i] > 0){
			col += mCplex->constraintsP2[i-1](a_ir_no[qRotas_no][i]);
		}
	}
	for (int i = 1; i <= numRequests; i++){
		if ((a_ir_no[qRotas_no][i] == 0) && (a_ir_no[qRotas_no][i+numRequests] > 0)){
			col += mCplex->constraintsP3[i-1](-1);
		}
	}
	//Inclui na coluna os indices associados as restricoes de branching em arcos, caso existam
	int numVisitasArco;
	for (int i = 0; i < arcosBranching.size(); ++i){
		numVisitasArco = 0;
		for (int j = 0; j < numVertAtualizar; ++j){
			if ((vertRota[j] == arcosBranching[i][0]) && ((vertRota[j+1] == arcosBranching[i][1]))){
				++numVisitasArco;
			}
		}
		if (numVisitasArco > 0)	col += mCplex->constraintsArvore[i](numVisitasArco);
	}

	char nome[20];
	sprintf(nome, "lNo%d", qRotas_no);
	mCplex->lambda_no.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
	col.end();
	++qRotas_no;
}

bool NoArvore::defineVariavelBranching(int& inicioArco, int& fimArco){
	IloNumArray lambdaValues(mCplex->env);
	int tamRota, totalVertices = numVertices+2;
	vector<int> verticesRota;
	float violacaoLambda;
	
	//Cada celula da matriz representara um arco (i,j) que sera preenchido em funcao da violacao
	//Aquele arco (i,j) que tiver o maior valor de violacao sera fixado no branching de variaveis
	float** matrizViolacao = new float*[totalVertices];
	for (int i = 0; i < totalVertices; ++i) {
		matrizViolacao[i] = new float[totalVertices];
		memset(matrizViolacao[i], 0, (totalVertices) * sizeof(float));
	}

	//antes de procurar por arcos violados, deve-se assegurar que um arco que ja tenha sido fixado em iteracoes anteriores
	//nao seja fixado novamente. Isto eh necessario pois nas restricoes que fixam o arco em 1, pode ser que ainda existam
	//variaveis fracionarias (atribue o valor -100 a estes arcos na matriz para impedir que sejam escolhidos)
	for (int i = 0; i < arcosBranching.size(); ++i) {
		matrizViolacao[arcosBranching[i][0]][arcosBranching[i][1]] = -100; //garante q este arco nao sera mais fixado
	}

	//primeiro busca por variaveis fracionarias entre as variaveis de rotas obtidas na raiz
	mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda);
	for (int r = 0; r < mCplex->qRotas; ++r) {
		if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999)) { //lambda[r] da raiz eh fracionaria
			violacaoLambda = min(lambdaValues[r], (1-lambdaValues[r]));
			verticesRota = mCplex->ptrRotas[r]->getVertices();
			tamRota = verticesRota.size() - 2;
			for (int i = 0; i < tamRota; ++i) {
				matrizViolacao[verticesRota[i]][verticesRota[i+1]] += violacaoLambda;
			}
		}
	}

	//depois computa quais variaveis fracionarias do no
	if (qRotas_no > 0) {
		mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda_no);
		for (int r = 0; r < qRotas_no; ++r) {
			if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999)) {
				violacaoLambda = min(lambdaValues[r], (1-lambdaValues[r]));
				verticesRota = ptrRotas_no[r]->getVertices();
				tamRota = verticesRota.size() - 2;
				for (int i = 0; i < tamRota; ++i) {
					matrizViolacao[verticesRota[i]][verticesRota[i+1]] += violacaoLambda;
				}
			}
		}
	}

	//obtem o VALOR do(s) arco(s) mais violado(s)
	float maiorViolacao = 0.5, valorRealViolacao = -1;
	for (int i = 0; i < totalVertices; ++i){
		for (int j = 0; j < totalVertices; ++j){
			if (fabs(matrizViolacao[i][j] - 0.5) < (maiorViolacao - 0.0001)) {
				maiorViolacao = fabs(matrizViolacao[i][j] - 0.5);
				valorRealViolacao = matrizViolacao[i][j];
			}
		}
	}

	if (valorRealViolacao > 0){

		vector < int > arcosMaisViolados; //armazena aqueles arcos com maior valor de violacao (calculado anteriormente)
		vector < IloExpr > exprArcosMaisViolados; //cada arco que tenha valor igual ao mais violado tera uma expressao no cplex, para obter seu valor dual
		for (int i = 0; i < totalVertices; ++i){
			for (int j = 0; j < totalVertices; ++j){
				if ((fabs(matrizViolacao[i][j] - 0.5) > (maiorViolacao-0.0001)) && 
					(fabs(matrizViolacao[i][j] - 0.5) < (maiorViolacao+0.0001))){ //arco (i,j) tem maior violacao					
					arcosMaisViolados.push_back(i);
					arcosMaisViolados.push_back(j);
					exprArcosMaisViolados.push_back(IloExpr(mCplex->env));
				}
			}
		}

		if (exprArcosMaisViolados.size() > 1) { //existem arcos com o mesmo valor de violacao. o desempate usara os valores dos custos reduzidos
			//busca novamente as variaveis de decisao fracionarias, mas desta vez para compor as restricoes (\sum{arcoViolado}lambda = maiorViolacao)
			int aux, numArcosMaisViolados = exprArcosMaisViolados.size();
			mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda); //primeiro as variaveis de decisao lambda da raiz
			for (int r = 0; r < mCplex->qRotas; ++r) {
				if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999)) { //variavel eh fracionaria
					verticesRota = mCplex->ptrRotas[r]->getVertices();
					tamRota = verticesRota.size() - 2;
					aux = 0;
					for (int i = 0; i < 2*numArcosMaisViolados; i+=2, ++aux){
						for (int j = 0; j < tamRota; ++j) {
							if ((arcosMaisViolados[i] == verticesRota[j]) && (arcosMaisViolados[i+1] == verticesRota[j+1])){
								exprArcosMaisViolados[aux] += mCplex->lambda[r]; break;
							}
						}
					}
				}
			}

			if (qRotas_no > 0) {
				mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda_no); //depois as variaveis de decisao lambda apenas deste no
				for (int r = 0; r < qRotas_no; ++r) {
					if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999)) { //lambda[r] da raiz eh fracionaria
						verticesRota = ptrRotas_no[r]->getVertices();
						tamRota = verticesRota.size() - 2;
						aux = 0;
						for (int i = 0; i < 2*numArcosMaisViolados; i+=2, ++aux){
							for (int j = 0; j < tamRota; ++j) {
								if ((arcosMaisViolados[i] == verticesRota[j]) && (arcosMaisViolados[i+1] == verticesRota[j+1])) {
									exprArcosMaisViolados[aux] += mCplex->lambda_no[r]; break;
								}
							}
						}
					}
				}
			}

			//RELAXA A RESTRICAO DE CONVEXIDADE, DEIXANDO-A PARA AS NOVAS RESTRICOES A SEREM INSERIDAS
			mCplex->constraintP1.setBounds(0, 2*mCplex->maxVeic);
			IloRangeArray constraintsSetArcoBranching(mCplex->env);
			for (int i = 0; i < numArcosMaisViolados; ++i) {
				constraintsSetArcoBranching.add(exprArcosMaisViolados[i] == valorRealViolacao);
			}
			mCplex->modelPrimal.add(constraintsSetArcoBranching);
			mCplex->solveMaster();

			//obtidas as expressoes, cria-se as restricoes, igualando todas ao valor do arco mais violado (pois elas estao empatadas neste valor)
			float maiorCustoDual = -MAIS_INFINITO;
			mCplex->cplexPrimal.getDuals(lambdaValues, constraintsSetArcoBranching);
			for (int i = 0; i < numArcosMaisViolados; ++i) {
				if (fabs(lambdaValues[i]) > maiorCustoDual) {
					maiorCustoDual = fabs(lambdaValues[i]);
					aux = i;
				}
			}

			//finalmente atribue o inicio e o fim do arco para realizar o branching e apaga as restricoes criadas do master
			inicioArco = arcosMaisViolados[2*aux]; 
			fimArco = arcosMaisViolados[(2*aux)+1];
			constraintsSetArcoBranching.endElements();
			constraintsSetArcoBranching.end();

			//retorna a restricao de convexidade relaxada para obter informacoes duais das novas restricoes
			mCplex->constraintP1.setBounds(mCplex->maxVeic, mCplex->maxVeic);

		}else{ //existe apenas um arco cujo valor eh o mais violado, portanto, este arco sera retornado para o branching
			inicioArco = arcosMaisViolados[0];
			fimArco = arcosMaisViolados[1];
		}
	}

	//libera memoria
	for (int i = 0; i < totalVertices; i++) delete [] matrizViolacao[i];
	delete [] matrizViolacao;
	lambdaValues.end();

	if ((maiorViolacao > 0.4999) && (maiorViolacao < 0.5001)) return false; //solucao inteira, nao executa-se branching e poda por integralidade
	else return true; //existem arcos para executar branching
}

void NoArvore::imprimir() {
	printf("indice = %d\n", indiceNo);
	printf("valor = %f\n", limiteDual);
	printf("qRotas_no = %d\n", qRotas_no);
	for (int r = 0; r < qRotas_no; ++r) {
		printf(" ");
		ptrRotas_no[r]->imprimir();
	}
	printf("Arcos com restricao:\n");
	for (int i = 0; i < arcosBranching.size(); ++i) {
		printf("  (%d, %d) = %d\n", arcosBranching[i][0], arcosBranching[i][1], arcosBranching[i][2]);
	}
	printf("\n");
}

void NoArvore::setProx(ptrNo p){
	prox = p;
}

ptrNo NoArvore::getProx(){
	return prox;
}

float NoArvore::getLimiteDual(){
	return limiteDual;
}

int NoArvore::getIndiceNo(){
	return indiceNo;
}

int NoArvore::getTotalNosCriados(){
	return totalNosCriados;
}

int NoArvore::getTotalNosAtivos(){
	return totalNosAtivos;
}
