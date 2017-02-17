#include "NoArvore.h"

//Inicializacao dos MEMBROS ESTATICOS, necessaria antes de atribuir valor a estes membros
int NoArvore::maxV;
int NoArvore::numCommod;
int NoArvore::totalNosCriados;
int NoArvore::numPontosExtremos;
int NoArvore::totalNosArmazenados;
vector<short int*>* NoArvore::matrizA_raiz;
vector<short int*>* NoArvore::matrizB_raiz;

NoArvore::~NoArvore(){
	for (int k = 0; k < maxV; ++k){
		for (int r = 0; r < qRFornNo[k]; ++r){
			delete [] matrizA_no[k][r];
			if (ptrRForn_no[k][r]->decrNumApontadores()) delete ptrRForn_no[k][r];
		}
		for (int r = 0; r < qRConsNo[k]; ++r){
			delete [] matrizB_no[k][r];
			if (ptrRCons_no[k][r]->decrNumApontadores()) delete ptrRCons_no[k][r];
		}
		matrizA_no[k].clear();
		matrizB_no[k].clear();
		ptrRForn_no[k].clear();
		ptrRCons_no[k].clear();
		delete [] variaveisComRestricao[k];
	}
	delete [] qRFornNo;
	delete [] qRConsNo;
	delete [] variaveisComRestricao;
	delete [] matrizA_no;
	delete [] matrizB_no;
	delete [] ptrRForn_no;
	delete [] ptrRCons_no;
	--totalNosArmazenados;
}

NoArvore::NoArvore(ModeloCplex& mCplex, int nPontosExt){
	//Inicializa os atributos estaticos da classe
	indiceNo = 1;
	totalNosCriados = 1;
	totalNosArmazenados = 0;
	maxV = mCplex.maxVeic;
	numCommod = mCplex.nCommodities;
	numPontosExtremos = nPontosExt;
	matrizA_raiz = mCplex.A_qr;
	matrizB_raiz = mCplex.B_qr;

	//inicializa os atributos do objeto
	qRFornNo = new int[maxV];
	qRConsNo = new int[maxV];
	variaveisComRestricao = new char*[maxV];
	matrizA_no = new vector<short int*>[maxV];
	matrizB_no = new vector<short int*>[maxV];
	ptrRForn_no = new vector<Rota*>[maxV];
	ptrRCons_no = new vector<Rota*>[maxV];
	for (int k = 0; k < maxV; ++k){
		qRFornNo[k] = 0;
		qRConsNo[k] = 0;
		//Aloca memoria para a matriz de pares com restricoes; Nenhum par tem restricoes no primeiro no
		variaveisComRestricao[k] = new char[numCommod+1];
		memset (variaveisComRestricao[k], 0, (numCommod+1)*sizeof(char));
	}

	//Aloca a memoria dos PONTEIROS para as variaveis e restricoes da arvore
	mCplex.constraintsArvoreLambda = IloRangeArray(mCplex.env);
	mCplex.constraintsArvoreGamma = IloRangeArray(mCplex.env);
	mCplex.artificiais = IloNumVarArray(mCplex.env);
	mCplex.lambdaNo = IloArray <IloNumVarArray> (mCplex.env, maxV);
	mCplex.gammaNo = IloArray <IloNumVarArray> (mCplex.env, maxV);
	for (int k = 0; k < maxV; k++){
		mCplex.lambdaNo[k] = IloNumVarArray(mCplex.env);
		mCplex.gammaNo[k] = IloNumVarArray(mCplex.env);
	}

	//Aloca a memoria dos PONTEIROS para as variaveis e restricoes duais,
	//necessarios para se usar a estabilizacao por pontos interiores
	mCplex.constraintsDual1_no = IloArray<IloRangeArray> (mCplex.env, maxV);
	mCplex.constraintsDual2_no = IloArray<IloRangeArray> (mCplex.env, maxV);
	mCplex.varDualArvoreLambda = IloArray<IloNumVarArray> (mCplex.env, maxV);
	mCplex.varDualArvoreGamma = IloArray<IloNumVarArray> (mCplex.env, maxV);
	for (int k = 0; k < maxV; ++k){
		mCplex.constraintsDual1_no[k] = IloRangeArray(mCplex.env);
		mCplex.constraintsDual2_no[k] = IloRangeArray(mCplex.env);
		mCplex.varDualArvoreLambda[k] = IloNumVarArray(mCplex.env); 
		mCplex.varDualArvoreGamma[k] = IloNumVarArray(mCplex.env);
	}
	mCplex.constraintsDualArtificiais = IloRangeArray(mCplex.env);

	//finalmenete atribui valores as variaveis k_branching e q_branching do primeiro no da arvore
	procuraVariavelBranching(mCplex);
}

