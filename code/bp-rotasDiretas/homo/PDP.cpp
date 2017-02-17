#include "PDP.h"

PDP::PDP(Grafo* g, float valorSub) : valorSubtrair(valorSub), menorLabelObtido(NULL), menorCustoObtido(MAIS_INFINITO){
	lixo = NULL;
	numRequests = g->getNumReqs();

	//Aloca memoria para vetorLabels (que tera os mesmos labels de listaLabels, mas será utilizado por questoes de eficiencia)
	vetorLabels = new vLabelRD[2*numRequests+1];

	//Aloca e inicializa o vetor que armazenara a codificacao dos vertices no sistema unsigned long long int
	verticesCoding = new unsigned long long int [numRequests+1];
	for (int i = 0; i <= numRequests; ++i) verticesCoding[i] = pow (2, i);
}


PDP::~PDP(){
	ptrLabelRD p, aux;
	//Exclui a memoria ainda alocada
	 //vetorLabels[0] <=> listaLabels (aponta para todos os labels, de todos os vertices)
	for (int i = 1; i <= 2*numRequests; ++i)
	{
		p = vetorLabels[i].cabeca;		
		while (p != NULL)
		{
			aux = p;
			p = p->prox;	
			delete aux;
		}
	}
	delete [] vetorLabels;
	delete [] verticesCoding;

	//apaga todos os labels que foram para o lixo (nao foram apagados pois podia haver label apontando para eles)
	p = lixo;
	while (p != NULL)
	{
		aux = p;
		p = p->prox;	
		delete aux;
	}
}


int PDP::calculaCaminhoElementar(Grafo* g, int tempo){
	bool extendable;
	int cargaMaxima = g->getCapacVeiculo();
	int depositoArtificial = g->getNumVertices()-1;
	int carga, numVisitados1, numVisitados2, menorIndice;
	unsigned long long int conjVisitados1, conjVisitados2;
	float custo, custoAteDestino;
	ptrLabelRD aux, it;

	//insiro os labels relativos ao caminho de 0 aos vertices adjacentes para depois processar 
	//os demais labels existentes em vetorLabels, armazenando apenas aqueles que chegam em 0' nao-dominados
	for (int i = 1; i <= numRequests; ++i)
	{
		carga = g->getCargaVertice(i);
		aux = new LabelRD(verticesCoding[i], verticesCoding[i], 1, 1, i, 0, carga, g->getCustoArestaDual(0, i), NULL);
		aux->pai = NULL;
		vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;
		vetorLabels[numRequests+i].cabeca = vetorLabels[numRequests+i].calda = vetorLabels[numRequests+i].posAtual = NULL;
	}

	//obtem todos os labels nao-dominados
	menorIndice = 1;
	while(menorIndice > 0)
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;
		
		for (int i = 1; i < depositoArtificial; ++i)
		{
			if ( ( i <= numRequests ) && ( ( it->vertVisitados2 & verticesCoding[i] ) == 0 ) )
			{
				extendable = true;
				carga = it->cargaAcumulada + g->getCargaVertice(i);
			}
			else if ( ( i > numRequests ) && ( ( it->vertVisitados1 & verticesCoding[i-numRequests] ) > 0 ) )
			{
				extendable = true;
				carga = it->cargaAcumulada - g->getCargaVertice(i);
			}
			else extendable = false;


			if ( ( extendable == true ) && ( carga <= cargaMaxima ) )
			{
				custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice, i);
				if ( i <= numRequests )
				{
					conjVisitados1 = (it->vertVisitados1 | verticesCoding[i]);
					conjVisitados2 = (it->vertVisitados2 | verticesCoding[i]);
					numVisitados1 = it->numVertVisitados1+1;
					numVisitados2 = it->numVertVisitados2+1;
				}
				else
				{
					conjVisitados1 = (it->vertVisitados1 & (~verticesCoding[i-numRequests]));
					conjVisitados2 = it->vertVisitados2;
					numVisitados1 = it->numVertVisitados1-1;
					numVisitados2 = it->numVertVisitados2;
				}

				//Se os valores forem nao dominados com relacao aos que estao inseridos no vertice, entao insere o novo label
				if ( !verificaLabelDominadoEDomina( g, conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, i, carga, custo ) )
				{
					aux = new LabelRD(conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, i, it->ultimoVertice, carga, custo, NULL);
					aux->pai = it;
					if ( vetorLabels[i].posAtual == NULL ) vetorLabels[i].posAtual = aux;
					if ( vetorLabels[i].cabeca == NULL ) vetorLabels[i].cabeca = aux;
					if ( vetorLabels[i].calda == NULL ) vetorLabels[i].calda = aux;
					else
					{
						vetorLabels[i].calda->prox = aux;
						vetorLabels[i].calda = aux;
					}

					if ( conjVisitados1 == 0 ) //Nao existem requisicoes abertas para este label, portanto, pode ir para o deposito
					{
						custoAteDestino = custo + g->getCustoArestaDual(i, depositoArtificial);
						if ( custoAteDestino < menorCustoObtido )
						{
							menorCustoObtido = custoAteDestino;
							menorLabelObtido = aux;
						}
					}
				}
			}

			if ( ( vetorLabels[i].posAtual != NULL ) && ( vetorLabels[i].posAtual->custoDual < menorCustoAtual ) )
			{
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}
		if (( time(0) - tempo ) > 14400 ) return -1;
	}

	if ( menorCustoObtido < ( valorSubtrair - 0.01 ) ) return 1;
	else return -1;
}


