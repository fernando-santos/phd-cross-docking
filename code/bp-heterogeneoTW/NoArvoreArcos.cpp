#include "NoArvoreArcos.h"

int NoArvoreArcos::maxV;
int NoArvoreArcos::numCommod;
int NoArvoreArcos::totalNosCriados;
ModeloCplex* NoArvoreArcos::mCplex;
char** NoArvoreArcos::variaveisComRestricao;

NoArvoreArcos::~NoArvoreArcos(){
	for (int k = 0; k < maxV; ++k){
		for (int r = 0; r < qRFornNo[k]; ++r){
			delete [] matrizA_no[k][r];
			if (ptrRForn_no[k][r]->decrNumApontadores()) delete ptrRForn_no[k][r];
		}
		for (int r = 0; r < qRConsNo[k]; ++r){
			delete [] matrizB_no[k][r];
			if (ptrRCons_no[k][r]->decrNumApontadores()) delete ptrRCons_no[k][r];
		}
		for (int i = 0; i < arcosBranching[k].size(); ++i){
			delete [] arcosBranching[k][i];
		}
		matrizA_no[k].clear();
		matrizB_no[k].clear();
		ptrRForn_no[k].clear();
		ptrRCons_no[k].clear();
		arcosBranching[k].clear();
	}
	delete [] qRFornNo;
	delete [] qRConsNo;
	delete [] matrizA_no;
	delete [] matrizB_no;
	delete [] ptrRForn_no;
	delete [] ptrRCons_no;
	delete [] arcosBranching;
}

NoArvoreArcos::NoArvoreArcos() {
	indiceNo = ++totalNosCriados;
	qRFornNo = new int[maxV];
	qRConsNo = new int[maxV];
	for (int k = 0; k < maxV; ++k) {
		qRFornNo[k] = 0;
		qRConsNo[k] = 0;
	}
	arcosBranching = new vector<short int*> [maxV];
	matrizA_no = new vector<short int*> [maxV];
	matrizB_no = new vector<short int*> [maxV];
	ptrRForn_no = new vector<Rota*> [maxV];
	ptrRCons_no = new vector<Rota*> [maxV];
	//Inicializa os objetos do ModeloCplex que serao utilizados no branching de arcos
	mCplex->constraintsArvoreArcos = IloRangeArray(mCplex->env);
	mCplex->artificiais = IloNumVarArray(mCplex->env);
	mCplex->lambdaNo = IloArray<IloNumVarArray> (mCplex->env, maxV);
	mCplex->gammaNo = IloArray<IloNumVarArray> (mCplex->env, maxV);
	for (int k = 0; k < maxV; k++) {
		mCplex->lambdaNo[k] = IloNumVarArray(mCplex->env);
		mCplex->gammaNo[k] = IloNumVarArray(mCplex->env);
	}
	mCplex->cplexPrimal.solve();

	//determina o arco (i,j) de um veiculo k no qual sera feito o branching dos filhos
	procuraArcoMaisViolado();
}