NoArvore::NoArvore(ptrNo pai, ModeloCplex& mCplex, int iFilho){
	++totalNosArmazenados;
	indiceNo = ++totalNosCriados;
	qRFornNo = new int[maxV];
	qRConsNo = new int[maxV];

	//Aloca a matriz que guarda o historico dos vertices com restricoes do No. A matriz tera 'k' linhas
	//e 'q' colunas, sendo que cada celula armazenara um dos 5 valores (1),(2),(3),(4) indicando qual restricao
	//de branching foi aplicada a mercadoria q e veiculo k, e (0) indicando que a mercadoria q e veiculo k
	//ainda nao foi fixada em uma restricao de branching. A matriz eh inicializada com os valores do pai
	variaveisComRestricao = new char*[maxV];

	//Aloca memoria para armazenar a matrizA_no e matrizB_no e também as matrizes de custos de cada rota
	matrizA_no = new vector<short int*>[maxV];
	matrizB_no = new vector<short int*>[maxV];
	ptrRForn_no = new vector<Rota*>[maxV];
	ptrRCons_no = new vector<Rota*>[maxV];
	short int* rota; //esse ponteiro pendurara a memoria em cada celula da matriz

	//O laco abaixo preenche o NO com os valores das rotas do pai, criando uma variavel de decisao para cada rota do pai
	for (int k = 0; k < maxV; ++k){
		qRFornNo[k] = pai->qRFornNo[k];
		qRConsNo[k] = pai->qRConsNo[k];

		variaveisComRestricao[k] = new char[numCommod+1];
		for (int j = 0; j <= numCommod; ++j){
			variaveisComRestricao[k][j] = pai->variaveisComRestricao[k][j];
		}

		//Insere em matrizA_no todas as entradas existentes no pai para os fornecedores
		for (int r = 0; r < qRFornNo[k]; ++r){
			rota = new short int[numCommod+1];
			matrizA_no[k].push_back(rota);
			for (int i = 1; i <= numCommod; ++i){
				matrizA_no[k][r][i] = pai->matrizA_no[k][r][i];
			}
			ptrRForn_no[k].push_back(pai->ptrRForn_no[k][r]);
			ptrRForn_no[k][r]->incrNumApontadores();

			//cria a variavel de decisao associada a esta rota no primal e tambem cria uma restricao dual
			//associada a esta variavel e insere ambas nos modelos primal e dual
			IloExpr res = mCplex.alfaDual[k];
			IloNumColumn col = mCplex.costPrimal(ptrRForn_no[k][r]->getCusto());
			col += mCplex.constraintsPrimal1[k](1);
			for (int i = 1; i <= numCommod; i++){
				if (matrizA_no[k][r][i] > 0){
					col += mCplex.constraintsPrimal3[i-1](matrizA_no[k][r][i]);
					col += mCplex.constraintsPrimal5[k*numCommod + i-1](matrizA_no[k][r][i]);
					col += mCplex.constraintsPrimal6[k*numCommod + i-1](-matrizA_no[k][r][i]);
					
					res += matrizA_no[k][r][i] * mCplex.thetaDual[i]
						  + matrizA_no[k][r][i] * mCplex.piDual[k][i]
						  - matrizA_no[k][r][i] * mCplex.xiDual[k][i];
				}
			}
			//insere a variavel de decisao no modelo primal
			mCplex.lambdaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
			col.end();

			//insere a restricao no modelo dual
			mCplex.constraintsDual1_no[k].add(res <= ptrRForn_no[k][r]->getCusto());
			mCplex.modelDual.add(mCplex.constraintsDual1_no[k][r]);
			res.end();
		}

		//Insere em matrizB_no todas as entradas existentes no pai para os consumidores
		for (int r = 0; r < qRConsNo[k]; ++r){
			rota = new short int[numCommod+1];
			matrizB_no[k].push_back(rota);
			for (int i = 1; i <= numCommod; ++i){
				matrizB_no[k][r][i] = pai->matrizB_no[k][r][i];
			}
			ptrRCons_no[k].push_back(pai->ptrRCons_no[k][r]);
			ptrRCons_no[k][r]->incrNumApontadores();

			//cria a variavel de decisao associada a esta rota e a insere no modelo
			IloExpr res = mCplex.betaDual[k];
			IloNumColumn col = mCplex.costPrimal(ptrRCons_no[k][r]->getCusto());
			col += mCplex.constraintsPrimal2[k](1);
			for (int i = 1; i <= numCommod; i++){
				if (matrizB_no[k][r][i] > 0){
					col += mCplex.constraintsPrimal4[i-1](matrizB_no[k][r][i]);
					col += mCplex.constraintsPrimal5[k*numCommod + i-1](-matrizB_no[k][r][i]);
					col += mCplex.constraintsPrimal6[k*numCommod + i-1](matrizB_no[k][r][i]);
					
					res += matrizB_no[k][r][i] * mCplex.muDual[i]
						  - matrizB_no[k][r][i] * mCplex.piDual[k][i]
						  + matrizB_no[k][r][i] * mCplex.xiDual[k][i];
				}
			}
			//insere a variavel de decisao no modelo primal
			mCplex.gammaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
			col.end();

			//insere a restricao no modelo dual
			mCplex.constraintsDual2_no[k].add(res <= ptrRCons_no[k][r]->getCusto());
			mCplex.modelDual.add(mCplex.constraintsDual2_no[k][r]);
			res.end();
		}
	}

	//insere neste NO mais uma variavel com restricao. Variavel selecionada atraves do metodo procuraVariavelBranching
	variaveisComRestricao[pai->k_branching][pai->q_branching] = iFilho;

	//termina de montar o modelo deste NO ao adicionar as restricoes da arvore e as variaveis artificiais
	setRestricoesArvore(mCplex);

	//otimiza o modelo para que seja possivel obter os valores das variaveis de decisao e duais
	mCplex.cplexPrimal.solve();
}