bool PDP::verificaLabelDominadoEDomina(Grafo* g, unsigned long long int visit1, unsigned long long int visit2, int numVisit1, int numVisit2, int ult, int carga, float custo ){
	bool itUpdate;
	int domCusto, domCarga;
	ptrLabelRD aux, anterior = NULL; //necessario na exclusao do label apontado por 'it' (caso seja dominado)
	ptrLabelRD it = vetorLabels[ult].cabeca;

	while (it != NULL) //Verifica a dominancia para todos os labels (o novo label so sera inserido se nao houver nenhum q o domina - ou seja igual)
	{
		itUpdate = true;
		//Verifico se o custo deste novo label domina o label armazenado em vLabel[ult]
		if ( custo < it->custoDual ) domCusto = 1;
		else if ( custo > it->custoDual ) domCusto = -1;
		else domCusto = 0;

		if ( carga < it->cargaAcumulada ) domCarga = 1;
		else if ( carga > it->cargaAcumulada ) domCarga = -1;
		else domCarga = 0;

		int a = 0, b = 0, c = 0, d = 0;
		for ( int j = 1; j <= numRequests; ++j )
 		{
			if ( ( visit1 & verticesCoding[j] ) < ( it->vertVisitados1 & verticesCoding[j] ) ) a = 1;
			else if ( ( visit1 & verticesCoding[j] ) > ( it->vertVisitados1 & verticesCoding[j] ) ) b = 1;

			if ( ( visit2 & verticesCoding[j] ) < ( it->vertVisitados2 & verticesCoding[j] ) ) c = 1;
			else if ( ( visit2 & verticesCoding[j] ) > ( it->vertVisitados2 & verticesCoding[j] ) ) d = 1;
		}

		if ( ( domCusto <= 0 ) && ( domCarga <= 0 ) && ( a == 0 ) && ( c == 0 ) ) return true;
		else if ( ( domCusto >= 0 ) && ( domCarga >= 0 ) && ( b == 0 ) && ( d == 0 ) ) //Novo Label DOMINA
		{
			if ( vetorLabels[ult].posAtual == it ) vetorLabels[ult].posAtual = it->prox;
			if ( vetorLabels[ult].cabeca == it ) vetorLabels[ult].cabeca = it->prox;
			if ( vetorLabels[ult].calda == it ) vetorLabels[ult].calda = anterior;
			if ( anterior != NULL ) anterior->prox = it->prox;
			aux = it;
			it = it->prox;
			itUpdate = false;
			//delete aux;
		
			aux->prox = lixo;
			lixo = aux;
		}

		//Atualiza os valores de it e anterior para continuar a verificacao em outros labels de vLabels[ult]
		if (itUpdate)
		{
			anterior = it;
			it = it->prox;
		}
	}
	return false;
}

Rota** PDP::getRotaCustoMinimo(Grafo* g, float percentual){
	float limiteAceitacao, custoR;
	int depositoArtif = g->getNumVertices()-1;

	//utiliza o percentual para obter rotas negativas (podem existir varias, pois nao parou na primeira)
	if (menorCustoObtido < 0) limiteAceitacao = percentual * menorCustoObtido;
	else limiteAceitacao = (2 - percentual) * menorCustoObtido;
	if (limiteAceitacao > (valorSubtrair - 0.01)) limiteAceitacao = valorSubtrair - 0.01;
	
	int numRotasRetornadas = 0;
	//Neste momento, verifica todos os labels nao dominados encontrados para alocar a quantidade de rotas a serem retornadas
	ptrLabelRD tmp, aux, p;
	for (int i = numRequests+1; i < depositoArtif; ++i)
	{
		aux = vetorLabels[i].cabeca;
		while (aux != NULL)
		{
			if ( ( aux->vertVisitados1 == 0 ) && ( ( aux->custoDual + g->getCustoArestaDual(i, depositoArtif)) < limiteAceitacao ) ) ++numRotasRetornadas;
			aux = aux->prox;
		}
	}

	if (numRotasRetornadas > 0)
	{
		Rota** r;
		r = new Rota*[numRotasRetornadas+1];
		numRotasRetornadas = 0;
		for (int i = numRequests+1; i < depositoArtif; ++i)
		{
			aux = vetorLabels[i].cabeca;
			while (aux != NULL)
			{
				custoR = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice, depositoArtif);
				if ( ( aux->vertVisitados1 == 0 ) && ( custoR < limiteAceitacao ) )
				{
					r[numRotasRetornadas] = new Rota();
					r[numRotasRetornadas]->inserirVerticeFim(depositoArtif);
					tmp = aux;
					while (tmp != NULL)
					{
						r[numRotasRetornadas]->inserirVerticeInicio(tmp->ultimoVertice);
						tmp = tmp->pai;
					}
					r[numRotasRetornadas]->inserirVerticeInicio(0);
					r[numRotasRetornadas]->setCustoReduzido(custoR);
					r[numRotasRetornadas]->setCustoRota(g, true);
					++numRotasRetornadas;
				}
				aux = aux->prox;
			}
		}		
		r[numRotasRetornadas] = NULL;
		return r;
	}
	else
	{
		return NULL;
	}
}