NoArvoreArcos::NoArvoreArcos(ptrNoArcos pai, int valorFixarArco) {
	indiceNo = ++totalNosCriados;
	qRFornNo = new int[maxV];
	qRConsNo = new int[maxV];

	short int* rota, *tmp;
	arcosBranching = new vector<short int*> [maxV];
	matrizA_no = new vector<short int*> [maxV];
	matrizB_no = new vector<short int*> [maxV];
	ptrRForn_no = new vector<Rota*> [maxV];
	ptrRCons_no = new vector<Rota*> [maxV];

	//matriz para armazenar para cada par (k,q) fixo seu respectivo indice na restricao da arvore
	int numRestricoes = 0;
	int** matrizIndices = new int*[maxV];
	for (int k = 0; k < maxV; ++k) {
		matrizIndices[k] = new int[numCommod + 1];
		for (int i = 1; i <= numCommod; ++i) {
			if (variaveisComRestricao[k][i] > 0) {
				matrizIndices[k][i] = numRestricoes;
				++numRestricoes;
			} else {
				matrizIndices[k][i] = -1;
			}
		}
	}

	//laco para copiar as informacoes do pai e construir o modelo do no que esta sendo criado
	for (int k = 0; k < maxV; ++k) {
		qRFornNo[k] = pai->qRFornNo[k];
		qRConsNo[k] = pai->qRConsNo[k];

		//herda do pai as rotas dos fornecedores do veiculo k
		for (int r = 0; r < qRFornNo[k]; ++r) {
			rota = new short int[numCommod + 1];
			for (int i = 1; i <= numCommod; ++i) {
				rota[i] = pai->matrizA_no[k][r][i];
			}
			matrizA_no[k].push_back(rota);
			ptrRForn_no[k].push_back(pai->ptrRForn_no[k][r]);
			ptrRForn_no[k][r]->incrNumApontadores();

			//cria a variavel de decisao associada a esta rota e a insere no modelo
			IloNumColumn col = mCplex->costPrimal(ptrRForn_no[k][r]->getCusto());
			col += mCplex->constraintsPrimal1[k](1);
			for (int i = 0; i < numCommod; i++) {
				if (matrizA_no[k][r][i + 1] > 0) {
					col += mCplex->constraintsPrimal3[i](matrizA_no[k][r][i + 1]);
					col += mCplex->constraintsPrimal5[k * numCommod + i](matrizA_no[k][r][i + 1]);
					col += mCplex->constraintsPrimal6[k * numCommod + i](-matrizA_no[k][r][i + 1]);

					//insere a coluna tambem na restricao lambda da arvore do primeiro branching
					if (matrizIndices[k][i + 1] >= 0) {
						col	+= mCplex->constraintsArvoreLambda[matrizIndices[k][i + 1]](matrizA_no[k][r][i + 1]);
					}
				}
			}
			mCplex->lambdaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
			col.end();
		}

		//herda do pai as rotas dos consumidores do veiculo k
		for (int r = 0; r < qRConsNo[k]; ++r) {
			rota = new short int[numCommod + 1];
			for (int i = 1; i <= numCommod; ++i) {
				rota[i] = pai->matrizB_no[k][r][i];
			}
			matrizB_no[k].push_back(rota);
			ptrRCons_no[k].push_back(pai->ptrRCons_no[k][r]);
			ptrRCons_no[k][r]->incrNumApontadores();

			//cria a variavel de decisao associada a esta rota e a insere no modelo
			IloNumColumn col = mCplex->costPrimal(ptrRCons_no[k][r]->getCusto());
			col += mCplex->constraintsPrimal2[k](1);
			for (int i = 0; i < numCommod; i++) {
				if (matrizB_no[k][r][i + 1] > 0) {
					col += mCplex->constraintsPrimal4[i](matrizB_no[k][r][i + 1]);
					col += mCplex->constraintsPrimal5[k * numCommod + i](-matrizB_no[k][r][i + 1]);
					col += mCplex->constraintsPrimal6[k * numCommod + i](matrizB_no[k][r][i + 1]);

					//insere a coluna tambem na restricao gamma da arvore do primeiro branching
					if (matrizIndices[k][i + 1] >= 0) {
						col	+= mCplex->constraintsArvoreGamma[matrizIndices[k][i + 1]](matrizB_no[k][r][i + 1]);
					}
				}
			}
			mCplex->gammaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
			col.end();
		}

		//herda os arcos fixos do veiculo k
		for (int i = 0; i < pai->arcosBranching[k].size(); i++) {
			tmp = new short int[3];
			tmp[0] = pai->arcosBranching[k][i][0];
			tmp[1] = pai->arcosBranching[k][i][1];
			tmp[2] = pai->arcosBranching[k][i][2];
			arcosBranching[k].push_back(tmp);
		}
	}

	//insere o arco calculado pelo pai para ser fixado
	tmp = new short int[3];
	tmp[0] = pai->iFixar;
	tmp[1] = pai->jFixar;
	tmp[2] = valorFixarArco;
	arcosBranching[pai->kFixar].push_back(tmp);

	//Insere todas as restricoes copiadas para o modelo
	setRestricoesArcos();

	//otimiza o modelo para que seja possivel obter os valores das variaveis
	mCplex->cplexPrimal.solve();

	//desaloca a matriz de indices das variavies fixadas nas restricoes do modelo
	for (int k = 0; k < maxV; ++k) {
		delete[] matrizIndices[k];
	}
	delete[] matrizIndices;
}