void NoArvore::procuraVariavelBranching(ModeloCplex& mCplex){
	IloNumArray lambdaValues(mCplex.env);
	IloNumArray gammaValues(mCplex.env);
	k_branching = 0; q_branching = -1;
	bool solucaoInteira = true;
	
	//Declara e inicializa a matriz que armazenara a violacao (o quanto esta distante de uma solucao inteira)
	//para todos os possiveis pares [k,q] do problema. Aquele mais violado sera fixado para o branching do no
	float** totalViolacao = new float*[maxV];
	for (int k = 0; k < maxV; k++){
		totalViolacao[k] = new float[numCommod+1];
		memset (totalViolacao[k], 0, (numCommod+1)*sizeof(float));
	}

	//primeiro busca por variaveis fracionarias entre as variaveis de rotas de fornecedores obtidas na raiz
	for(int k = 0; k < maxV; ++k){
		mCplex.cplexPrimal.getValues(lambdaValues, mCplex.lambdaPrimal[k]);
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r){
			if ((lambdaValues[r] > 0) && (lambdaValues[r] < 1)){
				solucaoInteira = false;
				for (int i = 1; i <= numCommod; ++i){
					if ((matrizA_raiz[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0)){
						if (lambdaValues[r] <= 0.5){
							totalViolacao[k][i] += lambdaValues[r];
						}else{
							totalViolacao[k][i] += (1 - lambdaValues[r]);
						}
					}
				}
			}
		}
	}
	//depois busca por variaveis fracionarias entre as variaveis de rotas de fornecedores obtidas neste no
	for(int k = 0; k < maxV; ++k){
		if (qRFornNo[k] > 0){
			mCplex.cplexPrimal.getValues(lambdaValues, mCplex.lambdaNo[k]);
			for (int r = 0; r < qRFornNo[k]; ++r){
				if ((lambdaValues[r] > 0) && (lambdaValues[r] < 1)){
					solucaoInteira = false;
					for (int i = 1; i <= numCommod; ++i){
						if ((matrizA_no[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0)){
							if (lambdaValues[r] <= 0.5){
								totalViolacao[k][i] += lambdaValues[r];
							}else{
								totalViolacao[k][i] += (1 - lambdaValues[r]);
							}
						}
					}
				}
			}
		}
	}
	//finaliza a busca por variaveis fracionarias relacionadas aos fornecedores
	lambdaValues.end();

	//busca por variaveis fracionarias entre as variaveis de rotas de consumidores obtidas na raiz
	for(int k = 0; k < maxV; ++k){
		mCplex.cplexPrimal.getValues(gammaValues, mCplex.gammaPrimal[k]);
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r){
			if ((gammaValues[r] > 0) && (gammaValues[r] < 1)){
				solucaoInteira = false;
				for (int i = 1; i <= numCommod; ++i){
					if ((matrizB_raiz[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0)){
						if (gammaValues[r] <= 0.5){
							totalViolacao[k][i] += gammaValues[r];
						}else{
							totalViolacao[k][i] += (1 - gammaValues[r]);
						}
					}
				}
			}
		}
	}
	//faz a mesma busca, mas agora nas variaveis de roats de consumidores do no
	for(int k = 0; k < maxV; ++k){
		if (qRConsNo[k] > 0){
			mCplex.cplexPrimal.getValues(gammaValues, mCplex.gammaNo[k]);
			for (int r = 0; r < qRConsNo[k]; ++r){
				if ((gammaValues[r] > 0) && (gammaValues[r] < 1)){
					solucaoInteira = false;
					for (int i = 1; i <= numCommod; ++i){
						if ((matrizB_no[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0)){
							if (gammaValues[r] <= 0.5){
								totalViolacao[k][i] += gammaValues[r];
							}else{
								totalViolacao[k][i] += (1 - gammaValues[r]);
							}
						}
					}
				}
			}
		}
	}
	//termina a busca por variaveis de rota fracionarias
	gammaValues.end();

	if (solucaoInteira == true){ //significa que lambda e gamma sao inteiros. Verifica se tau tambem eh inteiro
		q_branching = 0;
		for (int k = 0; k < maxV; ++k){
			for (int q = 1; ((q <= numCommod) && (q_branching == 0)); ++q){
				if ((mCplex.cplexPrimal.getValue(mCplex.tauPrimal[k][q]) > 0) && (mCplex.cplexPrimal.getValue(mCplex.tauPrimal[k][q]) < 1)){
					q_branching = -1; //nao existem variaveis [k,q] para realizar o branching e a solucao nao eh inteira
				}
			}
		}
	}else{ //significa que existem variavei(s) fracionaria(s): atribui o par [k,q] mais violado para k e q branching
		float maisViolado = 0;
		for (int k = 0; k < maxV; ++k){
			for (int i = 1; i <= numCommod; ++i){
				if ((totalViolacao[k][i] > maisViolado) && (variaveisComRestricao[k][i] == 0)){
					maisViolado = totalViolacao[k][i];
					k_branching = k;
					q_branching = i;
				}
			}
		}
		if (maisViolado == 0){
			q_branching = -1;
		}
	}	 

	//desaloca a matriz totalViolacao
	for (int k = 0; k < maxV; k++){
		delete [] totalViolacao[k];
	}
	delete [] totalViolacao;
}
	
ptrNo NoArvore::geraFilho(ModeloCplex& mCplex, Grafo* G, int indiceFilho, char opCam, bool& segBranching, bool& bestPrimal){
	//antes de invocar o construtor para inicializar o filho, verifica se o filho a ser criado sera viavel
	if (!this->verificaFilhoViavel(G, indiceFilho)){
		return NULL;
	}

	bestPrimal = false;
	segBranching = false;
	ptrNo filhoCriado = new NoArvore(this, mCplex, indiceFilho);

	//A otimizacao de um No filho nunca eh inviavel. O que pode acontecer eh de variaveis artificiais do NO
	//estarem na base apos a otimizacao, o que significa que este No nao apresenta solucao para o problema e deve 
	//ser descartado, mas para isto deve-se alcancar a raiz deste No e verificar sua base
	if (filhoCriado->alcancaRaizNo(mCplex, G, opCam)){
		//Caso o no nao contenha variaveis artificiais na base, verifica se ele tem solucao inteira ou fracionaria
		//no caso da solucao ser inteira verifica se o limite primal eh menor do que o atual e atualiza
		//caso a solucao seja fracionaria, verifica qual sera a variavel a ser feito o branching nos filhos deste no (proxima iteracao)
		filhoCriado->procuraVariavelBranching(mCplex);

		if (filhoCriado->q_branching > 0){ //solucao fracionaria passível de ser fixada no branching
			filhoCriado->limiteDual = mCplex.cplexPrimal.getObjValue();
			if (filhoCriado->limiteDual < ModeloCplex::limitePrimal){
				return filhoCriado;
			}else{
				delete filhoCriado;
				return NULL;
			}

		}else if (filhoCriado->q_branching == 0){ //solucao inteira (PODA POR OTIMALIDADE)

			filhoCriado->limiteDual = mCplex.cplexPrimal.getObjValue();
			if (filhoCriado->limiteDual < ModeloCplex::limitePrimal){
				ModeloCplex::limitePrimal = filhoCriado->limiteDual;
				bestPrimal = true;
				return filhoCriado;
			}else{
				delete filhoCriado; 
				return NULL;
			}

		}else{ 	//a solucao eh fracionaria, mas nao eh possivel realizar o branching ou por que as variaveis 
					//fracionarias ja foram restritas em iteracoes anteriores, ou principalmente por que a fixação
					//da célula [k,q] na matriz geraria um no que não levaria a solução otima.
					//O NO SERA ARMAZENADO PARA REALIZAR O BRANCHING DE VARIAVEIS NOS ARCOS, CASO SEU LIMITE DUAL PERMITA
			filhoCriado->limiteDual = mCplex.cplexPrimal.getObjValue();
			if (filhoCriado->limiteDual < ModeloCplex::limitePrimal){
				segBranching = true;
				return filhoCriado;
			}else{
				delete filhoCriado;
				return NULL;
			}
		}
	}else{
		delete filhoCriado;
		return NULL;
	}
}

bool NoArvore::alcancaRaizNo(ModeloCplex& mCplex, Grafo* G, char opcaoCaminho){
	Rota** colunas;
	bool pricingFull, alcancouRaiz = false;
	float valorSubtrair;
	int* pricingFullForn = new int[maxV];
	int* pricingFullCons = new int[maxV];
	int numPricingFull, k, limVeic, iVeicForn = 0, iVeicCons = 0;

	do{
		pricingFull = alcancouRaiz;
		alcancouRaiz = true;
		//atualiza o grafo com os custos duais dos vertices, calcula as rotas e as insere no master restrito
		limVeic = iVeicForn + maxV;
		for (int k = iVeicForn; k < limVeic; ++k, ++iVeicForn){
			if (pricingFullForn[(k % maxV)] != -1){
			
				if (NoArvore::numPontosExtremos > 0){			
					valorSubtrair = atualizaCustosDuaisPontosInterioresForn(mCplex, G, (k % maxV));
				}else{
					atualizaCustosDuaisForn(mCplex, G, (k % maxV));
					valorSubtrair = mCplex.cplexPrimal.getDual(mCplex.constraintsPrimal1[(k % maxV)]);
				}

				colunas = retornaColuna(G, (k % maxV), true, pricingFull, valorSubtrair, opcaoCaminho, pricingFullForn[(k % maxV)]);
				if (colunas != NULL){
					for (int i = 0; colunas[i] != NULL; ++i){
						inserirColunaForn(mCplex, colunas[i], (k % maxV));
					}
					
					//Retorna os valores dos vetores para que todos computem os Subproblemas
					for (int v = 0; v < maxV; ++v){
						pricingFullForn[v] = pricingFullCons[v] = 1;
					}
					
					alcancouRaiz = false;
					pricingFull = false;
					delete [] colunas;
					++iVeicForn;
					mCplex.cplexPrimal.solve();
					break;
				}
			}
		}
		limVeic = iVeicCons + maxV;

		for (int k = iVeicCons; k < limVeic; ++k, ++iVeicCons){
			if (pricingFullCons[(k % maxV)] != -1){
			
				if (NoArvore::numPontosExtremos > 0){			
					valorSubtrair = atualizaCustosDuaisPontosInterioresCons(mCplex, G, (k % maxV));
				}else{
					atualizaCustosDuaisCons(mCplex, G, (k % maxV));
					valorSubtrair = mCplex.cplexPrimal.getDual(mCplex.constraintsPrimal2[(k % maxV)]);
				}

				colunas = retornaColuna(G, (k % maxV), false, pricingFull, valorSubtrair, opcaoCaminho, pricingFullCons[(k % maxV)]);
				if (colunas != NULL){
					for (int i = 0; colunas[i] != NULL; ++i){
						inserirColunaCons(mCplex, colunas[i], (k % maxV));
					}
					
					//Retorna os valores dos vetores para que todos computem os Subproblemas
					for (int v = 0; v < maxV; ++v){
						pricingFullForn[v] = pricingFullCons[v] = 1;
					}
					
					alcancouRaiz = false;
					delete [] colunas;
					++iVeicCons;
					mCplex.cplexPrimal.solve();
					break;
				}
			}
		}

		numPricingFull = 0;
		for (int v = 0; v < maxV; ++v){
			numPricingFull += pricingFullForn[v];
			numPricingFull += pricingFullCons[v];
		}
		
	}while((!alcancouRaiz) || (numPricingFull != (-2*maxV)));

	IloNumArray valoresVarArtif (mCplex.env);
	mCplex.cplexPrimal.getValues(valoresVarArtif, mCplex.artificiais);
	int numVarArtificiais = valoresVarArtif.getSize();
	for (int i = 0; i < numVarArtificiais; ++i){
		if (valoresVarArtif[i] >= 0.00001){
			return false;
		}
	}
	return true;
}

void NoArvore::setRestricoesArvore(ModeloCplex& mCplex){
	setRestricoesArvoreTau(mCplex);
	setRestricoesArvoreLambdaGamma(mCplex);
}

void NoArvore::setRestricoesArvoreTau(ModeloCplex& mCplex){
	//as restricoes tau0 e tau1 darao origem a duas variaveis de decisao no dual
	int count = 0;
	IloNumColumn colTau0 = IloNumColumn(mCplex.env);
	IloNumColumn colTau1 = IloNumColumn(mCplex.env);

	//RESTRICOES QUE FIXAM TAU EM ZERO (variaveisComRestricao[k][i] = {1,2}) E UM (variaveisComRestricao[k][i] = {3,4})	
	int numRestrIgual1 = 0;
	IloExpr exp0 = IloExpr(mCplex.env);
	IloExpr exp1 = IloExpr(mCplex.env);
	for (int k = 0; k < maxV; ++k){
		for (int i = 1; i <= numCommod; ++i){
			if ((variaveisComRestricao[k][i] == 1) || (variaveisComRestricao[k][i] == 2)){
				colTau0 += mCplex.constraintsDual3[count](1);
				exp0 += mCplex.tauPrimal[k][i];
			}else if ((variaveisComRestricao[k][i] == 3) || (variaveisComRestricao[k][i] == 4)){
				colTau1 += mCplex.constraintsDual3[count](1);
				exp1 += mCplex.tauPrimal[k][i];
				++numRestrIgual1;
			}
			++count;
		}
	}
	mCplex.constraintArvoreTau0 = (exp0 == 0);
	mCplex.constraintArvoreTau1 = (exp1 == numRestrIgual1);
	mCplex.modelPrimal.add(mCplex.constraintArvoreTau0);
	mCplex.modelPrimal.add(mCplex.constraintArvoreTau1);
	exp0.end();
	exp1.end();
	
	//insere 2 VARIAVEIS ARTIFICIAIS associadas a estas 2 RESTRICOES
	IloNumColumn colA1 = mCplex.costPrimal(ModeloCplex::limitePrimal);
	colA1 += mCplex.constraintArvoreTau0(-1);
	mCplex.artificiais.add(IloNumVar(colA1, 0, +IloInfinity, ILOFLOAT));
	IloNumColumn colA2 = mCplex.costPrimal(ModeloCplex::limitePrimal);
	colA2 += mCplex.constraintArvoreTau1(1);
	mCplex.artificiais.add(IloNumVar(colA2, 0, +IloInfinity, ILOFLOAT));
	colA1.end();
	colA2.end();

	//Aproveita para inserir no modelo dual as variaveis de decisao associadas a estas restricoes primais
	//e tambem insere duas restricoes, associadas as duas variaveis artificiais inseridas no primal
	mCplex.varDualArvoreTau0 = IloNumVar(colTau0, -IloInfinity, +IloInfinity, ILOFLOAT);
	mCplex.varDualArvoreTau1 = IloNumVar(colTau1, -IloInfinity, +IloInfinity, ILOFLOAT);

	IloExpr expTau0 = -mCplex.varDualArvoreTau0;
	IloExpr expTau1 = mCplex.varDualArvoreTau1;
	mCplex.constraintsDualArtificiais.add(expTau0 <= ModeloCplex::limitePrimal);
	mCplex.constraintsDualArtificiais.add(expTau1 <= ModeloCplex::limitePrimal);
	mCplex.modelDual.add(mCplex.constraintsDualArtificiais[0]);
	mCplex.modelDual.add(mCplex.constraintsDualArtificiais[1]);
	expTau0.end();
	expTau1.end();
}

void NoArvore::setRestricoesArvoreLambdaGamma(ModeloCplex& mCplex){
	int indiceRestricaoArtificial = 2;
	
	//colunas para criar variaveis duais associadas as restricoes primais
	IloNumColumn colDualL = IloNumColumn(mCplex.env);
	IloNumColumn colDualG = IloNumColumn(mCplex.env);

	//RESTRICOES QUE FIXAM AS VARIAVEIS LAMBDA PARA ALGUNS NOS (que tiverem variaveisComRestricao[k][i] > 0)
	//RESTRICOES QUE FIXAM AS VARIAVEIS GAMMA PARA ALGUNS NOS (que tiverem variaveisComRestricao[k][i] > 0)
	//UMA VARIAVEL ARTIFICIAL ASSOCIADA A CADA RESTRICAO CRIADA (COEFICIENTE 1 APENAS NA RESTRICAO E OBJETIVO = limitePrimal)
	int limite, coefColunaArtificial, numColsDualL_k, numColsDualG_k, numRestrLambda = 0, numRestrGamma = 0;
	for (int k = 0; k < maxV; ++k){
		numColsDualL_k = 0;
		numColsDualG_k = 0;
		for (int i = 1; i <= numCommod; ++i){
			if (variaveisComRestricao[k][i] > 0){ //Insere a restricao fixando lambda
				IloExpr expL = IloExpr(mCplex.env);
				limite = matrizA_raiz[k].size();
				for (int r = 0; r < limite; ++r){
					if (matrizA_raiz[k][r][i] > 0){
						expL += matrizA_raiz[k][r][i] * mCplex.lambdaPrimal[k][r];
						colDualL += mCplex.constraintsDual1[k][r](matrizA_raiz[k][r][i]);
					}
				}
				for (int r = 0; r < qRFornNo[k]; ++r){
					if (matrizA_no[k][r][i] > 0){
						expL += matrizA_no[k][r][i] * mCplex.lambdaNo[k][r];
						colDualL += mCplex.constraintsDual1_no[k][r](matrizA_no[k][r][i]);
					}
				}
				if ((variaveisComRestricao[k][i] == 1) || (variaveisComRestricao[k][i] == 3)){
					mCplex.constraintsArvoreLambda.add(expL == 1);
					coefColunaArtificial = 1;
				}else{
					mCplex.constraintsArvoreLambda.add(expL == 0);
					coefColunaArtificial = -1;
				}
				mCplex.modelPrimal.add(mCplex.constraintsArvoreLambda[numRestrLambda]);
				expL.end();
				
				mCplex.varDualArvoreLambda[k].add(IloNumVar(colDualL, -IloInfinity, +IloInfinity, ILOFLOAT));
				colDualL.clear();

				//insere a variavel artificial associada a esta restricao
				IloNumColumn colL = mCplex.costPrimal(ModeloCplex::limitePrimal);
				colL += mCplex.constraintsArvoreLambda[numRestrLambda](coefColunaArtificial);
				mCplex.artificiais.add(IloNumVar(colL, 0, +IloInfinity, ILOFLOAT));
				colL.end();

				//insere a restricao dual associada a variavel artificial que acaba de ser criada
				IloExpr expArtifG = coefColunaArtificial * mCplex.varDualArvoreLambda[k][numColsDualL_k];
				mCplex.constraintsDualArtificiais.add(expArtifG <= ModeloCplex::limitePrimal);
				mCplex.modelDual.add(mCplex.constraintsDualArtificiais[indiceRestricaoArtificial]);
				expArtifG.end();
				
				++numRestrLambda;
				++numColsDualL_k;
				++indiceRestricaoArtificial;
			}

			if (variaveisComRestricao[k][i] > 0){ //Insere a restricao fixando gamma
				IloExpr expG = IloExpr(mCplex.env);
				limite = matrizB_raiz[k].size();
				for (int r = 0; r < limite; ++r){
					if (matrizB_raiz[k][r][i] > 0){
						expG += matrizB_raiz[k][r][i] * mCplex.gammaPrimal[k][r];
						colDualG += mCplex.constraintsDual2[k][r](matrizB_raiz[k][r][i]);
					}
				}
				for (int r = 0; r < qRConsNo[k]; ++r){
					if (matrizB_no[k][r][i] > 0){
						expG += matrizB_no[k][r][i] * mCplex.gammaNo[k][r];
						colDualG += mCplex.constraintsDual2_no[k][r](matrizB_no[k][r][i]);
					}
				}
				if ((variaveisComRestricao[k][i] == 1) || (variaveisComRestricao[k][i] == 4)){
					mCplex.constraintsArvoreGamma.add(expG == 1);
					coefColunaArtificial = 1;
				}else{
					mCplex.constraintsArvoreGamma.add(expG == 0);
					coefColunaArtificial = -1;
				}
				mCplex.modelPrimal.add(mCplex.constraintsArvoreGamma[numRestrGamma]);
				expG.end();

				mCplex.varDualArvoreGamma[k].add(IloNumVar(colDualG, -IloInfinity, +IloInfinity, ILOFLOAT));
				colDualG.clear();
				
				//insere a variavel artificial associada a esta restricao
				IloNumColumn colG = mCplex.costPrimal(ModeloCplex::limitePrimal);
				colG += mCplex.constraintsArvoreGamma[numRestrGamma](coefColunaArtificial);
				mCplex.artificiais.add(IloNumVar(colG, 0, +IloInfinity, ILOFLOAT));
				colG.end();

				//insere a restricao dual associada a variavel artificial que acaba de ser criada
				IloExpr expArtifG = coefColunaArtificial * mCplex.varDualArvoreGamma[k][numColsDualG_k];
				mCplex.constraintsDualArtificiais.add(expArtifG <= ModeloCplex::limitePrimal);
				mCplex.modelDual.add(mCplex.constraintsDualArtificiais[indiceRestricaoArtificial]);
				expArtifG.end();

				++numRestrGamma;
				++numColsDualG_k;
				++indiceRestricaoArtificial;
			}
		}
	}
}

void NoArvore::atualizaCustosDuaisForn(ModeloCplex& mCplex, Grafo* G, int k){
	float custoDualVertice;
	int saltoK = k * numCommod;
	IloNumArray thetaValues(mCplex.env), piValues(mCplex.env), xiValues(mCplex.env), arvoreValues(mCplex.env);
	mCplex.cplexPrimal.getDuals(thetaValues, mCplex.constraintsPrimal3);
	mCplex.cplexPrimal.getDuals(piValues, mCplex.constraintsPrimal5);
	mCplex.cplexPrimal.getDuals(xiValues, mCplex.constraintsPrimal6);
	mCplex.cplexPrimal.getDuals(arvoreValues, mCplex.constraintsArvoreLambda);

	//CALCULO DOS INDICES PARA INCLUIR O CUSTO DUAL DAS RESTRICOES DA ARVORE NO VERTICE
	int *vetorIndices = new int[numCommod+1];
	int numRestrLambda = 0;
	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v){
		for (int j = 1; j <= numCommod; ++j){
			if (variaveisComRestricao[v][j] > 0){
				++numRestrLambda;
			} 
		}
	}
	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j){
		vetorIndices[j] = numRestrLambda;
		if (variaveisComRestricao[k][j] > 0){
			++numRestrLambda;
		}
	}

	for (int i = 1; i <= numCommod; ++i){
		if (variaveisComRestricao[k][i] == 0){
			custoDualVertice = -thetaValues[i-1] - piValues[saltoK + i-1] + xiValues[saltoK + i-1];
		}else{
			custoDualVertice = -thetaValues[i-1] - piValues[saltoK + i-1] + xiValues[saltoK + i-1] - arvoreValues[vetorIndices[i]];
		}
		G->setCustoVerticeDual(i, custoDualVertice);
	}
	G->setCustoArestasDual(k, 'F');

	thetaValues.end(); piValues.end(); xiValues.end(); arvoreValues.end();
	delete [] vetorIndices;
}

