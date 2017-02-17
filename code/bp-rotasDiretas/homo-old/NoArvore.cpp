#include "NoArvore.h"

Grafo* NoArvore::G;
ModeloCplex* NoArvore::mCplex;
int NoArvore::numRequests;
int NoArvore::numVertices;
int NoArvore::totalNosAtivos;
int NoArvore::totalNosCriados;

NoArvore::~NoArvore(){
	--totalNosAtivos;
	for (int r = 0; r < qRotas_no; ++r)
	{
		delete [] a_ir_no[r];
		if (ptrRotas_no[r]->decrNumApontadores()) delete ptrRotas_no[r];
	}
	for (int r = 0; r < qRotasD_no; ++r)
	{
		delete [] d_ir_no[r];
		if (ptrRotasD_no[r]->decrNumApontadores()) delete ptrRotasD_no[r];
	}
	for (int i = 0; i < arcosBranching.size(); ++i)
	{
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
	qRotasD_no = 0;
	indiceNo = 0;
	int i, j;

	//instancia os objetos do cplex para manipular as restricoes e variaveis do branching
	mCplex->lambda_no = IloNumVarArray(mCplex->env);
	mCplex->delta_no = IloNumVarArray(mCplex->env);
	mCplex->artificiais = IloNumVarArray(mCplex->env);
	mCplex->constraintsArvore = IloRangeArray(mCplex->env);

	//variaveis i e j passadas como referencia. armazenarao o arco a ser realizado o branching.
	if ( defineVariavelBranching( i, j ) )
	{
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
	qRotasD_no = pai->qRotasD_no;	
	limiteDual = pai->limiteDual;
	for (int r = 0; r < qRotas_no; ++r) //rotas com trocas
	{
		tmp = new short int[numVertices+1];
		for (int i = 1; i <= numVertices; ++i)
		{
			tmp[i] = pai->a_ir_no[r][i];
		}
		a_ir_no.push_back(tmp);
		ptrRotas_no.push_back(pai->ptrRotas_no[r]);
		ptrRotas_no[r]->incrNumApontadores();
	}
	for (int r = 0; r < qRotasD_no; ++r) //rotas diretas
	{
		tmp = new short int[numVertices+1];
		for (int i = 1; i <= numVertices; ++i)
		{
			tmp[i] = pai->d_ir_no[r][i];
		}
		d_ir_no.push_back(tmp);
		ptrRotasD_no.push_back(pai->ptrRotasD_no[r]);
		ptrRotasD_no[r]->incrNumApontadores();
	}

	//herda os arcos fixos do veiculo k
	for (int i = 0; i < pai->arcosBranching.size(); i++)
	{
		tmp = new short int[3];
		tmp[0] = pai->arcosBranching[i][0];
		tmp[1] = pai->arcosBranching[i][1];
		tmp[2] = pai->arcosBranching[i][2];
		arcosBranching.push_back(tmp);
	}
	//por fim, inclue o arco de branching passado como parametro (o ultimo definido para executar o branching)
	arcosBranching.push_back(arcoBranching);
}


float NoArvore::executaBranching(vector<ptrNo>& nosCriados, int* vetReqsCD, int* vetReqsPD, int nReqsCD, int nReqsPD, char opSub, char opSubRD, int tempoTotal){
	//limpa as informacoes do no processado anteriormente no ModeloCplex (variaveis de decisao, artificiais e restricoes da arvore)
	mCplex->lambda_no.endElements();
	mCplex->delta_no.endElements();
	mCplex->artificiais.endElements();
	mCplex->constraintsArvore.endElements();

	//insere no ModeloCplex as informacoes pertinentes ao no
	setVariaveisDecisao_no();
	setRestricoes_no();

	//Uma vez que o modelo deste no esta montado, eh possivel chegar na raiz atraves da geracao de colunas
	//ao fixar os arcos, eh possivel que o modelo se torne inviavel, neste caso, interrompe a geracao de filhos (poda por inviabilidade)
	if (alcancaRaiz_no(vetReqsCD, vetReqsPD, nReqsCD, nReqsPD, opSub, opSubRD, tempoTotal))
	{
		//variaveis i e j passadas como referencia (arco a ser realizado o branching)
		int i, j;
		if (defineVariavelBranching(i, j))
		{
			//caso o limite dual do no seja maior ou igual ao limite o primal, nao gera os filhos (poda por limite dual)
			if (limiteDual < ModeloCplex::getLimitePrimal())
			{ 
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
		}
		else //caso nao existam variaveis a executar Branching (todas variaveis sao inteiras), nao cria-se filhos (poda por integralidade)
		{
			return limiteDual;
		}
	}
	else
	{
		return 999999;
	}
}


void NoArvore::setVariaveisDecisao_no(){
	//variaveis LAMBDA
	for (int r = 0; r < qRotas_no; ++r)
	{
		IloNumColumn col = mCplex->costPrimal(ptrRotas_no[r]->getCusto());
		col += mCplex->constraintP1(1);

		for (int i = 1; i <= numVertices; i++)
		{
			if (a_ir_no[r][i] > 0) col += mCplex->constraintsP2[i-1](a_ir_no[r][i]);
		}
		for (int i = 1; i <= numRequests; i++)
		{
			if ((a_ir_no[r][i] == 0) && (a_ir_no[r][i+numRequests] > 0)) col += mCplex->constraintsP3[i-1](-1);
		}

		char nome[20];
		sprintf(nome, "lNo%d", r);
		mCplex->lambda_no.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
		col.end();
	}
	
	//variaveis DELTA
	for (int r = 0; r < qRotasD_no; ++r)
	{
		IloNumColumn col = mCplex->costPrimal(ptrRotasD_no[r]->getCusto());
		col += mCplex->constraintP1(1);

		for (int i = 1; i <= numVertices; i++)
		{
			if (d_ir_no[r][i] > 0) col += mCplex->constraintsP2[i-1](d_ir_no[r][i]);
		}

		char nome[20];
		sprintf(nome, "dNo%d", r);
		mCplex->delta_no.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
		col.end();
	}
}


void NoArvore::setRestricoes_no(){
	int vFixoI, vFixoJ, quantV, coefColuna, numVisitasArco;
	vector<int> verticesRota;
	int numRestrArcos = 0;

	for (int i = 0; i < arcosBranching.size(); ++i)
	{
		IloExpr exp = IloExpr(mCplex->env);
		vFixoI = arcosBranching[i][0];
		vFixoJ = arcosBranching[i][1];

		//insere na restricao as variaveis de decisao LAMBDA do no raiz 
		for (int r = 0; r < mCplex->qRotas; ++r)
		{
			verticesRota = mCplex->ptrRotas[r]->getVertices();
			quantV = verticesRota.size()-2;
			numVisitasArco = 0;
			for (int i = 0; i < quantV; ++i)
			{
				if ((verticesRota[i] == vFixoI) && (verticesRota[i+1] == vFixoJ))
				{
					++numVisitasArco;
				}
			}
			if (numVisitasArco > 0) exp += numVisitasArco * mCplex->lambda[r];
		}

		//insere na restricao as variaveis de decisao LAMBDA deste no
		for (int r = 0; r < qRotas_no; ++r)
		{
			verticesRota = ptrRotas_no[r]->getVertices();
			quantV = verticesRota.size()-2;
			numVisitasArco = 0;
			for (int i = 0; i < quantV; ++i)
			{
				if ((verticesRota[i] == vFixoI) && (verticesRota[i+1] == vFixoJ))
				{
					++numVisitasArco;
				}
			}
			if (numVisitasArco > 0) exp += numVisitasArco * mCplex->lambda_no[r];
		}

		//variaveis de decisao DELTA na raiz
		for (int r = 0; r < mCplex->qRotasD; ++r)
		{
			verticesRota = mCplex->ptrRotasD[r]->getVertices();
			quantV = verticesRota.size()-2;
			numVisitasArco = 0;
			for (int i = 0; i < quantV; ++i)
			{
				if ((verticesRota[i] == vFixoI) && (verticesRota[i+1] == vFixoJ))
				{
					++numVisitasArco;
				}
			}
			if (numVisitasArco > 0) exp += numVisitasArco * mCplex->delta[r];
		}

		//variaveis de decisao DELTA deste no
		for (int r = 0; r < qRotasD_no; ++r)
		{
			verticesRota = ptrRotasD_no[r]->getVertices();
			quantV = verticesRota.size()-2;
			numVisitasArco = 0;
			for (int i = 0; i < quantV; ++i)
			{
				if ((verticesRota[i] == vFixoI) && (verticesRota[i+1] == vFixoJ))
				{
					++numVisitasArco;
				}
			}
			if (numVisitasArco > 0) exp += numVisitasArco * mCplex->delta_no[r];
		}

		//a variavel coefColuna sera usada para controlar o indice da variavel artificial nesta restricao
		if (arcosBranching[i][2] == 1)
		{
			mCplex->constraintsArvore.add(exp == 1);
			coefColuna = 1;
		}
		else
		{
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


bool NoArvore::alcancaRaiz_no(int* vetReqsCD, int* vetReqsPD, int nReqsCD, int nReqsPD, char opSub, char opSubRD, int tempo){
	Rota** rotasCD;
	Rota** rotasDiretas;
	bool pricingHeuristica;
	int aux, pricingRD, pricingCD;
	pricingRD = pricingCD = 1;

	do{
		pricingHeuristica = false;

		if ( nReqsCD < numRequests )
		{
			mCplex->solveMaster();
			atualizaCustosDuaisRD(vetReqsCD, nReqsCD);
			GRASPRD graspRD(G, numRequests, 2*numRequests+1, 0.5);
			aux = graspRD.run(200*numRequests, mCplex->getAlfa());
			if ( aux > 0 )
			{
				pricingHeuristica = true;
				for (int h = 0; h < aux; ++h) inserirColunaDireta_no(graspRD.getRotaConvertida(h));
			}
		}
		if ( nReqsPD < numRequests )
		{
			mCplex->solveMaster();
			atualizaCustosDuais(vetReqsPD, nReqsPD);
			GRASP grasp(G, numRequests, 2*numRequests+1, mCplex->getXi(), 0.5);
			aux = grasp.run(200*numRequests, mCplex->getAlfa());
			if (aux > 0)
			{
				pricingHeuristica = true;
				for (int h = 0; h < aux; ++h) inserirColuna_no(grasp.getRotaConvertida(h));
			}
		}
		if ( pricingHeuristica ) pricingRD = pricingCD = 1;

		//execucao do pricing exato para rotas que passam no CD
		if ( ( !pricingHeuristica ) && ( pricingCD == 1 ) && ( nReqsPD < numRequests ) )
		{
			if ( opSub == 'B' )
			{
				ModeloBC BCE(G, mCplex->getXi(), mCplex->getAlfa());
				BCE.calculaCaminhoElementar(G, tempo);
				aux = BCE.rotasNegativas.size();
				if ( aux > 0 ) //significa que encontrou rota(s) de custo reduzido negativo
				{
					for (int y = 0; y < aux; ++y) inserirColuna_no( BCE.rotasNegativas[y] );
				}
				else pricingCD = 0;
			}
			else if ( opSub == 'P' )
			{
				ESPPRCbi *bi = new ESPPRCbi(G, mCplex->getXi(), mCplex->getAlfa());
				bi->calculaCaminhoElementarBi(G);
				rotasCD = bi->getRotaCustoMinimo(G, 0.9);
				if ( rotasCD != NULL )
				{
					for ( int y = 0; rotasCD[y] != NULL; ++y ) inserirColuna_no(rotasCD[y]);
				}
				else pricingCD = 0;
				delete bi;
			}
		}

		//execucao do pricing exato para rotas diretas
		if ( ( !pricingHeuristica ) && ( pricingRD == 1 ) && ( nReqsCD < numRequests ) )
		{
			mCplex->solveMaster();
			atualizaCustosDuaisRD(vetReqsCD, nReqsCD);
			if ( opSubRD == 'B' )
			{
				ModeloBC_RD bcRD(G, mCplex->getAlfa());
				bcRD.calculaCaminhoElementar(G, tempo);
				int aux = bcRD.rotasNegativas.size();
				if ( aux > 0 )
				{
					for ( int i = 0; i < aux; ++i ) inserirColunaDireta_no(bcRD.rotasNegativas[i]);
				}
				else pricingRD = 0;
			}
			else if ( opSubRD == 'P' )
			{
				ElementarRD camElemRD(G, mCplex->getAlfa());
				if ( camElemRD.calculaCaminhoElementar(G, tempo) > 0 )
				{
					rotasDiretas = camElemRD.getRotaCustoMinimo(G, 0.9);
					if ( rotasDiretas != NULL )
					{
						for ( int i = 0; rotasDiretas[i] != NULL; ++i ) inserirColunaDireta_no(rotasDiretas[i]);
					}
					else pricingRD = 0;
				}
				else pricingRD = 0;
			}
		}

		if ( ( pricingRD == 0 ) && ( pricingCD == 1 ) ) pricingRD = -1;
		if ( ( pricingCD == 0 ) && ( pricingRD == 1 ) ) pricingCD = -1;

		if ( ( pricingCD == 0 ) && ( pricingRD == -1 ) ) pricingRD = 1;
		if ( ( pricingRD == 0 ) && ( pricingCD == -1 ) ) pricingCD = 1;

	}while( !( !pricingHeuristica && ( pricingCD == 0 ) && ( pricingRD == 0 ) ) );

	IloNumArray valoresVarArtif(mCplex->env);
	mCplex->cplexPrimal.getValues(valoresVarArtif, mCplex->artificiais);
	int numVarArtificiais = valoresVarArtif.getSize();
	for (int i = 0; i < numVarArtificiais; ++i)
	{
		if (valoresVarArtif[i] > 0.00001) return false;
	}

	limiteDual = mCplex->solveMaster(); //armazena o limite dual oferecido pelo no
	return true;
}


void NoArvore::atualizaCustosDuais(int* vetReqsPD, int nReqsPD){
	//insere nos vertices do grafo os valores das variaveis duais theta (como eh feito na raiz)
	IloNumArray dualValuesConstraintsP2(mCplex->env);
	mCplex->cplexPrimal.getDuals(dualValuesConstraintsP2, mCplex->constraintsP2);
	for (int i = 1; i <= numVertices; ++i)
	{
		G->setCustoVerticeDual(i, -dualValuesConstraintsP2[i-1]);
	}
	G->setCustoArestasDual(vetReqsPD, nReqsPD, false);

	//insere no grafo os custos duais associados aos arcos que foram fixados para este no
	float custoAresta;
	int* indiceVertDual = G->getIndiceVerticesDual();
	int iDual, jDual, numVertDual = G->getNumVerticesDual();
	for (int i = 0; i < arcosBranching.size(); ++i)
	{
		iDual = jDual = -1;
		for ( int j = 0; j < numVertDual; ++j )
		{
			if ( arcosBranching[i][0] == indiceVertDual[j] ) iDual = j;
			else if ( arcosBranching[i][1] == indiceVertDual[j] ) jDual = j;
		}
		if ( ( iDual >= 0 ) && ( jDual >= 0 ) )
		{
			custoAresta = G->getCustoArestaDual(iDual, jDual);
			G->setCustoArestaDual(iDual, jDual, (custoAresta - mCplex->cplexPrimal.getDual(mCplex->constraintsArvore[i])));
		}
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


void NoArvore::atualizaCustosDuaisRD(int* vetReqsCD, int nReqsCD){
	//insere nos vertices do grafo os valores das variaveis duais theta (como eh feito na raiz)
	IloNumArray dualValuesConstraintsP2(mCplex->env);
	mCplex->cplexPrimal.getDuals(dualValuesConstraintsP2, mCplex->constraintsP2);
	for (int i = 1; i <= numRequests; ++i){
		G->setCustoVerticeDual(i, -dualValuesConstraintsP2[i-1] - dualValuesConstraintsP2[i+numRequests-1]);
		G->setCustoVerticeDual(i+numRequests, 0);
	}
	G->setCustoArestasDual(vetReqsCD, nReqsCD, true);

	//insere no grafo os custos duais associados aos arcos que foram fixados para este no
	float custoAresta;
	int* indiceVertDual = G->getIndiceVerticesDual();
	int iDual, jDual, numVertDual = G->getNumVerticesDual();
	for (int i = 0; i < arcosBranching.size(); ++i)
	{
		iDual = jDual = -1;
		for ( int j = 0; j < numVertDual; ++j )
		{
			if ( arcosBranching[i][0] == indiceVertDual[j] ) iDual = j;
			else if ( arcosBranching[i][1] == indiceVertDual[j] ) jDual = j;
		}
		if ( ( iDual >= 0 ) && ( jDual >= 0 ) )
		{
			custoAresta = G->getCustoArestaDual(iDual, jDual);
			G->setCustoArestaDual(iDual, jDual, (custoAresta - mCplex->cplexPrimal.getDual(mCplex->constraintsArvore[i])));
		}
	}

	//armzena o valor de alfa e libera memoria
	mCplex->alfaPtInterior = mCplex->cplexPrimal.getDual(mCplex->constraintP1);
	dualValuesConstraintsP2.end();
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
	for(int j = 1; j < numVertAtualizar; ++j)
	{
		++a_ir_no[qRotas_no][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA NESTE NO
	IloNumColumn col = mCplex->costPrimal(r->getCusto());
	col += mCplex->constraintP1(1);
	for (int i = 1; i <= numVertices; i++)
	{
		if (a_ir_no[qRotas_no][i] > 0)
		{
			col += mCplex->constraintsP2[i-1](a_ir_no[qRotas_no][i]);
		}
	}
	for (int i = 1; i <= numRequests; i++)
	{
		if ((a_ir_no[qRotas_no][i] == 0) && (a_ir_no[qRotas_no][i+numRequests] > 0))
		{
			col += mCplex->constraintsP3[i-1](-1);
		}
	}
	//Inclui na coluna os indices associados as restricoes de branching em arcos, caso existam
	int numVisitasArco;
	for (int i = 0; i < arcosBranching.size(); ++i)
	{
		numVisitasArco = 0;
		for (int j = 0; j < numVertAtualizar; ++j)
		{
			if ((vertRota[j] == arcosBranching[i][0]) && ((vertRota[j+1] == arcosBranching[i][1])))
			{
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


void NoArvore::inserirColunaDireta_no(Rota* r){
	//Armazena a rota na matriz de rotas do no
	short int* rota = new short int[numVertices+1];
	memset(rota, 0, (numVertices+1) * sizeof(short int));
	d_ir_no.push_back(rota);
	ptrRotasD_no.push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-1;
	for(int j = 1; j < numVertAtualizar; ++j)
	{
		++d_ir_no[qRotasD_no][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//CRIA-SE UMA COLUNA ASSOCIADA A ROTA NESTE NO
	IloNumColumn col = mCplex->costPrimal(r->getCusto());
	col += mCplex->constraintP1(1);
	for (int i = 1; i <= numVertices; i++)
	{
		if (d_ir_no[qRotasD_no][i] > 0)
		{
			col += mCplex->constraintsP2[i-1](d_ir_no[qRotasD_no][i]);
		}
	}

	//Inclui na coluna os indices associados as restricoes de branching em arcos, caso existam
	int numVisitasArco;
	for (int i = 0; i < arcosBranching.size(); ++i)
	{
		numVisitasArco = 0;
		for (int j = 0; j < numVertAtualizar; ++j)
		{
			if ((vertRota[j] == arcosBranching[i][0]) && ((vertRota[j+1] == arcosBranching[i][1])))
			{
				++numVisitasArco;
			}
		}
		if (numVisitasArco > 0)	col += mCplex->constraintsArvore[i](numVisitasArco);
	}

	char nome[20];
	sprintf(nome, "dNo%d", qRotasD_no);
	mCplex->delta_no.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
	col.end();
	++qRotasD_no;
}


bool NoArvore::defineVariavelBranching(int& inicioArco, int& fimArco){
	IloNumArray lambdaValues(mCplex->env);
	IloNumArray deltaValues(mCplex->env);
	int tamRota, totalVertices = numVertices+2;
	vector<int> verticesRota;

	//Cada celula da matriz representara um arco (i,j) que sera preenchido em funcao da violacao
	//Aquele arco (i,j) que tiver o maior valor de violacao sera fixado no branching de variaveis
	float** matrizViolacao = new float*[totalVertices];
	for (int i = 0; i < totalVertices; ++i)
	{
		matrizViolacao[i] = new float[totalVertices];
		memset(matrizViolacao[i], 0, (totalVertices) * sizeof(float));
	}

	//primeiro busca por variaveis fracionarias entre as variaveis LAMBDA obtidas na raiz
	mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda);
	for (int r = 0; r < mCplex->qRotas; ++r)
	{
		if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999)) //lambda[r] da raiz eh fracionaria
		{
			verticesRota = mCplex->ptrRotas[r]->getVertices();
			tamRota = verticesRota.size() - 2;
			for (int i = 0; i < tamRota; ++i)
			{
				matrizViolacao[verticesRota[i]][verticesRota[i+1]] += lambdaValues[r];
			}
		}
	}

	//depois computa quais variaveis fracionarias LAMBDA do no
	if (qRotas_no > 0)
	{
		mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda_no);
		for (int r = 0; r < qRotas_no; ++r)
		{
			if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999))
			{
				verticesRota = ptrRotas_no[r]->getVertices();
				tamRota = verticesRota.size() - 2;
				for (int i = 0; i < tamRota; ++i)
				{
					matrizViolacao[verticesRota[i]][verticesRota[i+1]] += lambdaValues[r];
				}
			}
		}
	}
	
	//primeiro busca por variaveis fracionarias entre as variaveis DELTA obtidas na raiz
	mCplex->cplexPrimal.getValues(deltaValues, mCplex->delta);
	for (int r = 0; r < mCplex->qRotasD; ++r)
	{
		if ((deltaValues[r] > 0.00001) && (deltaValues[r] < 0.99999)) //delta[r] da raiz eh fracionaria
		{
			verticesRota = mCplex->ptrRotasD[r]->getVertices();
			tamRota = verticesRota.size() - 2;
			for (int i = 0; i < tamRota; ++i)
			{
				matrizViolacao[verticesRota[i]][verticesRota[i+1]] += deltaValues[r];
			}
		}
	}

	//depois computa quais variaveis fracionarias DELTA do no
	if (qRotasD_no > 0)
	{
		mCplex->cplexPrimal.getValues(deltaValues, mCplex->delta_no);
		for (int r = 0; r < qRotasD_no; ++r)
		{
			if ((deltaValues[r] > 0.00001) && (deltaValues[r] < 0.99999))
			{
				verticesRota = ptrRotasD_no[r]->getVertices();
				tamRota = verticesRota.size() - 2;
				for (int i = 0; i < tamRota; ++i)
				{
					matrizViolacao[verticesRota[i]][verticesRota[i+1]] += deltaValues[r];
				}
			}
		}
	}

	//obtem o VALOR do(s) arco(s) mais violado(s)
	float violacao = 0, valorRealViolacao;
	for (int i = 0; i < totalVertices; ++i)
	{
		for (int j = 0; j < totalVertices; ++j)
		{
			if ( min( matrizViolacao[i][j], ( 1-matrizViolacao[i][j] ) ) > ( violacao + 0.00001 ) )
			{
				valorRealViolacao = matrizViolacao[i][j];
				violacao = min( matrizViolacao[i][j], ( 1-matrizViolacao[i][j] ) );
			}
		}
	}

	if ( violacao > 0.0001 )
	{
		vector < int > arcosMaisViolados; //armazena aqueles arcos com maior valor de violacao (calculado anteriormente)
		vector < IloExpr > exprArcosMaisViolados; //cada arco que tenha valor igual ao mais violado tera uma expressao no cplex, para obter seu valor dual
		for (int i = 0; i < totalVertices; ++i)
		{
			for (int j = 0; j < totalVertices; ++j)
			{
				if ( ( matrizViolacao[i][j] > ( valorRealViolacao-0.0001 ) ) && ( matrizViolacao[i][j] < ( valorRealViolacao+0.0001 ) ) )
				{
					arcosMaisViolados.push_back(i);
					arcosMaisViolados.push_back(j);
					exprArcosMaisViolados.push_back(IloExpr(mCplex->env));
				}
			}
		}

		if (exprArcosMaisViolados.size() > 1) //existem arcos com o mesmo valor de violacao. o desempate usara os valores dos custos reduzidos
			//busca novamente as variaveis de decisao fracionarias, mas desta vez para compor as restricoes (\sum{arcoViolado}lambda = maiorViolacao)
		{
			int aux, numArcosMaisViolados = exprArcosMaisViolados.size();
			mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda); //primeiro as variaveis de decisao LAMBDA da raiz
			for (int r = 0; r < mCplex->qRotas; ++r)
			{
				if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999)) //variavel eh fracionaria
				{
					verticesRota = mCplex->ptrRotas[r]->getVertices();
					tamRota = verticesRota.size() - 2;
					aux = 0;
					for (int i = 0; i < 2*numArcosMaisViolados; i+=2, ++aux)
					{
						for (int j = 0; j < tamRota; ++j)
						{
							if ((arcosMaisViolados[i] == verticesRota[j]) && (arcosMaisViolados[i+1] == verticesRota[j+1]))
							{
								exprArcosMaisViolados[aux] += mCplex->lambda[r]; break;
							}
						}
					}
				}
			}

			if (qRotas_no > 0)
			{
				mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambda_no); //depois as variaveis de decisao LAMBDA deste no
				for (int r = 0; r < qRotas_no; ++r)
				{
					if ((lambdaValues[r] > 0.00001) && (lambdaValues[r] < 0.99999)) //lambda[r] da raiz eh fracionaria
					{
						verticesRota = ptrRotas_no[r]->getVertices();
						tamRota = verticesRota.size() - 2;
						aux = 0;
						for (int i = 0; i < 2*numArcosMaisViolados; i+=2, ++aux)
						{
							for (int j = 0; j < tamRota; ++j)
							{
								if ((arcosMaisViolados[i] == verticesRota[j]) && (arcosMaisViolados[i+1] == verticesRota[j+1]))
								{
									exprArcosMaisViolados[aux] += mCplex->lambda_no[r]; break;
								}
							}
						}
					}
				}
			}
			
			mCplex->cplexPrimal.getValues(deltaValues, mCplex->delta); //primeiro as variaveis de decisao DELTA da raiz
			for (int r = 0; r < mCplex->qRotasD; ++r)
			{
				if ((deltaValues[r] > 0.00001) && (deltaValues[r] < 0.99999)) //variavel eh fracionaria
				{
					verticesRota = mCplex->ptrRotasD[r]->getVertices();
					tamRota = verticesRota.size() - 2;
					aux = 0;
					for (int i = 0; i < 2*numArcosMaisViolados; i+=2, ++aux)
					{
						for (int j = 0; j < tamRota; ++j)
						{
							if ((arcosMaisViolados[i] == verticesRota[j]) && (arcosMaisViolados[i+1] == verticesRota[j+1]))
							{
								exprArcosMaisViolados[aux] += mCplex->delta[r]; break;
							}
						}
					}
				}
			}

			if (qRotasD_no > 0)
			{
				mCplex->cplexPrimal.getValues(deltaValues, mCplex->delta_no); //depois as variaveis de decisao DELTA deste no
				for (int r = 0; r < qRotasD_no; ++r)
				{
					if ((deltaValues[r] > 0.00001) && (deltaValues[r] < 0.99999)) //delta[r] da raiz eh fracionaria
					{
						verticesRota = ptrRotasD_no[r]->getVertices();
						tamRota = verticesRota.size() - 2;
						aux = 0;
						for (int i = 0; i < 2*numArcosMaisViolados; i+=2, ++aux)
						{
							for (int j = 0; j < tamRota; ++j)
							{
								if ((arcosMaisViolados[i] == verticesRota[j]) && (arcosMaisViolados[i+1] == verticesRota[j+1]))
								{
									exprArcosMaisViolados[aux] += mCplex->delta_no[r]; break;
								}
							}
						}
					}
				}
			}

			//RELAXA A RESTRICAO DE CONVEXIDADE, DEIXANDO-A PARA AS NOVAS RESTRICOES A SEREM INSERIDAS
			mCplex->constraintP1.setBounds(0, 2*mCplex->maxVeic);
			IloRangeArray constraintsSetArcoBranching(mCplex->env);
			for (int i = 0; i < numArcosMaisViolados; ++i)
			{
				constraintsSetArcoBranching.add(exprArcosMaisViolados[i] == valorRealViolacao);
			}
			mCplex->modelPrimal.add(constraintsSetArcoBranching);
			mCplex->solveMaster();

			//obtidas as expressoes, cria-se as restricoes, igualando todas ao valor do arco mais violado (pois elas estao empatadas neste valor)
			float maiorCustoDual = -MAIS_INFINITO;
			mCplex->cplexPrimal.getDuals(lambdaValues, constraintsSetArcoBranching);
			for (int i = 0; i < numArcosMaisViolados; ++i)
			{
				if (fabs(lambdaValues[i]) > maiorCustoDual)
				{
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
		}
		else //existe apenas um arco cujo valor eh o mais violado, portanto, este arco sera retornado para o branching
		{
			inicioArco = arcosMaisViolados[0];
			fimArco = arcosMaisViolados[1];
		}
	}

	//libera memoria
	for (int i = 0; i < totalVertices; i++) delete [] matrizViolacao[i];
	delete [] matrizViolacao;
	lambdaValues.end();
	deltaValues.end();

	if ( violacao > 0.0001 ) return true; //existem arcos para executar branching
	else return false; //solucao inteira, nao executa-se branching e poda por integralidade
}

void NoArvore::imprimir() {
	printf("indice = %d\n", indiceNo);
	printf("valor = %f\n", limiteDual);
	printf("qRotas_no = %d\n", qRotas_no);
	printf("qRotasD_no = %d\n", qRotasD_no);
	for (int r = 0; r < qRotas_no; ++r)
	{
		printf(" ");
		ptrRotas_no[r]->imprimir();
	}
	for (int r = 0; r < qRotasD_no; ++r)
	{
		printf("*");
		ptrRotasD_no[r]->imprimir();
	}
	printf("Arcos com restricao:\n");
	for (int i = 0; i < arcosBranching.size(); ++i)
	{
		printf("  (%d, %d) = %d\n", arcosBranching[i][0], arcosBranching[i][1], arcosBranching[i][2]);
	}
	printf("\n");
}

void NoArvore::setMelhoresRotas(ModeloCplex* mCplex){
	int k = 0;
	IloNumArray values(mCplex->env);

	//ROTAS COM TROCAS
	mCplex->cplexPrimal.getValues(values, mCplex->lambda);
	for ( int r = 0; r < mCplex->qRotas; ++r )
	{
		if (values[r] > 0.99)
		{
			if ( mCplex->melhoresRotas[k]->decrNumApontadores() ) delete mCplex->melhoresRotas[k];
			mCplex->melhoresRotas[k] = mCplex->ptrRotas[r];
			mCplex->melhoresRotas[k]->incrNumApontadores();
			++k;
		}
	}

	mCplex->cplexPrimal.getValues(values, mCplex->lambda_no);
	for ( int r = 0; r < qRotas_no; ++r )
	{
		if (values[r] > 0.99)
		{
			if ( mCplex->melhoresRotas[k]->decrNumApontadores() ) delete mCplex->melhoresRotas[k];
			mCplex->melhoresRotas[k] = ptrRotas_no[r];
			mCplex->melhoresRotas[k]->incrNumApontadores();
			++k;
		}
	}

	//ROTAS DIRETAS
	mCplex->cplexPrimal.getValues(values, mCplex->delta);
	for ( int r = 0; r < mCplex->qRotasD; ++r )
	{
		if (values[r] > 0.99)
		{
			if ( mCplex->melhoresRotas[k]->decrNumApontadores() ) delete mCplex->melhoresRotas[k];
			mCplex->melhoresRotas[k] = mCplex->ptrRotasD[r];
			mCplex->melhoresRotas[k]->incrNumApontadores();
			++k;
		}
	}

	mCplex->cplexPrimal.getValues(values, mCplex->delta_no);
	for ( int r = 0; r < qRotasD_no; ++r )
	{
		if (values[r] > 0.99)
		{
			if ( mCplex->melhoresRotas[k]->decrNumApontadores() ) delete mCplex->melhoresRotas[k];
			mCplex->melhoresRotas[k] = ptrRotasD_no[r];
			mCplex->melhoresRotas[k]->incrNumApontadores();
			++k;
		}
	}
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