bool NoArvoreArcos::executaBranchingArcos(ModeloCplex* ptrMod, Grafo* g, char** varComRestr, int veic, int commod, char opCam){
	bool bestPrimal = false;
	mCplex = ptrMod;
	maxV = veic;
	numCommod = commod;
	totalNosCriados = 0;
	variaveisComRestricao = varComRestr;
	ptrNoArcos pai = new NoArvoreArcos();
	ptrNoArcos aux, listaNosFixados = NULL;

	while (pai != NULL) {
		//inicializa o primeiro filho, fixa o arco em 0 e chega na raiz
		mCplex->limpaVariaveisArvoreB2();
		aux = new NoArvoreArcos(pai, 0);
		if (aux->alcancaRaizNoSegundoBranching(g, opCam)) {
			aux->limiteDual = mCplex->cplexPrimal.getObjValue();
			if (aux->limiteDual < ModeloCplex::limitePrimal) {
				if (aux->procuraArcoMaisViolado() > 0) {
					aux->prox = listaNosFixados;
					listaNosFixados = aux;
				} else { //solucao inteira e funcao objetivo menor que o  limite primal
					ModeloCplex::limitePrimal = aux->limiteDual;
					bestPrimal = true;
					delete aux;
				}
			} else {
				delete aux;
			}
		} else {
			delete aux;
		}

		//inicializa o segundo filho, fixa o arco em 1 e chega na raiz
		mCplex->limpaVariaveisArvoreB2();
		aux = new NoArvoreArcos(pai, 1);
		if (aux->alcancaRaizNoSegundoBranching(g, opCam)) {
			aux->limiteDual = mCplex->cplexPrimal.getObjValue();
			if (aux->limiteDual < ModeloCplex::limitePrimal) {
				if (aux->procuraArcoMaisViolado() > 0) {
					aux->prox = listaNosFixados;
					listaNosFixados = aux;
				} else { //solucao inteira e funcao objetivo menor que o  limite primal
					ModeloCplex::limitePrimal = aux->limiteDual;
					bestPrimal = true;
					delete aux;
				}
			} else {
				delete aux;
			}
		} else {
			delete aux;
		}

		delete pai;
		pai = listaNosFixados;
		listaNosFixados = (pai != NULL) ? pai->prox : NULL;
	}
	return bestPrimal;
}