void NoArvore::atualizaCustosDuaisCons(ModeloCplex& mCplex, Grafo* G, int k){
	float custoDualVertice;
	int saltoK = k * numCommod;
	IloNumArray muValues(mCplex.env), piValues(mCplex.env), xiValues(mCplex.env), arvoreValues(mCplex.env);
	mCplex.cplexPrimal.getDuals(muValues, mCplex.constraintsPrimal4);
	mCplex.cplexPrimal.getDuals(piValues, mCplex.constraintsPrimal5);
	mCplex.cplexPrimal.getDuals(xiValues, mCplex.constraintsPrimal6);
	mCplex.cplexPrimal.getDuals(arvoreValues, mCplex.constraintsArvoreGamma);

	//CALCULO DOS INDICES PARA INCLUIR O CUSTO DUAL DAS RESTRICOES DA ARVORE NO VERTICE
	int *vetorIndices = new int[numCommod+1];
	int numRestrGamma = 0;
	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v){
		for (int j = 1; j <= numCommod; ++j){
			if (variaveisComRestricao[v][j] > 0){
				++numRestrGamma;
			} 
		}
	}
	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j){
		vetorIndices[j] = numRestrGamma;
		if (variaveisComRestricao[k][j] > 0){
			++numRestrGamma;
		}
	}

	for (int i = 1; i <= numCommod; ++i){
		if (variaveisComRestricao[k][i] == 0){
			custoDualVertice = -muValues[i-1] + piValues[saltoK + i-1] - xiValues[saltoK + i-1];
		}else{
			custoDualVertice = -muValues[i-1] + piValues[saltoK + i-1] - xiValues[saltoK + i-1] - arvoreValues[vetorIndices[i]];
		}
		G->setCustoVerticeDual(numCommod+i, custoDualVertice);
	}
	G->setCustoArestasDual(k, 'C');

	muValues.end(); piValues.end(); xiValues.end(); arvoreValues.end();
	delete [] vetorIndices;
}

