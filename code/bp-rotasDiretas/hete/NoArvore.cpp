#include "NoArvore.h"

//Inicializacao dos MEMBROS ESTATICOS, necessaria antes de atribuir valor a estes membros
int NoArvore::maxV;
int NoArvore::numCommod;
int NoArvore::totalNosCriados;
int NoArvore::totalNosArmazenados;
vector<short int*>* NoArvore::matrizA_raiz;
vector<short int*>* NoArvore::matrizB_raiz;
vector<short int*>* NoArvore::matrizD_raiz;

NoArvore::~NoArvore(){
	for (int k = 0; k < maxV; ++k)
	{
		for (int r = 0; r < qRFornNo[k]; ++r)
		{
			delete [] matrizA_no[k][r];
			if (ptrRForn_no[k][r]->decrNumApontadores()) delete ptrRForn_no[k][r];
		}
		for (int r = 0; r < qRConsNo[k]; ++r)
		{
			delete [] matrizB_no[k][r];
			if (ptrRCons_no[k][r]->decrNumApontadores()) delete ptrRCons_no[k][r];
		}
		for (int r = 0; r < qRDirectNo[k]; ++r)
		{
			delete [] matrizD_no[k][r];
			if (ptrRDirect_no[k][r]->decrNumApontadores()) delete ptrRDirect_no[k][r];
		}
		matrizA_no[k].clear();
		matrizB_no[k].clear();
		matrizD_no[k].clear();
		ptrRForn_no[k].clear();
		ptrRCons_no[k].clear();
		ptrRDirect_no[k].clear();
		delete [] variaveisComRestricao[k];
	}
	delete [] qRFornNo;
	delete [] qRConsNo;
	delete [] qRDirectNo;
	delete [] matrizA_no;
	delete [] matrizB_no;
	delete [] matrizD_no;
	delete [] ptrRForn_no;
	delete [] ptrRCons_no;
	delete [] ptrRDirect_no;
	delete [] variaveisComRestricao;
	--totalNosArmazenados;
}

NoArvore::NoArvore(ModeloCplex& mCplex){
	//Inicializa os atributos estaticos da classe
	indiceNo = 1;
	totalNosCriados = 1;
	totalNosArmazenados = 0;
	maxV = mCplex.maxVeic;
	numCommod = mCplex.nCommodities;
	matrizA_raiz = mCplex.A_qr;
	matrizB_raiz = mCplex.B_qr;
	matrizD_raiz = mCplex.D_qr;

	//inicializa os atributos do objeto
	qRFornNo = new int[maxV];
	qRConsNo = new int[maxV];
	qRDirectNo = new int[maxV];
	variaveisComRestricao = new char*[maxV];
	
	matrizA_no = new vector<short int*>[maxV];
	matrizB_no = new vector<short int*>[maxV];
	matrizD_no = new vector<short int*>[maxV];
	
	ptrRForn_no = new vector<Rota*>[maxV];
	ptrRCons_no = new vector<Rota*>[maxV];
	ptrRDirect_no = new vector<Rota*>[maxV];
	for (int k = 0; k < maxV; ++k)
	{
		qRFornNo[k] = 0;
		qRConsNo[k] = 0;
		qRDirectNo[k] = 0;
		//Aloca memoria para a matriz de pares com restricoes; Nenhum par tem restricoes no primeiro no
		variaveisComRestricao[k] = new char[numCommod+1];
		memset (variaveisComRestricao[k], 0, (numCommod+1)*sizeof(char));
	}

	//Aloca a memoria dos PONTEIROS para as variaveis e restricoes da arvore
	mCplex.constraintsArvoreLambda = IloRangeArray(mCplex.env);
	mCplex.constraintsArvoreGamma = IloRangeArray(mCplex.env);
	mCplex.constraintsArvoreDelta = IloRangeArray(mCplex.env);

	mCplex.artificiais = IloNumVarArray(mCplex.env);
	mCplex.lambdaNo = IloArray <IloNumVarArray> (mCplex.env, maxV);
	mCplex.gammaNo = IloArray <IloNumVarArray> (mCplex.env, maxV);
	mCplex.deltaNo = IloArray <IloNumVarArray> (mCplex.env, maxV);

	for (int k = 0; k < maxV; k++)
	{
		mCplex.lambdaNo[k] = IloNumVarArray(mCplex.env);
		mCplex.gammaNo[k] = IloNumVarArray(mCplex.env);
		mCplex.deltaNo[k] = IloNumVarArray(mCplex.env);
	}

	//finalmente atribui valores as variaveis k_branching e q_branching do primeiro no da arvore
	procuraVariavelBranching(mCplex);
}