void NoArvoreArcos::setRestricoesArcos() {
	vector<Rota*>* rotasForn = mCplex->ptrRotasForn;
	vector<Rota*>* rotasCons = mCplex->ptrRotasCons;
	int verticeFixoI, verticeFixoJ, valorFixo, quantRotas, quantVertices, coefColuna, numVisitasArco;
	int numRestrArcos = 0;
	vector<int> verticesRota;

	for (int k = 0; k < maxV; ++k) {

		for (int i = 0; i < arcosBranching[k].size(); ++i) {
			IloExpr exp = IloExpr(mCplex->env);

			verticeFixoI = arcosBranching[k][i][0];
			verticeFixoJ = arcosBranching[k][i][1];
			valorFixo = arcosBranching[k][i][2];

			if (verticeFixoJ <= numCommod) {

				quantRotas = rotasForn[k].size();
				for (int r = 0; r < quantRotas; ++r) {
					verticesRota = rotasForn[k][r]->getVertices();
					quantVertices = verticesRota.size()-2;
					numVisitasArco = 0;
					for (int i = 0; i < quantVertices; ++i) {
						if ((verticesRota[i] == verticeFixoI) && (verticesRota[i+1] == verticeFixoJ)) {
							++numVisitasArco;
						}
					}
					if (numVisitasArco > 0){
						exp += numVisitasArco * mCplex->lambdaPrimal[k][r];
					}
				}
				quantRotas = ptrRForn_no[k].size();
				for (int r = 0; r < quantRotas; ++r) {
					verticesRota = ptrRForn_no[k][r]->getVertices();
					quantVertices = verticesRota.size()-2;
					numVisitasArco = 0;
					for (int i = 0; i < quantVertices; ++i) {
						if ((verticesRota[i] == verticeFixoI) && (verticesRota[i+1] == verticeFixoJ)) {
							++numVisitasArco;
						}
					}
					if (numVisitasArco > 0){
						exp += numVisitasArco * mCplex->lambdaNo[k][r];
					}
				}

			} else {

				quantRotas = rotasCons[k].size();
				for (int r = 0; r < quantRotas; ++r) {
					verticesRota = rotasCons[k][r]->getVertices();
					quantVertices = verticesRota.size()-2;
					numVisitasArco = 0;
					for (int i = 0; i < quantVertices; ++i) {
						if ((verticesRota[i] == verticeFixoI) && (verticesRota[i+1] == verticeFixoJ)) {
							++numVisitasArco;
						}
					}
					if (numVisitasArco > 0){
						exp += numVisitasArco * mCplex->gammaPrimal[k][r];
					}
				}
				quantRotas = ptrRCons_no[k].size();
				for (int r = 0; r < quantRotas; ++r) {
					verticesRota = ptrRCons_no[k][r]->getVertices();
					quantVertices = verticesRota.size()-2;
					numVisitasArco = 0;
					for (int i = 0; i < quantVertices; ++i) {
						if ((verticesRota[i] == verticeFixoI) && (verticesRota[i+1] == verticeFixoJ)) {
							++numVisitasArco;
						}
					}
					if (numVisitasArco > 0){
						exp += numVisitasArco * mCplex->gammaNo[k][r];
					}
				}

			}

			if (valorFixo == 1) {
				mCplex->constraintsArvoreArcos.add(exp == 1);
				coefColuna = 1;
			} else {
				mCplex->constraintsArvoreArcos.add(exp == 0);
				coefColuna = -1;
			}

			mCplex->modelPrimal.add(
			mCplex->constraintsArvoreArcos[numRestrArcos]);
			exp.end();

			//insere a variavel artificial associada a esta restricao
			IloNumColumn col = mCplex->costPrimal(ModeloCplex::limitePrimal);
			col += mCplex->constraintsArvoreArcos[numRestrArcos](coefColuna);
			mCplex->artificiais.add(IloNumVar(col, 0, +IloInfinity, ILOFLOAT));
			col.end();
			++numRestrArcos;
		}
	}
}