float NoArvore::atualizaCustosDuaisPontosInterioresForn(ModeloCplex& mCplex, Grafo* G, int k){
	float custoDualVertice;
	int pontosExtremosObtidos = 0, acresc = 0, i = 0;

	//numero de restricoes de arvore que existem no modelo primal
	//equivalente ao numero de variaveis acrescentadas no modelo dual
	for (int v = 0; v < maxV; ++v){
		for(int q = 1; q <= numCommod; ++q){
			if (variaveisComRestricao[v][q] > 0){
				++acresc;
			}
		}
	}
	int tamanhoPI = mCplex.tamPontoInterior + 2*acresc; 
	IloNum *pontoExtremo, *pInterior = new IloNum[tamanhoPI];
	memset(pInterior, 0, tamanhoPI * sizeof(IloNum));

	//Fixa variaveis e restricoes no dual em funcao das variaveis de folga e de decisao do primal
	mCplex.geraModeloDual(this->variaveisComRestricao);

	while(i < NoArvore::numPontosExtremos){
		//atribui novos multiplicadores a funcao objetivo dual
		mCplex.setObjectiveFunctionDual(this->variaveisComRestricao);

		//O metodo getPontoExtremo() pode retornar um valor NULL, caso o dual seja 'unbounded' ou 'infeasible'
		//caso isto aconteca, o valor retornado deve ser desconsiderado
		pontoExtremo = mCplex.getPontoExtremo(this->variaveisComRestricao, acresc);

		if (pontoExtremo != NULL){
			for (int j = 0; j < tamanhoPI; ++j){
				pInterior[j] += pontoExtremo[j];
			}
			delete [] pontoExtremo;
			++pontosExtremosObtidos;
		}
		++i;
	}

	if (pontosExtremosObtidos > 0){
		//obtem a media em cada dimensao do ponto interior obtido
		for (int j = 0; j < tamanhoPI; ++j){
			pInterior[j] /= pontosExtremosObtidos;
		}
	}else{
		cout << "Nao retornou pontos extremos no metodo de pontos interires! [EXIT]" << endl;
		exit(0);
	}

	//Neste ponto tem-se o vetor com os valores de cada coordenada interior ao poliedro dual
	//Deve-se inserir os valores nos vertices para calcular o pricing
	int numRestrArvoreK = 0;
	int saltoK = k*numCommod;
	int inicioTHETA = 2*maxV;
	int inicioPI = inicioTHETA + 2*numCommod;
	int inicioXI = inicioPI + numCommod*maxV;
	int indicePrimeiraRestrArvore = inicioXI + numCommod*maxV;
	for (int v = 0; v < k; ++v){
		for (int i = 1; i <= numCommod; ++i){
			if (variaveisComRestricao[v][i] > 0){
				++indicePrimeiraRestrArvore;
			}
		}
	}

	for (int i = 0; i < numCommod; ++i){
		if (variaveisComRestricao[k][i+1] == 0){
			custoDualVertice =	- pInterior[inicioTHETA + i] 
										- pInterior[inicioPI + saltoK + i]
										+ pInterior[inicioXI + saltoK + i];
		}else{
			custoDualVertice =	- pInterior[inicioTHETA + i] 
										- pInterior[inicioPI + saltoK + i]
										+ pInterior[inicioXI + saltoK + i]
										- pInterior[indicePrimeiraRestrArvore + numRestrArvoreK];
			++numRestrArvoreK;
		}
		G->setCustoVerticeDual(i+1, custoDualVertice);
	}
	G->setCustoArestasDual(k, 'F');

	//retorna alfaDual[k] para ser subtraido do valor da rota retornada pelo subproblema
	return pInterior[k];
}