NoArvore::NoArvore(ptrNo pai, ModeloCplex& mCplex, int iFilho){
	++totalNosArmazenados;
	indiceNo = ++totalNosCriados;
	qRFornNo = new int[maxV];
	qRConsNo = new int[maxV];
	qRDirectNo = new int[maxV];

	//Aloca a matriz que guarda o historico dos vertices com restricoes do No. A matriz tera |K| linhas
	//e |P| colunas, sendo que cada celula armazenara um dos 6 valores (1),(2),(3),(4),(5) indicando qual restricao
	//de branching foi aplicada a requisicao p_i e veiculo k, e (0) indicando que a requisicao p_i e veiculo k
	//ainda nao foi fixada em uma restricao de branching. A matriz eh inicializada com os valores do pai
	variaveisComRestricao = new char*[maxV];

	//Aloca memoria para armazenar a matrizA_no e matrizB_no e também as matrizes de custos de cada rota
	matrizA_no = new vector<short int*>[maxV];
	matrizB_no = new vector<short int*>[maxV];
	matrizD_no = new vector<short int*>[maxV];

	ptrRForn_no = new vector<Rota*>[maxV];
	ptrRCons_no = new vector<Rota*>[maxV];
	ptrRDirect_no = new vector<Rota*>[maxV];
	short int* rota; //esse ponteiro pendurara a memoria em cada celula da matriz

	//O laco abaixo preenche o NO com os valores das rotas do pai, criando uma variavel de decisao para cada rota do pai
	for (int k = 0; k < maxV; ++k)
	{
		qRFornNo[k] = pai->qRFornNo[k];
		qRConsNo[k] = pai->qRConsNo[k];
		qRDirectNo[k] = pai->qRDirectNo[k];

		variaveisComRestricao[k] = new char[numCommod+1];
		for (int j = 0; j <= numCommod; ++j)
		{
			variaveisComRestricao[k][j] = pai->variaveisComRestricao[k][j];
		}

		//Insere todas as entradas existentes no pai para os FORNECEDORES
		for (int r = 0; r < qRFornNo[k]; ++r)
		{
			rota = new short int[numCommod+1];
			matrizA_no[k].push_back(rota);
			for (int i = 1; i <= numCommod; ++i)
			{
				matrizA_no[k][r][i] = pai->matrizA_no[k][r][i];
			}
			ptrRForn_no[k].push_back(pai->ptrRForn_no[k][r]);
			ptrRForn_no[k][r]->incrNumApontadores();

			//cria a variavel de decisao associada a esta rota no modelo
			IloNumColumn col = mCplex.costPrimal(ptrRForn_no[k][r]->getCusto());
			col += mCplex.constraintsPrimal1[k](1);
			for (int i = 1; i <= numCommod; i++)
			{
				if (matrizA_no[k][r][i] > 0)
				{
					col += mCplex.constraintsPrimal3[i-1](matrizA_no[k][r][i]);
					col += mCplex.constraintsPrimal5[k*numCommod + i-1](matrizA_no[k][r][i]);
					col += mCplex.constraintsPrimal6[k*numCommod + i-1](-matrizA_no[k][r][i]);
				}
			}
			//insere a variavel de decisao no modelo primal
			mCplex.lambdaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
			col.end();
		}

		//Insere todas as entradas existentes no pai para os CONSUMIDORES
		for (int r = 0; r < qRConsNo[k]; ++r)
		{
			rota = new short int[numCommod+1];
			matrizB_no[k].push_back(rota);
			for (int i = 1; i <= numCommod; ++i)
			{
				matrizB_no[k][r][i] = pai->matrizB_no[k][r][i];
			}
			ptrRCons_no[k].push_back(pai->ptrRCons_no[k][r]);
			ptrRCons_no[k][r]->incrNumApontadores();

			//cria a variavel de decisao associada a esta rota e a insere no modelo
			IloNumColumn col = mCplex.costPrimal(ptrRCons_no[k][r]->getCusto());
			col += mCplex.constraintsPrimal2[k](1);
			for (int i = 1; i <= numCommod; i++)
			{
				if (matrizB_no[k][r][i] > 0)
				{
					col += mCplex.constraintsPrimal4[i-1](matrizB_no[k][r][i]);
					col += mCplex.constraintsPrimal5[k*numCommod + i-1](-matrizB_no[k][r][i]);
					col += mCplex.constraintsPrimal6[k*numCommod + i-1](matrizB_no[k][r][i]);
				}
			}
			//insere a variavel de decisao no modelo primal
			mCplex.gammaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
			col.end();
		}

		//Insere todas as entradas existentes no pai para as ROTAS DIRETAS
		int nVertices = 2*numCommod;
		for (int r = 0; r < qRDirectNo[k]; ++r)
		{
			rota = new short int[nVertices+1];
			matrizD_no[k].push_back(rota);
			for (int i = 1; i <= nVertices; ++i)
			{
				matrizD_no[k][r][i] = pai->matrizD_no[k][r][i];
			}
			ptrRDirect_no[k].push_back(pai->ptrRDirect_no[k][r]);
			ptrRDirect_no[k][r]->incrNumApontadores();

			//cria a variavel de decisao associada a esta rota e a insere no modelo
			IloNumColumn col = mCplex.costPrimal(ptrRDirect_no[k][r]->getCusto());
			col += mCplex.constraintsPrimal1[k](1);
			col += mCplex.constraintsPrimal2[k](1);
			for (int i = 1; i <= numCommod; i++)
			{
				if (matrizD_no[k][r][i] > 0)
				{
					col += mCplex.constraintsPrimal3[i-1](matrizD_no[k][r][i]);
					col += mCplex.constraintsPrimal4[i-1](matrizD_no[k][r][i]);
				}
			}
			//insere a variavel de decisao no modelo primal
			mCplex.deltaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
			col.end();
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
	IloNumArray deltaValues(mCplex.env);
	k_branching = 0; q_branching = -1;
	bool solucaoInteira = true;

	//Declara e inicializa a matriz que armazenara a violacao (o quanto esta distante de uma solucao inteira)
	//para todos os possiveis pares [k,q] do problema. Aquele mais violado sera fixado para o branching do no
	float** totalViolacao = new float*[maxV];
	for (int k = 0; k < maxV; k++)
	{
		totalViolacao[k] = new float[numCommod+1];
		memset (totalViolacao[k], 0, (numCommod+1)*sizeof(float));
	}

	//primeiro busca por variaveis fracionarias entre as variaveis de rotas de fornecedores obtidas na raiz
	for(int k = 0; k < maxV; ++k)
	{
		mCplex.cplexPrimal.getValues(lambdaValues, mCplex.lambdaPrimal[k]);
		for (int r = 0; r < mCplex.qRotasForn[k]; ++r)
		{
			if ((lambdaValues[r] > 0) && (lambdaValues[r] < 1))
			{
				solucaoInteira = false;
				for (int i = 1; i <= numCommod; ++i)
				{
					if ((matrizA_raiz[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0))
					{
						if (lambdaValues[r] <= 0.5)
						{
							totalViolacao[k][i] += lambdaValues[r];
						}
						else
						{
							totalViolacao[k][i] += (1 - lambdaValues[r]);
						}
					}
				}
			}
		}
	}
	//depois busca por variaveis fracionarias entre as variaveis de rotas de fornecedores obtidas neste no
	for(int k = 0; k < maxV; ++k)
	{
		if (qRFornNo[k] > 0)
		{
			mCplex.cplexPrimal.getValues(lambdaValues, mCplex.lambdaNo[k]);
			for (int r = 0; r < qRFornNo[k]; ++r)
			{
				if ((lambdaValues[r] > 0) && (lambdaValues[r] < 1))
				{
					solucaoInteira = false;
					for (int i = 1; i <= numCommod; ++i)
					{
						if ((matrizA_no[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0))
						{
							if (lambdaValues[r] <= 0.5)
							{
								totalViolacao[k][i] += lambdaValues[r];
							}
							else
							{
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
	for(int k = 0; k < maxV; ++k)
	{
		mCplex.cplexPrimal.getValues(gammaValues, mCplex.gammaPrimal[k]);
		for (int r = 0; r < mCplex.qRotasCons[k]; ++r)
		{
			if ((gammaValues[r] > 0) && (gammaValues[r] < 1))
			{
				solucaoInteira = false;
				for (int i = 1; i <= numCommod; ++i)
				{
					if ((matrizB_raiz[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0))
					{
						if (gammaValues[r] <= 0.5)
						{
							totalViolacao[k][i] += gammaValues[r];
						}
						else
						{
							totalViolacao[k][i] += (1 - gammaValues[r]);
						}
					}
				}
			}
		}
	}
	//faz a mesma busca, mas agora nas variaveis de roats de consumidores do no
	for(int k = 0; k < maxV; ++k)
	{
		if (qRConsNo[k] > 0)
		{
			mCplex.cplexPrimal.getValues(gammaValues, mCplex.gammaNo[k]);
			for (int r = 0; r < qRConsNo[k]; ++r)
			{
				if ((gammaValues[r] > 0) && (gammaValues[r] < 1))
				{
					solucaoInteira = false;
					for (int i = 1; i <= numCommod; ++i)
					{
						if ((matrizB_no[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0))
						{
							if (gammaValues[r] <= 0.5)
							{
								totalViolacao[k][i] += gammaValues[r];
							}
							else
							{
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

	//busca por variaveis fracionarias entre as variaveis de rotas diretas na raiz
	for(int k = 0; k < maxV; ++k)
	{
		mCplex.cplexPrimal.getValues(deltaValues, mCplex.deltaPrimal[k]);
		for (int r = 0; r < mCplex.qRotasDiretas[k]; ++r)
		{
			if ((deltaValues[r] > 0) && (deltaValues[r] < 1))
			{
				solucaoInteira = false;
				for (int i = 1; i <= numCommod; ++i)
				{
					if ((matrizD_raiz[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0))
					{
						if (deltaValues[r] <= 0.5)
						{
							totalViolacao[k][i] += deltaValues[r];
						}
						else
						{
							totalViolacao[k][i] += (1 - deltaValues[r]);
						}
					}
				}
			}
		}
	}
	//faz a mesma busca, mas agora nas variaveis de rotas diretas do no
	for(int k = 0; k < maxV; ++k)
	{
		if (qRDirectNo[k] > 0)
		{
			mCplex.cplexPrimal.getValues(deltaValues, mCplex.deltaNo[k]);
			for (int r = 0; r < qRDirectNo[k]; ++r)
			{
				if ((deltaValues[r] > 0) && (deltaValues[r] < 1))
				{
					solucaoInteira = false;
					for (int i = 1; i <= numCommod; ++i)
					{
						if ((matrizD_no[k][r][i] > 0) && (variaveisComRestricao[k][i] == 0))
						{
							if (deltaValues[r] <= 0.5)
							{
								totalViolacao[k][i] += deltaValues[r];
							}
							else
							{
								totalViolacao[k][i] += (1 - deltaValues[r]);
							}
						}
					}
				}
			}
		}
	}
	//termina a busca por variaveis fracionarias
	deltaValues.end();

	if (solucaoInteira == true) //significa que lambda, gamma e delta sao inteiros. Verifica se tau tambem eh inteiro
	{
		q_branching = 0;
		for (int k = 0; k < maxV; ++k)
		{
			for (int q = 1; ((q <= numCommod) && (q_branching == 0)); ++q)
			{
				if ((mCplex.cplexPrimal.getValue(mCplex.tauPrimal[k][q]) > 0) && (mCplex.cplexPrimal.getValue(mCplex.tauPrimal[k][q]) < 1))
				{
					q_branching = -1; //nao existem variaveis [k,q] para realizar o branching e a solucao nao eh inteira
				}
			}
		}
	}
	else //significa que existem variavei(s) fracionaria(s): atribui o par [k,q] mais violado para k e q branching
	{
		float maisViolado = 0;
		for (int k = 0; k < maxV; ++k)
		{
			for (int i = 1; i <= numCommod; ++i)
			{
				if ((totalViolacao[k][i] > maisViolado) && (variaveisComRestricao[k][i] == 0))
				{
					maisViolado = totalViolacao[k][i];
					k_branching = k;
					q_branching = i;
				}
			}
		}
		if (maisViolado == 0)
		{
			q_branching = -1;
		}
	}	 

	//desaloca a matriz totalViolacao
	for (int k = 0; k < maxV; k++)
	{
		delete [] totalViolacao[k];
	}
	delete [] totalViolacao;
}
	
ptrNo NoArvore::geraFilho(ModeloCplex& mCplex, Grafo* G, int indiceFilho, char opCam, char opCamRD, int tempo, bool& bestPrimal){
	//antes de invocar o construtor para inicializar o filho, verifica se o filho a ser criado sera viavel
	if (!this->verificaFilhoViavel(G, indiceFilho))
	{
		return NULL;
	}

	bestPrimal = false;
	ptrNo filhoCriado = new NoArvore(this, mCplex, indiceFilho);

	//A otimizacao de um No filho nunca eh inviavel. O que pode acontecer eh de variaveis artificiais do NO
	//estarem na base apos a otimizacao, o que significa que este No nao apresenta solucao para o problema e deve 
	//ser descartado, mas para isto deve-se alcancar a raiz deste No e verificar sua base
	if (filhoCriado->alcancaRaizNo(mCplex, G, opCam, opCamRD, tempo))
	{
		//Caso o no nao contenha variaveis artificiais na base, verifica se ele tem solucao inteira ou fracionaria
		//no caso da solucao ser inteira verifica se o limite primal eh menor do que o atual e atualiza
		//caso a solucao seja fracionaria, verifica qual sera a variavel a ser feito o branching nos filhos deste no (proxima iteracao)
		filhoCriado->procuraVariavelBranching(mCplex);

		if (filhoCriado->q_branching > 0) //solucao fracionaria passivel de ser fixada no branching
		{
			filhoCriado->limiteDual = mCplex.cplexPrimal.getObjValue();
			if (filhoCriado->limiteDual < ModeloCplex::limitePrimal)
			{
				return filhoCriado;
			}
			else
			{
				delete filhoCriado;
				return NULL;
			}
		}
		else if (filhoCriado->q_branching == 0) //solucao inteira (PODA POR OTIMALIDADE)
		{
			filhoCriado->limiteDual = mCplex.cplexPrimal.getObjValue();
			if (filhoCriado->limiteDual < ModeloCplex::limitePrimal)
			{
				ModeloCplex::limitePrimal = filhoCriado->limiteDual;
				bestPrimal = true;
				return filhoCriado;
			}
			else
			{
				delete filhoCriado; 
				return NULL;
			}
		}
		else
		{	//a solucao eh fracionaria, mas nao eh possivel realizar o branching ou por que as variaveis 
			//fracionarias ja foram fixadas em iteracoes anteriores, ou principalmente por que a fixação
			//da célula [k,q] na matriz geraria um no que não levaria a solução otima.
			//O NO SERA ARMAZENADO PARA REALIZAR O BRANCHING DE VARIAVEIS NOS ARCOS, CASO SEU LIMITE DUAL PERMITA
			filhoCriado->limiteDual = mCplex.cplexPrimal.getObjValue();
			if (filhoCriado->limiteDual < ModeloCplex::limitePrimal)
			{
				printf("Surgimento incorreto de no para 'segundo branching'!\n");
				exit(0);
			}
			else
			{
				delete filhoCriado;
				return NULL;
			}
		}
	}
	else
	{
		delete filhoCriado;
		return NULL;
	}
}

bool NoArvore::alcancaRaizNo(ModeloCplex& mCplex, Grafo* G, char opcaoCaminho, char opcaoCaminhoRD, int tempo){
	Rota** colunas;
	float valorSubtrair;
	bool pricingFull, alcancouRaiz;
	int* pricingFullForn = new int[maxV];
	int* pricingFullCons = new int[maxV];
	int numPricingFull, limVeic, iVeicForn, iVeicCons, iVeicDirect = 0;

	//Declara um ponteiro para as colunas que serao retornadas do pricing
	do{
		iVeicForn = 0; iVeicCons = 0;
		memset(pricingFullForn, 0, maxV*sizeof(int));
		memset(pricingFullCons, 0, maxV*sizeof(int));

		do{
			pricingFull = false;
			alcancouRaiz = true;
			mCplex.solveMaster();
			limVeic = iVeicForn + maxV;
			for ( int k = iVeicForn; k < limVeic; ++k, ++iVeicForn )
			{
				if ( pricingFullForn[(k % maxV)] != -1 )
				{
					atualizaCustosDuaisForn(mCplex, G, (k % maxV));
					valorSubtrair = mCplex.cplexPrimal.getDual(mCplex.constraintsPrimal1[(k % maxV)]);
					colunas = retornaColuna(G, (k % maxV), valorSubtrair, opcaoCaminho, true, pricingFull, pricingFullForn[(k % maxV)]);

					if (colunas != NULL)
					{
						for (int i = 0; colunas[i] != NULL; ++i)
						{
							inserirColunaForn(mCplex, colunas[i], (k % maxV));
						}
						
						//Retorna os valores dos vetores para que todos computem os Subproblemas
						for (int v = 0; v < maxV; ++v)
						{
							pricingFullForn[v] = pricingFullCons[v] = 1;
						}
						
						mCplex.cplexPrimal.solve();
						alcancouRaiz = false;
						pricingFull = false;
						delete [] colunas;
						++iVeicForn;
						break;
					}
				}
			}

			limVeic = iVeicCons + maxV;
			for ( int k = iVeicCons; k < limVeic; ++k, ++iVeicCons )
			{
				if ( pricingFullCons[(k % maxV)] != -1 )
				{
					atualizaCustosDuaisCons(mCplex, G, (k % maxV));
					valorSubtrair = mCplex.cplexPrimal.getDual(mCplex.constraintsPrimal2[(k % maxV)]);
					colunas = retornaColuna(G, (k % maxV), valorSubtrair, opcaoCaminho, false, pricingFull, pricingFullCons[(k % maxV)]);					
					if (colunas != NULL)
					{
						for (int i = 0; colunas[i] != NULL; ++i)
						{
							inserirColunaCons(mCplex, colunas[i], (k % maxV));
						}
						
						//Retorna os valores dos vetores para que todos computem os Subproblemas
						for (int v = 0; v < maxV; ++v)
						{
							pricingFullForn[v] = pricingFullCons[v] = 1;
						}
						
						mCplex.cplexPrimal.solve();
						alcancouRaiz = false;
						delete [] colunas;
						++iVeicCons;
						break;
					}
				}
			}

			numPricingFull = 0;
			for ( int v = 0; v < maxV; ++v )
			{
				numPricingFull += pricingFullForn[v];
				numPricingFull += pricingFullCons[v];
			}
		}while( ( !alcancouRaiz ) || ( numPricingFull != ( -2*maxV ) ) );

		mCplex.cplexPrimal.solve();
		limVeic = iVeicDirect + maxV;
		for ( int k = iVeicDirect; k < limVeic; ++k, ++iVeicDirect )
		{
			atualizaCustosDuaisDirect(mCplex, G, (k % maxV));
			valorSubtrair = mCplex.cplexPrimal.getDual(mCplex.constraintsPrimal1[(k % maxV)]) + mCplex.cplexPrimal.getDual(mCplex.constraintsPrimal2[(k % maxV)]);
			colunas = retornaColuna(G, (k % maxV), valorSubtrair, opcaoCaminhoRD, tempo);
			if ( colunas != NULL )
			{
				for ( int i = 0; colunas[i] != NULL; ++i )
				{
					inserirColunaDirect(mCplex, colunas[i], (k % maxV));
				}
				
				alcancouRaiz = false;
				delete [] colunas;
				++iVeicDirect;
				break;
			}
		}
	}while ( !alcancouRaiz );

	IloNumArray valoresVarArtif (mCplex.env);
	mCplex.cplexPrimal.getValues(valoresVarArtif, mCplex.artificiais);
	int numVarArtificiais = valoresVarArtif.getSize();
	for (int i = 0; i < numVarArtificiais; ++i)
	{
		if (valoresVarArtif[i] >= 0.00001) return false;
	}
	return true;
}

void NoArvore::setRestricoesArvore(ModeloCplex& mCplex){
	setRestricoesArvoreTau(mCplex);
	setRestricoesArvoreLambdaGammaDelta(mCplex);
}

void NoArvore::setRestricoesArvoreTau(ModeloCplex& mCplex){
	//as restricoes tau0 e tau1 darao origem a duas variaveis de decisao no dual
	int count = 0, numRestrIgual1 = 0;
	IloExpr exp0 = IloExpr(mCplex.env);
	IloExpr exp1 = IloExpr(mCplex.env);
	for (int k = 0; k < maxV; ++k)
	{
		for (int i = 1; i <= numCommod; ++i)
		{
			if ((variaveisComRestricao[k][i] == 1) || (variaveisComRestricao[k][i] == 2))
			{
				exp1 += mCplex.tauPrimal[k][i];
				++numRestrIgual1;
			}
			else if ((variaveisComRestricao[k][i] == 3) || (variaveisComRestricao[k][i] == 4) || (variaveisComRestricao[k][i] == 5))
			{
				exp0 += mCplex.tauPrimal[k][i];
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
}

void NoArvore::setRestricoesArvoreLambdaGammaDelta(ModeloCplex& mCplex){
	//RESTRICOES QUE FIXAM AS VARIAVEIS LAMBDA, GAMMA, DELTA PARA ALGUNS NOS (que tiverem variaveisComRestricao[k][i] > 0)
	//CRIA-SE UMA VARIAVEL ARTIFICIAL ASSOCIADA A CADA RESTRICAO (COEFICIENTE 1 APENAS NA RESTRICAO E OBJETIVO = limitePrimal)
	int limite, coefColunaArtificial, numRestr = 0;
	for (int k = 0; k < maxV; ++k)
	{
		for (int i = 1; i <= numCommod; ++i)
		{
			if (variaveisComRestricao[k][i] > 0) 
			{
				//Insere a restricao fixando LAMBDA
				IloExpr expL = IloExpr(mCplex.env);
				limite = matrizA_raiz[k].size();
				for (int r = 0; r < limite; ++r)
				{
					if (matrizA_raiz[k][r][i] > 0)
					{
						expL += matrizA_raiz[k][r][i] * mCplex.lambdaPrimal[k][r];
					}
				}
				for (int r = 0; r < qRFornNo[k]; ++r)
				{
					if (matrizA_no[k][r][i] > 0)
					{
						expL += matrizA_no[k][r][i] * mCplex.lambdaNo[k][r];
					}
				}
				if ((variaveisComRestricao[k][i] == 1) || (variaveisComRestricao[k][i] == 3))
				{
					mCplex.constraintsArvoreLambda.add(expL == 1);
					coefColunaArtificial = 1;
				}
				else
				{
					mCplex.constraintsArvoreLambda.add(expL == 0);
					coefColunaArtificial = -1;
				}
				mCplex.modelPrimal.add(mCplex.constraintsArvoreLambda[numRestr]);
				expL.end();

				//insere a variavel artificial associada a esta restricao
				IloNumColumn colL = mCplex.costPrimal(ModeloCplex::limitePrimal);
				colL += mCplex.constraintsArvoreLambda[numRestr](coefColunaArtificial);
				mCplex.artificiais.add(IloNumVar(colL, 0, +IloInfinity, ILOFLOAT));
				colL.end();

				//Insere a restricao fixando GAMMA
				IloExpr expG = IloExpr(mCplex.env);
				limite = matrizB_raiz[k].size();
				for (int r = 0; r < limite; ++r)
				{
					if (matrizB_raiz[k][r][i] > 0)
					{
						expG += matrizB_raiz[k][r][i] * mCplex.gammaPrimal[k][r];
					}
				}
				for (int r = 0; r < qRConsNo[k]; ++r)
				{
					if (matrizB_no[k][r][i] > 0)
					{
						expG += matrizB_no[k][r][i] * mCplex.gammaNo[k][r];
					}
				}
				if ((variaveisComRestricao[k][i] == 2) || (variaveisComRestricao[k][i] == 3))
				{
					mCplex.constraintsArvoreGamma.add(expG == 1);
					coefColunaArtificial = 1;
				}
				else
				{
					mCplex.constraintsArvoreGamma.add(expG == 0);
					coefColunaArtificial = -1;
				}
				mCplex.modelPrimal.add(mCplex.constraintsArvoreGamma[numRestr]);
				expG.end();
				
				//insere a variavel artificial associada a esta restricao
				IloNumColumn colG = mCplex.costPrimal(ModeloCplex::limitePrimal);
				colG += mCplex.constraintsArvoreGamma[numRestr](coefColunaArtificial);
				mCplex.artificiais.add(IloNumVar(colG, 0, +IloInfinity, ILOFLOAT));
				colG.end();

				//Insere a restricao fixando DELTA
				IloExpr expD = IloExpr(mCplex.env);
				limite = matrizD_raiz[k].size();
				for (int r = 0; r < limite; ++r)
				{
					if (matrizD_raiz[k][r][i] > 0)
					{
						expD += matrizD_raiz[k][r][i] * mCplex.deltaPrimal[k][r];
					}
				}
				for (int r = 0; r < qRDirectNo[k]; ++r)
				{
					if (matrizD_no[k][r][i] > 0)
					{
						expD += matrizD_no[k][r][i] * mCplex.deltaNo[k][r];
					}
				}
				if (variaveisComRestricao[k][i] == 5)
				{
					mCplex.constraintsArvoreDelta.add(expD == 1);
					coefColunaArtificial = 1;
				}
				else
				{
					mCplex.constraintsArvoreDelta.add(expD == 0);
					coefColunaArtificial = -1;
				}
				mCplex.modelPrimal.add(mCplex.constraintsArvoreDelta[numRestr]);
				expD.end();

				//insere a variavel artificial associada a esta restricao
				IloNumColumn colD = mCplex.costPrimal(ModeloCplex::limitePrimal);
				colD += mCplex.constraintsArvoreDelta[numRestr](coefColunaArtificial);
				mCplex.artificiais.add(IloNumVar(colD, 0, +IloInfinity, ILOFLOAT));
				colD.end();
				++numRestr;
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
	int numRestrLambda = 0;
	int *vetorIndices = new int[numCommod+1];

	//verifica quantas restricoes de lambda na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v)
	{
		for (int j = 1; j <= numCommod; ++j)
		{
			if (variaveisComRestricao[v][j] > 0)
			{
				++numRestrLambda;
			} 
		}
	}
	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j)
	{
		vetorIndices[j] = numRestrLambda;
		if (variaveisComRestricao[k][j] > 0)
		{
			++numRestrLambda;
		}
	}

	for (int i = 1; i <= numCommod; ++i)
	{
		if (variaveisComRestricao[k][i] == 0)
		{
			custoDualVertice = -thetaValues[i-1] - piValues[saltoK + i-1] + xiValues[saltoK + i-1];
		}
		else
		{
			custoDualVertice = -thetaValues[i-1] - piValues[saltoK + i-1] + xiValues[saltoK + i-1] - arvoreValues[vetorIndices[i]];
		}
		G->setCustoVerticeDual(i, custoDualVertice);
	}
	G->setCustoArestasDual(k);

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
	int numRestrGamma = 0;
	int *vetorIndices = new int[numCommod+1];

	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v)
	{
		for (int j = 1; j <= numCommod; ++j)
		{
			if (variaveisComRestricao[v][j] > 0)
			{
				++numRestrGamma;
			} 
		}
	}
	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j)
	{
		vetorIndices[j] = numRestrGamma;
		if (variaveisComRestricao[k][j] > 0)
		{
			++numRestrGamma;
		}
	}

	for (int i = 1; i <= numCommod; ++i)
	{
		if (variaveisComRestricao[k][i] == 0)
		{
			custoDualVertice = -muValues[i-1] + piValues[saltoK + i-1] - xiValues[saltoK + i-1];
		}
		else
		{
			custoDualVertice = -muValues[i-1] + piValues[saltoK + i-1] - xiValues[saltoK + i-1] - arvoreValues[vetorIndices[i]];
		}
		G->setCustoVerticeDual(numCommod+i, custoDualVertice);
	}
	G->setCustoArestasDual(k);

	muValues.end(); piValues.end(); xiValues.end(); arvoreValues.end();
	delete [] vetorIndices;
}


void NoArvore::atualizaCustosDuaisDirect(ModeloCplex& mCplex, Grafo* G, int k){
	float custoDualVertice;
	IloNumArray thetaValues(mCplex.env), muValues(mCplex.env), arvoreValues(mCplex.env);
	mCplex.cplexPrimal.getDuals(thetaValues, mCplex.constraintsPrimal3);
	mCplex.cplexPrimal.getDuals(muValues, mCplex.constraintsPrimal4);
	mCplex.cplexPrimal.getDuals(arvoreValues, mCplex.constraintsArvoreDelta);

	//CALCULO DOS INDICES PARA INCLUIR O CUSTO DUAL DAS RESTRICOES DA ARVORE NO VERTICE
	int numRestrDelta = 0;
	int *vetorIndices = new int[numCommod+1];

	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v)
	{
		for (int j = 1; j <= numCommod; ++j)
		{
			if (variaveisComRestricao[v][j] > 0)
			{
				++numRestrDelta;
			} 
		}
	}
	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j)
	{
		vetorIndices[j] = numRestrDelta;
		if (variaveisComRestricao[k][j] > 0)
		{
			++numRestrDelta;
		}
	}

	for (int i = 1; i <= numCommod; ++i)
	{
		if (variaveisComRestricao[k][i] == 0)
		{
			custoDualVertice = -thetaValues[i-1] - muValues[i-1];
		}
		else
		{
			custoDualVertice = -thetaValues[i-1] - muValues[i-1] - arvoreValues[vetorIndices[i]];
		}
		G->setCustoVerticeDual(i, custoDualVertice);
		G->setCustoVerticeDual(numCommod+i, 0);
	}
	G->setCustoArestasDual(k);

	thetaValues.end(); muValues.end(); arvoreValues.end();
	delete [] vetorIndices;
}


void NoArvore::inserirColunaForn(ModeloCplex& mCplex, Rota* r, int k){
	int numRestrLambda = 0;
	int *vetorIndices = new int[numCommod+1];

	//verifica quantas restricoes de lambda na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v){
		for (int j = 1; j <= numCommod; ++j)
		{
			if (variaveisComRestricao[v][j] > 0)
			{
				++numRestrLambda;
			} 
		}
	}

	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os insere no vetor
	for (int j = 1; j <= numCommod; ++j)
	{
		vetorIndices[j] = numRestrLambda;
		if (variaveisComRestricao[k][j] > 0)
		{
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
	for(int j = 1; j < numVertAtualizar; ++j)
	{
		++matrizA_no[k][qRFornNo[k]][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//Cria a variavel de decisao e insere a coluna no modelo (aproveita para criar a restricao no dual)
	IloNumColumn col = mCplex.costPrimal(r->getCusto());
	col += mCplex.constraintsPrimal1[k](1);

	for (int i = 1; i <= numCommod; i++)
	{
		if (matrizA_no[k][qRFornNo[k]][i] > 0)
		{
			col += mCplex.constraintsPrimal3[i-1](matrizA_no[k][qRFornNo[k]][i]);
			col += mCplex.constraintsPrimal5[k*numCommod + i-1](matrizA_no[k][qRFornNo[k]][i]);
			col += mCplex.constraintsPrimal6[k*numCommod + i-1](-matrizA_no[k][qRFornNo[k]][i]);

			/*VERIFICA SE PRECISA INSERIR COEFICIENTE NA COLUNA PARA AS RESTRICOES DA ARVORE*/
			if (variaveisComRestricao[k][i] > 0)
			{
				col += mCplex.constraintsArvoreLambda[vetorIndices[i]](matrizA_no[k][qRFornNo[k]][i]);
			}
		}
	}
	mCplex.lambdaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRFornNo[k];
	delete [] vetorIndices;
}


void NoArvore::inserirColunaCons(ModeloCplex& mCplex, Rota* r, int k){
	int numRestrGamma = 0;
	int *vetorIndices = new int[numCommod+1];

	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v)
	{
		for (int j = 1; j <= numCommod; ++j)
		{
			if (variaveisComRestricao[v][j] > 0)
			{
				++numRestrGamma;
			} 
		}
	}

	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j)
	{
		vetorIndices[j] = numRestrGamma;
		if (variaveisComRestricao[k][j] > 0)
		{
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
	for(int j = 1; j <= numVertAtualizar; ++j)
	{
		++matrizB_no[k][qRConsNo[k]][vertRota[j]-numCommod]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//Cria a variavel de decisao e insere a coluna no modelo
	IloNumColumn col = mCplex.costPrimal(r->getCusto());
	col += mCplex.constraintsPrimal2[k](1);

	for (int i = 1; i <= numCommod; i++)
	{
		if (matrizB_no[k][qRConsNo[k]][i] > 0)
		{
			col += mCplex.constraintsPrimal4[i-1](matrizB_no[k][qRConsNo[k]][i]);
			col += mCplex.constraintsPrimal5[k*numCommod + i-1](-matrizB_no[k][qRConsNo[k]][i]);
			col += mCplex.constraintsPrimal6[k*numCommod + i-1](matrizB_no[k][qRConsNo[k]][i]);
			
			/*VERIFICA SE PRECISA INSERIR COEFICIENTE NA COLUNA PARA AS RESTRICOES DA ARVORE*/
			if (variaveisComRestricao[k][i] > 0)
			{
				col += mCplex.constraintsArvoreGamma[vetorIndices[i]](matrizB_no[k][qRConsNo[k]][i]);
			}
		}
	}
	mCplex.gammaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRConsNo[k];
	delete [] vetorIndices;
}


void NoArvore::inserirColunaDirect(ModeloCplex& mCplex, Rota* r, int k){
	int numRestrDelta = 0;
	int *vetorIndices = new int[numCommod+1];

	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v)
	{
		for (int j = 1; j <= numCommod; ++j)
		{
			if (variaveisComRestricao[v][j] > 0)
			{
				++numRestrDelta;
			} 
		}
	}

	//para o veiculo k, verifica quantos vertices foram fixados antes de cada vertice e os adiciona no vetor
	for (int j = 1; j <= numCommod; ++j)
	{
		vetorIndices[j] = numRestrDelta;
		if (variaveisComRestricao[k][j] > 0)
		{
			++numRestrDelta;
		}
	}

	//INSERE A ROTA NO MODELO
	//Armazena a rota na matriz de rotas de consumidores do NO
	int nVertices = 2*numCommod+1;
	short int* rota = new short int[nVertices];
	memset(rota, 0, nVertices * sizeof(short int));
	matrizD_no[k].push_back(rota);
	ptrRDirect_no[k].push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j)
	{
		++matrizD_no[k][qRDirectNo[k]][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//Cria a variavel de decisao e insere a coluna no modelo
	IloNumColumn col = mCplex.costPrimal(r->getCusto());
	col += mCplex.constraintsPrimal1[k](1);
	col += mCplex.constraintsPrimal2[k](1);

	for (int i = 1; i <= numCommod; i++)
	{
		if (matrizD_no[k][qRDirectNo[k]][i] > 0)
		{
			col += mCplex.constraintsPrimal3[i-1](matrizD_no[k][qRDirectNo[k]][i]);
			col += mCplex.constraintsPrimal4[i-1](matrizD_no[k][qRDirectNo[k]][numCommod+i]);
			
			/*VERIFICA SE PRECISA INSERIR COEFICIENTE NA COLUNA PARA AS RESTRICOES DA ARVORE*/
			if (variaveisComRestricao[k][i] > 0)
			{
				col += mCplex.constraintsArvoreDelta[vetorIndices[i]](matrizD_no[k][qRDirectNo[k]][i]);
			}
		}
	}
	mCplex.deltaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRDirectNo[k];
	delete [] vetorIndices;
}


bool NoArvore::verificaFilhoViavel(Grafo* g, int iFilho){
	/*
	 * ANTES DE CRIAR O FILHO VERIFICA SE ELE, AO ESTENDER O PAI, SERA VIAVEL. PARA ISTO CERTAS RESTRICOES DEVEM SER SATISFEITAS
	 * Regras de geração de filhos VIÁVEIS no Branch and Price:
	 * 
	 **Se for atribuído o valor 3 a uma célula qualquer da coluna, as demais células desta coluna poderão assumir
	 * apenas o valor 4. O valor 3 significa que a commodity q será coletada e entregue pelo veículo k, logo não
	 * poderá ser coletada nem entregue por nenhum outro veículo (cada veículo é representado por uma linha na matriz).
	 * Sendo assim, qualquer um dos valores {1, 2, 3, 5} seria uma atribuicao inviavel
	 * 
	 **Não existem restrições quanto à atribuição do valor 4 a uma célula qualquer da coluna, exceto pelo fato de que
	 * a commodity q representada por aquela coluna deve ser coletada e entregue por algum veículo, portanto, em uma
	 * coluna deve existir ou um único valor {3, 5}, ou um valor 1 e um valor 2, significando que a commodity será coletada
	 * e entregue.
	 * 
	 **Como consequencia da restrição acima, se for atribuido 1 a uma célula qualquer da coluna, os únicos valores 
	 * possíveis para aquela coluna são 2 ou 4 (respeitando-se a restrição que deverá existir uma célula com valor 2)
	 * 
	 **De maneira análoga à restrição anterior, caso o valor atribuido a uma célula qualquer da coluna seja 2, os 
	 * únicos valores possíveis para esta mesma coluna são 1 ou 4 (respeitando-se a restrição que deverá existir 
	 * uma célula com valor 2)
	 * 
	 **Caso exista alguma celula na coluna com valor 5, nenhuma das atribuicoes {1, 2, 3} pode ser realizada. A unica 
	 * possibilidade eh a atribuicao 4, indicando que o veiculo nao coletara/entregara a mercadoria desta coluna
	 * 
	 **Existe uma regra considerando a linha da matriz (um mesmo veículo). Soma-se os pesos de todas as commodities
	 * desta linha cujo valor da célula seja {1,3} (rota do veiculo k representado pela linha para os fornecedores)
	 * ou {1,4} (rota do veiculo k representado pela linha para os consumidores). Ambos os somatórios devem ser menores
	 * do que a capacidade do veículo, caso contrário, o nó a ser gerado será inviável por capacidade.*/

	bool colunaOK = true, linhaOK = true;
	int numCelulasZero, numCelulasUm, numCelulasDois, numCelulasTres, numCelulasQuatro; 
	//percorre a coluna para verificar se todas as restricoes associadas a coluna sao satisfeitas
	switch (iFilho)
	{
		case 1:
			numCelulasZero = 0;
			numCelulasUm = 0;
			numCelulasDois = 0;
			numCelulasQuatro = 0;
			for (int k = 0; k < maxV; ++k)
			{
				if (variaveisComRestricao[k][q_branching] == 0)
				{
					++numCelulasZero;
				}
				else if ((variaveisComRestricao[k][q_branching] == 1) || (variaveisComRestricao[k][q_branching] == 3) || 
						(variaveisComRestricao[k][q_branching] == 5))
				{
					++numCelulasUm;
				}
				else if (variaveisComRestricao[k][q_branching] == 2)
				{
					++numCelulasDois;
				}
				else if (variaveisComRestricao[k][q_branching] == 4)
				{
					++numCelulasQuatro;
				}
			}
			if (numCelulasUm > 0)
			{
				colunaOK = false;
			}
			else if ((numCelulasZero == 1) && (numCelulasDois == 0))
			{
				colunaOK = false;
			}
			break;

		case 2:
			numCelulasZero = 0;
			numCelulasUm = 0;
			numCelulasDois = 0;
			numCelulasQuatro = 0;
			for (int k = 0; k < maxV; ++k)
			{
				if (variaveisComRestricao[k][q_branching] == 0)
				{
					++numCelulasZero;
				}
				else if ((variaveisComRestricao[k][q_branching] == 2) || (variaveisComRestricao[k][q_branching] == 3) || 
						(variaveisComRestricao[k][q_branching] == 5))
				{
					++numCelulasDois;
				}
				else if (variaveisComRestricao[k][q_branching] == 1)
				{
					++numCelulasUm;
				}
				else if (variaveisComRestricao[k][q_branching] == 4)
				{
					++numCelulasQuatro;
				}
			}
			if (numCelulasDois > 0)
			{
				colunaOK = false;
			}
			else if ((numCelulasZero == 1) && (numCelulasUm == 0))
			{
				colunaOK = false;
			}
			break;
		
		case 3:
		case 5:
			for (int k = 0; k < maxV; ++k)
			{
				if ( (variaveisComRestricao[k][q_branching] == 1) ||
					 (variaveisComRestricao[k][q_branching] == 2) ||
					 (variaveisComRestricao[k][q_branching] == 3) ||
					 (variaveisComRestricao[k][q_branching] == 5) )
				{
					colunaOK = false;
					break;
				}
			}
			break;

		case 4:
			numCelulasZero = 0;
			numCelulasTres = 0;
			numCelulasUm = 0;
			for (int k = 0; k < maxV; ++k)
			{
				if (variaveisComRestricao[k][q_branching] == 0)
				{
					++numCelulasZero;
				}
				else if ( (variaveisComRestricao[k][q_branching] == 3) || (variaveisComRestricao[k][q_branching] == 5) )
				{
					++numCelulasTres;
				}
				else if ( (variaveisComRestricao[k][q_branching] == 1) || (variaveisComRestricao[k][q_branching] == 2) )
				{
					++numCelulasUm;
				}
			}
			if ((numCelulasZero == 1) && ((numCelulasTres == 0) || (numCelulasUm < 2)))
			{
				colunaOK = false;
			}
			break;
	}
		
	//percorre a linha para verificar se a restricao de capacidade do veiculo eh satisfeita
	if ( ( colunaOK ) && ( iFilho != 4 ) )
	{
		int cargaRotaFornecedores = 0, cargaRotaConsumidores = 0, capacidade = g->getCapacVeiculo( k_branching );
		for ( int q = 1; q <= numCommod; ++q )
		{
			if ((variaveisComRestricao[k_branching][q] == 1) || (variaveisComRestricao[k_branching][q] == 5))
			{
				cargaRotaFornecedores += g->getPesoCommodity(q);
			}
			else if (variaveisComRestricao[k_branching][q] == 2)
			{
				cargaRotaConsumidores += g->getPesoCommodity(q+numCommod);
			}
			else if (variaveisComRestricao[k_branching][q] == 3)
			{
				cargaRotaFornecedores += g->getPesoCommodity(q);
				cargaRotaConsumidores += g->getPesoCommodity(q+numCommod);
			}
		}

		if ( (iFilho == 1) || (iFilho == 5) )
		{
			if ((cargaRotaFornecedores + g->getPesoCommodity(q_branching)) > capacidade)
			{
				linhaOK = false;
			}
		}
		else if (iFilho == 2)
		{
			if ((cargaRotaConsumidores + g->getPesoCommodity(q_branching+numCommod)) > capacidade)
			{
				linhaOK = false;
			}
		}
		else if (iFilho == 3)
		{
			if ((cargaRotaFornecedores + g->getPesoCommodity(q_branching) > capacidade) || 
				 (cargaRotaConsumidores + g->getPesoCommodity(q_branching+numCommod) > capacidade))
			{
				linhaOK = false;
			}
		}
	}
	return (colunaOK && linhaOK);
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