float NoArvoreArcos::procuraArcoMaisViolado() {
	IloNumArray lambdaValues(mCplex->env);
	IloNumArray gammaValues(mCplex->env);
	vector<Rota*>* rotasForn = mCplex->ptrRotasForn;
	vector<Rota*>* rotasCons = mCplex->ptrRotasCons;
	int tamRota, totalVertices = 2 * numCommod + 2;
	vector<int> verticesRota;
	float maiorViolacao = 0;

	//Cada celula da matriz representara um arco (i,j) que sera preenchido em funcao da violacao
	//Aquele arco (i,j) que tiver o maior valor de violacao sera fixado no branching de variaveis
	float** matrizViolacao = new float*[totalVertices];
	for (int i = 0; i < totalVertices; ++i) {
		matrizViolacao[i] = new float[totalVertices];
		memset(matrizViolacao[i], 0, (totalVertices) * sizeof(float));
	}

	for (int k = 0; k < maxV; ++k) {

		//antes de procurar por arcos violados, deve-se assegurar que um arco que ja tenha sido fixado
		//em iteracoes anteriores nao seja fixado novamente. Isto eh necessario pois nas restricoes que
		//fixam o arco em 1, pode ser que continuem existindo variaveis fracionarias
		for (int i = 0; i < totalVertices; ++i) {
			memset(matrizViolacao[i], 0, (totalVertices) * sizeof(float));
		}
		for (int i = 0; i < arcosBranching[k].size(); ++i) {
			matrizViolacao[arcosBranching[k][i][0]][arcosBranching[k][i][1]] = -numCommod;
		}

		//primeiro busca por variaveis fracionarias entre as variaveis de rotas de fornecedores obtidas na raiz
		mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambdaPrimal[k]);
		for (int r = 0; r < mCplex->qRotasForn[k]; ++r) {
			if ((lambdaValues[r] > 0) && (lambdaValues[r] < 1)) {
				verticesRota = rotasForn[k][r]->getVertices();
				tamRota = verticesRota.size() - 2;
				for (int i = 0; i < tamRota; ++i) {
					matrizViolacao[verticesRota[i]][verticesRota[i + 1]] += lambdaValues[r];
					if (matrizViolacao[verticesRota[i]][verticesRota[i + 1]] > maiorViolacao) {
						maiorViolacao = matrizViolacao[verticesRota[i]][verticesRota[i + 1]];
						iFixar = verticesRota[i];
						jFixar = verticesRota[i + 1];
						kFixar = k;
					}
				}
			}
		}

		if (qRFornNo[k] > 0) {
			mCplex->cplexPrimal.getValues(lambdaValues, mCplex->lambdaNo[k]);
			for (int r = 0; r < qRFornNo[k]; ++r) {
				if ((lambdaValues[r] > 0) && (lambdaValues[r] < 1)) {
					verticesRota = ptrRForn_no[k][r]->getVertices();
					tamRota = verticesRota.size() - 2;
					for (int i = 0; i < tamRota; ++i) {
						matrizViolacao[verticesRota[i]][verticesRota[i + 1]] += lambdaValues[r];
						if (matrizViolacao[verticesRota[i]][verticesRota[i + 1]] > maiorViolacao) {
							maiorViolacao = matrizViolacao[verticesRota[i]][verticesRota[i + 1]];
							iFixar = verticesRota[i];
							jFixar = verticesRota[i + 1];
							kFixar = k;
						}
					}
				}
			}
		}

		//busca por variaveis fracionarias entre as variaveis de rotas de consumidores obtidas na raiz
		mCplex->cplexPrimal.getValues(gammaValues, mCplex->gammaPrimal[k]);
		for (int r = 0; r < mCplex->qRotasCons[k]; ++r) {
			if ((gammaValues[r] > 0) && (gammaValues[r] < 1)) {
				verticesRota = rotasCons[k][r]->getVertices();
				tamRota = verticesRota.size() - 2;
				for (int i = 0; i < tamRota; ++i) {
					matrizViolacao[verticesRota[i]][verticesRota[i + 1]] += gammaValues[r];
					if (matrizViolacao[verticesRota[i]][verticesRota[i + 1]] > maiorViolacao) {
						maiorViolacao = matrizViolacao[verticesRota[i]][verticesRota[i + 1]];
						iFixar = verticesRota[i];
						jFixar = verticesRota[i + 1];
						kFixar = k;
					}
				}
			}
		}

		if (qRConsNo[k] > 0) {
			mCplex->cplexPrimal.getValues(gammaValues, mCplex->gammaNo[k]);
			for (int r = 0; r < qRConsNo[k]; ++r) {
				if ((gammaValues[r] > 0) && (gammaValues[r] < 1)) {
					verticesRota = ptrRCons_no[k][r]->getVertices();
					tamRota = verticesRota.size() - 2;
					for (int i = 0; i < tamRota; ++i) {
						matrizViolacao[verticesRota[i]][verticesRota[i + 1]] += gammaValues[r];
						if (matrizViolacao[verticesRota[i]][verticesRota[i + 1]] > maiorViolacao) {
							maiorViolacao = matrizViolacao[verticesRota[i]][verticesRota[i + 1]];
							iFixar = verticesRota[i];
							jFixar = verticesRota[i + 1];
							kFixar = k;
						}
					}
				}
			}
		}
	}

	//libera memoria
	for (int i = 0; i < totalVertices; i++) {
		delete[] matrizViolacao[i];
	}
	delete[] matrizViolacao;
	lambdaValues.end();
	gammaValues.end();

	return maiorViolacao;
}