float NoArvore::atualizaCustosDuaisPontosInterioresCons(ModeloCplex& mCplex, Grafo* G, int k){
	float custoDualVertice;
	int pontosExtremosObtidos = 0, acresc = 0, i = 0;

	//numero de restricoes de arvore que existem no modelo primal
	//equivalente ao numero de variaveis acrescentadas no modelo dual
	for (int v = 0; v < maxV; ++v){
		for(int q = 1; q <= numCommod; ++q){
			if (variaveisComRestricao[v][q] > 0){
				++acresc;
			}
		}
	}
	int tamanhoPI = mCplex.tamPontoInterior + 2*acresc; 
	IloNum *pontoExtremo, *pInterior = new IloNum[tamanhoPI];
	memset(pInterior, 0, tamanhoPI * sizeof(IloNum));

	//Fixa variaveis e restricoes no dual em funcao das variaveis de folga e de decisao do primal
	mCplex.geraModeloDual(this->variaveisComRestricao);

	while(i < numPontosExtremos){
		//atribui novos multiplicadores a funcao objetivo dual
		mCplex.setObjectiveFunctionDual(this->variaveisComRestricao);

		//O metodo getPontoExtremo() pode retornar um valor NULL, caso o dual seja 'unbounded' ou 'infeasible'
		//caso isto aconteca, o valor retornado deve ser desconsiderado
		pontoExtremo = mCplex.getPontoExtremo(this->variaveisComRestricao, acresc);

		if (pontoExtremo != NULL){
			for (int j = 0; j < tamanhoPI; ++j){
				pInterior[j] += pontoExtremo[j];
			}
			delete [] pontoExtremo;
			++pontosExtremosObtidos;
		}
		++i;
	}

	if (pontosExtremosObtidos > 0){
		//obtem a media em cada dimensao do ponto interior obtido
		for (int j = 0; j < tamanhoPI; ++j){
			pInterior[j] /= pontosExtremosObtidos;
		}
	}else{
		cout << "Nao retornou pontos extremos no metodo de pontos interires! [EXIT]" << endl;
		exit(0);
	}

	//Neste ponto tem-se o vetor com os valores de cada coordenada interior ao poliedro dual
	//Deve-se inserir os valores nos vertices para calcular o pricing
	int numRestrArvoreK = 0;
	int saltoK = k*numCommod;
	int inicioMU = 2*maxV + numCommod;
	int inicioPI = inicioMU + numCommod;
	int inicioXI = inicioPI + numCommod*maxV;
	int indicePrimeiraRestrArvore = inicioXI + numCommod*maxV + acresc;
	for (int v = 0; v < k; ++v){
		for (int i = 1; i <= numCommod; ++i){
			if (variaveisComRestricao[v][i] > 0){
				++indicePrimeiraRestrArvore;
			}
		}
	}

	for (int i = 0; i < numCommod; ++i){
		if (variaveisComRestricao[k][i+1] == 0){
			custoDualVertice =	- pInterior[inicioMU + i] 
										+ pInterior[inicioPI + saltoK + i]
										- pInterior[inicioXI + saltoK + i];
		}else{
			custoDualVertice =	- pInterior[inicioMU + i] 
										+ pInterior[inicioPI + saltoK + i]
										- pInterior[inicioXI + saltoK + i]
										- pInterior[indicePrimeiraRestrArvore + numRestrArvoreK];
			++numRestrArvoreK;
		}
		G->setCustoVerticeDual(numCommod+i+1, custoDualVertice);
	}
	G->setCustoArestasDual(k, 'C');
	
	//retorna betaDual[k] para ser subtraido do valor da rota retornada pelo subproblema
	return pInterior[maxV + k];
}