bool NoArvoreArcos::alcancaRaizNoSegundoBranching(Grafo* G, char opCaminho) {
	Rota** colunas;
	bool alcancouRaiz;
	int k, limVeic, iVeicForn = 0, iVeicCons = 0, temp;

	do{
		alcancouRaiz = true;
		limVeic = iVeicForn + maxV;
		for (int k = iVeicForn; k < limVeic; ++k, ++iVeicForn){
			atualizaCustosDuaisForn(G, (k % maxV));
//			colunas = retornaColuna(G, true, mCplex->cplexPrimal.getDual(mCplex->constraintsPrimal1[(k % maxV)]), opCaminho, temp);
			if (colunas != NULL){
				for (int i = 0; colunas[i] != NULL; ++i){
					inserirColunaForn(colunas[i], (k % maxV));
				}
				alcancouRaiz = false;
				delete [] colunas;
				++iVeicForn;
				mCplex->cplexPrimal.solve();
				break;
			}
		}

		limVeic = iVeicCons + maxV;
		for (int k = iVeicCons; k < limVeic; ++k, ++iVeicCons){
			atualizaCustosDuaisCons(G, (k % maxV));
//			colunas = retornaColuna(G, false, mCplex->cplexPrimal.getDual(mCplex->constraintsPrimal2[(k % maxV)]), opCaminho, temp);
			if (colunas != NULL){
				for (int i = 0; colunas[i] != NULL; ++i){
					inserirColunaCons(colunas[i], (k % maxV));
				}
				alcancouRaiz = false;
				delete [] colunas;
				++iVeicCons;
				mCplex->cplexPrimal.solve();
				break;
			}
		}
	}while(!alcancouRaiz);

	IloNumArray valoresVarArtif(mCplex->env);
	mCplex->cplexPrimal.getValues(valoresVarArtif, mCplex->artificiais);
	int numVarArtificiais = valoresVarArtif.getSize();
	for (int i = 0; i < numVarArtificiais; ++i) {
		if (valoresVarArtif[i] >= 0.00001){
			return false;
		}
	}
	return true;
}

void NoArvoreArcos::atualizaCustosDuaisForn(Grafo* G, int k){
	float custoDualVertice;
	int saltoArvoreArcos = 0;
	int saltoK = k * numCommod;
	IloNumArray thetaValues(mCplex->env), piValues(mCplex->env), xiValues(mCplex->env), arvoreValues(mCplex->env);
	mCplex->cplexPrimal.getDuals(thetaValues, mCplex->constraintsPrimal3);
	mCplex->cplexPrimal.getDuals(piValues, mCplex->constraintsPrimal5);
	mCplex->cplexPrimal.getDuals(xiValues, mCplex->constraintsPrimal6);
	mCplex->cplexPrimal.getDuals(arvoreValues, mCplex->constraintsArvoreLambda);
	
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
		saltoArvoreArcos += arcosBranching[v].size(); //verifica quantas restricoes de arcos existem antes de k
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
	G->setCustoArestasDual('F');
	
	//insere no grafo os custos duais associados aos arcos que foram fixados
	for (int i = 0; i < arcosBranching[k].size(); ++i){
		if (arcosBranching[k][i][1] <= numCommod){
				G->setCustoArestaDual(arcosBranching[k][i][0], arcosBranching[k][i][1], 
											-mCplex->cplexPrimal.getDual(mCplex->constraintsArvoreArcos[saltoArvoreArcos+i]));
		}
	}

	thetaValues.end(); piValues.end(); xiValues.end(); arvoreValues.end();
	delete [] vetorIndices;
}

void NoArvoreArcos::atualizaCustosDuaisCons(Grafo* G, int k){
	float custoDualVertice;
	int saltoArvoreArcos = 0;
	int saltoK = k * numCommod;
	IloNumArray muValues(mCplex->env), piValues(mCplex->env), xiValues(mCplex->env), arvoreValues(mCplex->env);
	mCplex->cplexPrimal.getDuals(muValues, mCplex->constraintsPrimal4);
	mCplex->cplexPrimal.getDuals(piValues, mCplex->constraintsPrimal5);
	mCplex->cplexPrimal.getDuals(xiValues, mCplex->constraintsPrimal6);
	mCplex->cplexPrimal.getDuals(arvoreValues, mCplex->constraintsArvoreGamma);

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
		saltoArvoreArcos += arcosBranching[v].size(); //verifica quantas restricoes de arcos existem antes de k
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
	G->setCustoArestasDual('C');

	//insere no grafo os custos duais associados aos arcos que foram fixados para o veiculo k
	for (int i = 0; i < arcosBranching[k].size(); ++i){
		if (arcosBranching[k][i][1] > numCommod){
			G->setCustoArestaDual(arcosBranching[k][i][0], arcosBranching[k][i][1], 
											-mCplex->cplexPrimal.getDual(mCplex->constraintsArvoreArcos[saltoArvoreArcos+i]));
		}
	}

	muValues.end(); piValues.end(); xiValues.end(); arvoreValues.end();
	delete [] vetorIndices;
}