void NoArvore::inserirColunaForn(ModeloCplex& mCplex, Rota* r, int k){
	//CALCULO DOS INDICES PARA INCLUIR O CUSTO DUAL DAS RESTRICOES DA ARVORE NO VERTICE
	int *vetorIndices = new int[numCommod+1];
	int kAnteriores, numRestrLambda = 0;
	//verifica quantas restricoes de lambda na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v){
		for (int j = 1; j <= numCommod; ++j){
			if (variaveisComRestricao[v][j] > 0){
				++numRestrLambda;
			} 
		}
	}
	kAnteriores = numRestrLambda;

	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j){
		vetorIndices[j] = numRestrLambda;
		if (variaveisComRestricao[k][j] > 0){
			++numRestrLambda;
		}
	}

	//INSERE A ROTA NO MODELO
	//Armazena a rota na matriz de rotas de fornecedores do NO
	short int* rota = new short int[numCommod+1];
	memset(rota, 0, (numCommod+1) * sizeof(short int));
	matrizA_no[k].push_back(rota);
	ptrRForn_no[k].push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-1;
	for(int j = 1; j < numVertAtualizar; ++j){
		++matrizA_no[k][qRFornNo[k]][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//Cria a variavel de decisao e insere a coluna no modelo (aproveita para criar a restricao no dual)
	IloExpr restDual = mCplex.alfaDual[k];
	IloNumColumn col = mCplex.costPrimal(r->getCusto());
	col += mCplex.constraintsPrimal1[k](1);

	for (int i = 1; i <= numCommod; i++){
		if (matrizA_no[k][qRFornNo[k]][i] > 0){
			col += mCplex.constraintsPrimal3[i-1](matrizA_no[k][qRFornNo[k]][i]);
			col += mCplex.constraintsPrimal5[k*numCommod + i-1](matrizA_no[k][qRFornNo[k]][i]);
			col += mCplex.constraintsPrimal6[k*numCommod + i-1](-matrizA_no[k][qRFornNo[k]][i]);
			
			restDual += matrizA_no[k][qRFornNo[k]][i] * mCplex.thetaDual[i]
						 + matrizA_no[k][qRFornNo[k]][i] * mCplex.piDual[k][i]
						 - matrizA_no[k][qRFornNo[k]][i] * mCplex.xiDual[k][i]; 
			
			/*VERIFICA SE PRECISA INSERIR COEFICIENTE NA COLUNA PARA AS RESTRICOES DA ARVORE*/
			if (variaveisComRestricao[k][i] > 0){
				//vincula a coluna a restricao da arvore (a posicao correta foi calculada anteriormente por eficiencia)
				col += mCplex.constraintsArvoreLambda[vetorIndices[i]](matrizA_no[k][qRFornNo[k]][i]);
				restDual += matrizA_no[k][qRFornNo[k]][i] * mCplex.varDualArvoreLambda[k][vetorIndices[i] - kAnteriores];
			}
		}
	}
	mCplex.lambdaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	mCplex.constraintsDual1_no[k].add(restDual <= r->getCusto());
	mCplex.modelDual.add(mCplex.constraintsDual1_no[k][qRFornNo[k]]);
	restDual.end();

	++qRFornNo[k];
	delete [] vetorIndices;
}

void NoArvore::inserirColunaCons(ModeloCplex& mCplex, Rota* r, int k){
	//CALCULO DOS INDICES PARA INCLUIR O CUSTO DUAL DAS RESTRICOES DA ARVORE NO VERTICE
	int *vetorIndices = new int[numCommod+1];
	int kAnteriores, numRestrGamma = 0;

	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v){
		for (int j = 1; j <= numCommod; ++j){
			if (variaveisComRestricao[v][j] > 0){
				++numRestrGamma;
			} 
		}
	}
	kAnteriores = numRestrGamma;

	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j){
		vetorIndices[j] = numRestrGamma;
		if (variaveisComRestricao[k][j] > 0){
			++numRestrGamma;
		}
	}

	//INSERE A ROTA NO MODELO
	//Armazena a rota na matriz de rotas de consumidores do NO
	short int* rota = new short int[numCommod+1];
	memset(rota, 0, (numCommod+1) * sizeof(short int));
	matrizB_no[k].push_back(rota);
	ptrRCons_no[k].push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j){
		++matrizB_no[k][qRConsNo[k]][vertRota[j]-numCommod]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//Cria a variavel de decisao e insere a coluna no modelo
	IloExpr restDual = mCplex.betaDual[k];
	IloNumColumn col = mCplex.costPrimal(r->getCusto());
	col += mCplex.constraintsPrimal2[k](1);

	for (int i = 1; i <= numCommod; i++){
		if (matrizB_no[k][qRConsNo[k]][i] > 0){
			col += mCplex.constraintsPrimal4[i-1](matrizB_no[k][qRConsNo[k]][i]);
			col += mCplex.constraintsPrimal5[k*numCommod + i-1](-matrizB_no[k][qRConsNo[k]][i]);
			col += mCplex.constraintsPrimal6[k*numCommod + i-1](matrizB_no[k][qRConsNo[k]][i]);
			
			restDual += matrizB_no[k][qRConsNo[k]][i] * mCplex.muDual[i]
						 - matrizB_no[k][qRConsNo[k]][i] * mCplex.piDual[k][i]
						 + matrizB_no[k][qRConsNo[k]][i] * mCplex.xiDual[k][i];
			
			/*VERIFICA SE PRECISA INSERIR COEFICIENTE NA COLUNA PARA AS RESTRICOES DA ARVORE*/
			if (variaveisComRestricao[k][i] > 0){
				//vincula a coluna a restricao da arvore na respectiva posicao (calculada no inicio do metodo)
				col += mCplex.constraintsArvoreGamma[vetorIndices[i]](matrizB_no[k][qRConsNo[k]][i]);
				restDual += matrizB_no[k][qRConsNo[k]][i] * mCplex.varDualArvoreGamma[k][vetorIndices[i] - kAnteriores];
			}
		}
	}
	mCplex.gammaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	mCplex.constraintsDual2_no[k].add(restDual <= r->getCusto());
	mCplex.modelDual.add(mCplex.constraintsDual2_no[k][qRConsNo[k]]);
	restDual.end();

	++qRConsNo[k];
	delete [] vetorIndices;
}