void NoArvoreArcos::inserirColunaForn(Rota* r, int k){
	//CALCULO DOS INDICES PARA INCLUIR O CUSTO DUAL DAS RESTRICOES DA ARVORE NO VERTICE
	int *vetorIndices = new int[numCommod+1];
	int numRestrLambda = 0;
	int saltoArvoreArcos = 0;
	//verifica quantas restricoes de lambda na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v){
		for (int j = 1; j <= numCommod; ++j){
			if (variaveisComRestricao[v][j] > 0){
				++numRestrLambda;
			} 
		}
		saltoArvoreArcos += arcosBranching[v].size();
	}
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
	int numVertAtualizar = vertRota.size()-2;
	for(int j = 1; j <= numVertAtualizar; ++j){
		++matrizA_no[k][qRFornNo[k]][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//Cria a variavel de decisao e insere a coluna no modelo
	IloNumColumn col = mCplex->costPrimal(r->getCusto());
	col += mCplex->constraintsPrimal1[k](1);
	for (int i = 1; i <= numCommod; i++){
		if (matrizA_no[k][qRFornNo[k]][i] > 0){
			col += mCplex->constraintsPrimal3[i-1](matrizA_no[k][qRFornNo[k]][i]);
			col += mCplex->constraintsPrimal5[k*numCommod + i-1](matrizA_no[k][qRFornNo[k]][i]);
			col += mCplex->constraintsPrimal6[k*numCommod + i-1](-matrizA_no[k][qRFornNo[k]][i]);
			
			/*VERIFICA SE PRECISA INSERIR COEFICIENTE NA COLUNA PARA AS RESTRICOES DA ARVORE*/
			if (variaveisComRestricao[k][i] > 0){
				//vincula a coluna a restricao da arvore (a posicao correta foi calculada anteriormente por eficiencia)
				col += mCplex->constraintsArvoreLambda[vetorIndices[i]](matrizA_no[k][qRFornNo[k]][i]);			
			}			
		}
	}
	
	//Inclui na coluna os indices associados as restricoes de branching em arcos, caso existam
	int numVisitasArco;
	for (int i = 0; i < arcosBranching[k].size(); ++i){
		numVisitasArco = 0;
		for (int j = 0; j < numVertAtualizar; ++j){
			if ((vertRota[j] == arcosBranching[k][i][0]) && ((vertRota[j+1] == arcosBranching[k][i][1]))){
				++numVisitasArco;
			}
		}
		if (numVisitasArco > 0){
			col += mCplex->constraintsArvoreArcos[saltoArvoreArcos + i](numVisitasArco);
		}
	}
	mCplex->lambdaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRFornNo[k];
	delete [] vetorIndices;
}

void NoArvoreArcos::inserirColunaCons(Rota* r, int k){
	//CALCULO DOS INDICES PARA INCLUIR O CUSTO DUAL DAS RESTRICOES DA ARVORE NO VERTICE
	int *vetorIndices = new int[numCommod+1];
	int numRestrGamma = 0;
	int saltoArvoreArcos = 0;

	//verifica quantas restricoes de gamma na arvore existem para veiculos com indices inferiores a k
	for (int v = 0; v < k; ++v){
		for (int j = 1; j <= numCommod; ++j){
			if (variaveisComRestricao[v][j] > 0){
				++numRestrGamma;
			} 
		}
		saltoArvoreArcos += arcosBranching[v].size();
	}
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
	IloNumColumn col = mCplex->costPrimal(r->getCusto());
	col += mCplex->constraintsPrimal2[k](1);
	for (int i = 1; i <= numCommod; i++){
		if (matrizB_no[k][qRConsNo[k]][i] > 0){
			col += mCplex->constraintsPrimal4[i-1](matrizB_no[k][qRConsNo[k]][i]);
			col += mCplex->constraintsPrimal5[k*numCommod + i-1](-matrizB_no[k][qRConsNo[k]][i]);
			col += mCplex->constraintsPrimal6[k*numCommod + i-1](matrizB_no[k][qRConsNo[k]][i]);
			
			/*VERIFICA SE PRECISA INSERIR COEFICIENTE NA COLUNA PARA AS RESTRICOES DA ARVORE*/
			if (variaveisComRestricao[k][i] > 0){
				//vincula a coluna a restricao da arvore na respectiva posicao (calculada no inicio do metodo)
				col += mCplex->constraintsArvoreGamma[vetorIndices[i]](matrizB_no[k][qRConsNo[k]][i]);
			}
		}
	}

	//Inclui na coluna os indices associados as restricoes de branching em arcos, caso existam
	int numVisitasArco;
	for (int i = 0; i < arcosBranching[k].size(); ++i){
		numVisitasArco = 0;
		for (int j = 0; j < numVertAtualizar; ++j){
			if ((vertRota[j] == arcosBranching[k][i][0]) && ((vertRota[j+1] == arcosBranching[k][i][1]))){
				++numVisitasArco;
			}
		}
		if (numVisitasArco > 0){
			col += mCplex->constraintsArvoreArcos[saltoArvoreArcos + i](numVisitasArco);
		}
	}
	mCplex->gammaNo[k].add(IloNumVar(col, 0, 1, ILOFLOAT));
	col.end();

	++qRConsNo[k];
	delete [] vetorIndices;
}

void NoArvoreArcos::imprimeNo() {
	cout << "indice = " << indiceNo << endl;
	cout << "valor = " << limiteDual << endl;
	for (int k = 0; k < maxV; ++k) {
		cout << "qRFornNo[" << k << "] = " << qRFornNo[k] << endl;
		for (int r = 0; r < qRFornNo[k]; ++r) {
			cout << "  ";
			ptrRForn_no[k][r]->imprimir();
		}
	}
	for (int k = 0; k < maxV; ++k) {
		cout << "qRConsNo[" << k << "] = " << qRConsNo[k] << endl;
		for (int r = 0; r < qRConsNo[k]; ++r) {
			cout << "  ";
			ptrRCons_no[k][r]->imprimir();
		}
	}
	cout << "Arcos com restricao:" << endl;
	for (int k = 0; k < maxV; ++k) {
		cout << "k = " << k << endl;
		for (int i = 0; i < arcosBranching[k].size(); ++i) {
			cout << "  (" << arcosBranching[k][i][0] << ","
					<< arcosBranching[k][i][1] << ") = "
					<< arcosBranching[k][i][2] << endl;
		}
	}
	cout << endl;
}