bool NoArvore::verificaFilhoViavel(Grafo* g, int iFilho){
	/*
	 * ANTES DE CRIAR O FILHO VERIFICA SE ELE, AO ESTENDER O PAI, SERA VIAVEL. PARA ISTO CERTAS RESTRICOES DEVEM SER SATISFEITAS
	 * Regras de geração de filhos VIÁVEIS no Branch and Price:
	 **Se for atribuído o valor 1 a uma célula qualquer da coluna, as demais células desta coluna poderão assumir
	 * apenas o valor 2. O valor 1 significa que a commodity q será coletada e entregue pelo veículo k, logo não
	 * poderá ser coletada nem entregue por nenhum outro veículo (cada veículo é representado por uma linha na matriz).
	 * Sendo assim, os valores {1, 3, 4} que denotam a passagem de um veículo para coletar e/ou entregar a commodity q
	 * não é possível.
	 **Não existem restrições quanto à atribuição do valor 2 a uma célula qualquer da coluna, exceto pelo fato de que
	 * a commodity q representada por aquela coluna deve ser coletada e entregue por algum veículo, portanto, em uma
	 * coluna deve existir ou um único valor 1, ou um valor 3 e um valor 4, significando que a commodity será coletada
	 * e entregue.
	 **Como consequencia da restrição acima, se for atribuido 3 a uma célula qualquer da coluna, os únicos valores 
	 * possíveis para aquela coluna são 2 ou 4 (respeitando-se a restrição que deverá existir uma célula com valor 4)
	 **De maneira análoga à restrição anterior, caso o valor atribuido a uma célula qualquer da coluna seja 4, os 
	 * únicos valores possíveis para esta mesma coluna são 2 ou 3 (respeitando-se a restrição que deverá existir 
	 * uma célula com valor 3)
	 **Existe uma regra considerando a linha da matriz (um mesmo veículo). Soma-se os pesos de todas as commodities
	 * desta linha cujo valor da célula seja {1,3} (rota do veiculo k representado pela linha para os fornecedores)
	 * ou {1,4} (rota do veiculo k representado pela linha para os consumidores). Ambos os somatórios devem ser menores
	 * do que a capacidade do veículo, caso contrário, o nó a ser gerado será inviável por capacidade.*/

	bool colunaOK = true, linhaOK = true;
	int numCelulasZero, numCelulasUm, numCelulasTres, numCelulasQuatro; 
	//percorre a coluna para verificar se todas as restricoes associadas a coluna sao satisfeitas
	switch (iFilho){
		case 1:
			for (int k = 0; k < maxV; ++k){
				if ((variaveisComRestricao[k][q_branching] == 1) ||
					 (variaveisComRestricao[k][q_branching] == 3) ||
					 (variaveisComRestricao[k][q_branching] == 4)){
					colunaOK = false;
					break;
				}
			}
			break;

		case 2:
			numCelulasZero = 0;
			numCelulasUm = 0;
			numCelulasTres = 0;
			numCelulasQuatro = 0;
			for (int k = 0; k < maxV; ++k){
				if (variaveisComRestricao[k][q_branching] == 0){
					++numCelulasZero;
				}else if (variaveisComRestricao[k][q_branching] == 1){
					++numCelulasUm;
				}else if (variaveisComRestricao[k][q_branching] == 3){
					++numCelulasUm;
				}else if (variaveisComRestricao[k][q_branching] == 4){
					++numCelulasUm;
				}
			}
			if ((numCelulasZero == 1) && ((numCelulasUm == 0) || ((numCelulasTres + numCelulasQuatro < 2)))){
				colunaOK = false;
			}
			break;

		case 3:
			numCelulasZero = 0;
			numCelulasUm = 0;
			numCelulasTres = 0;
			numCelulasQuatro = 0;
			for (int k = 0; k < maxV; ++k){
				if (variaveisComRestricao[k][q_branching] == 0){
					++numCelulasZero;
				}else if (variaveisComRestricao[k][q_branching] == 1){
					++numCelulasUm;
				}else if (variaveisComRestricao[k][q_branching] == 3){
					++numCelulasTres;
				}else if (variaveisComRestricao[k][q_branching] == 4){
					++numCelulasQuatro;
				}
			}
			if ((numCelulasUm > 0) || (numCelulasTres > 0)){
				colunaOK = false;
			}else if ((numCelulasZero == 1) && (numCelulasQuatro == 0)){
				colunaOK = false;
			}
			break;

		case 4:
			numCelulasZero = 0;
			numCelulasUm = 0;
			numCelulasTres = 0;
			numCelulasQuatro = 0;
			for (int k = 0; k < maxV; ++k){
				if (variaveisComRestricao[k][q_branching] == 0){
					++numCelulasZero;
				}else if (variaveisComRestricao[k][q_branching] == 1){
					++numCelulasUm;
				}else if (variaveisComRestricao[k][q_branching] == 3){
					++numCelulasTres;
				}else if (variaveisComRestricao[k][q_branching] == 4){
					++numCelulasQuatro;
				}
			}
			if ((numCelulasUm > 0) || (numCelulasQuatro > 0)){
				colunaOK = false;
			}else if ((numCelulasZero == 1) && (numCelulasTres == 0)){
				colunaOK = false;
			}
	}
		
	//percorre a linha para verificar se a restricao de capacidade do veiculo eh satisfeita
	if ((colunaOK) && (iFilho != 2)){
		int cargaRotaFornecedores = 0, cargaRotaConsumidores = 0, capacidade = g->getCapacVeiculo( k_branching );
		for (int q = 1; q <= numCommod; ++q){
			if (variaveisComRestricao[k_branching][q] == 1){
				cargaRotaFornecedores += g->getPesoCommoditySemCorrigirPosicao(q);
				cargaRotaConsumidores += g->getPesoCommoditySemCorrigirPosicao(q+numCommod);
			}else if (variaveisComRestricao[k_branching][q] == 3){
				cargaRotaFornecedores += g->getPesoCommoditySemCorrigirPosicao(q);
			}else if (variaveisComRestricao[k_branching][q] == 4){
				cargaRotaConsumidores += g->getPesoCommoditySemCorrigirPosicao(q+numCommod);
			}
		}
		if (iFilho == 1){
			if ((cargaRotaFornecedores + g->getPesoCommoditySemCorrigirPosicao(q_branching) > capacidade) || 
				 (cargaRotaConsumidores + g->getPesoCommoditySemCorrigirPosicao(q_branching+numCommod) > capacidade)){
				linhaOK = false;
			}
		}else if (iFilho == 3){
			if ((cargaRotaFornecedores + g->getPesoCommoditySemCorrigirPosicao(q_branching)) > capacidade){
				linhaOK = false;
			}
		}else if (iFilho == 4){
			if ((cargaRotaConsumidores + g->getPesoCommoditySemCorrigirPosicao(q_branching+numCommod)) > capacidade){
				linhaOK = false;
			}
		}
	}
	return ((colunaOK) && (linhaOK));
}

void NoArvore::exportaModeloNo(ModeloCplex& mCplex, const char* nomeModelo){
	mCplex.cplexPrimal.exportModel(nomeModelo);
}

void NoArvore::imprimeNo(){
	cout << "indice = " << indiceNo << endl;
	cout << "valor = " << limiteDual << endl;
	for (int k = 0; k < maxV; ++k){
		cout << "qRFornNo[" << k << "] = " << qRFornNo[k] << endl;
		for (int r = 0; r < qRFornNo[k]; ++r){
			cout << "  ";
			ptrRForn_no[k][r]->imprimir();
		}
	}
	for (int k = 0; k < maxV; ++k){
		cout << "qRConsNo[" << k << "] = " << qRConsNo[k] << endl;
		for (int r = 0; r < qRConsNo[k]; ++r){
			cout << "  ";
			ptrRCons_no[k][r]->imprimir();
		}
	}
	cout << "Variaveis com restricao:" << endl;
	for (int k = 0; k < maxV; ++k){
		for(int i = 0; i <=numCommod; ++i){
			cout << (int)variaveisComRestricao[k][i] << " ";
		}
		cout << endl;
	}
}

int NoArvore::getIndiceNo(){
	return indiceNo;
}

ptrNo NoArvore::getProx(){
	return prox;
}

void NoArvore::setProx(ptrNo p){
	prox = p;
}

int NoArvore::getKBranching(){
	return k_branching;
}

int NoArvore::getQBranching(){
	return q_branching;
}

vector<short int*>* NoArvore::getMatrizA_no(){
	return matrizA_no;
}

vector<short int*>* NoArvore::getMatrizB_no(){
	return matrizB_no;
}

vector<Rota*>* NoArvore::getPtrRForn_no(){
	return ptrRForn_no;
}

vector<Rota*>* NoArvore::getPtrRCons_no(){
	return ptrRCons_no;
}

char** NoArvore::getVariaveisComRestricao(){
	return variaveisComRestricao;
}

float NoArvore::getLimiteDual(){
	return limiteDual;
}

int NoArvore::getTotalNosCriados(){
	return totalNosCriados;
}

int NoArvore::getTotalNosArmazenados(){
	return totalNosArmazenados;
}
