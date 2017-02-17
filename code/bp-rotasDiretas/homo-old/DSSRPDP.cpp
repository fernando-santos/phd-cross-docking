#include "DSSRPDP.h"

DSSRPDP::DSSRPDP(Grafo* g, float valorSub) : verticesFixarCoding(0), valorSubtrair(valorSub)
{
	nRequests = g->getNumReqsDual();
	nVertices = 2*nRequests;
	vetorLabels = new vLabelDSSR[nVertices+1];
	indiceVertDual = g->getIndiceVerticesDual();

	//Aloca e inicializa o vetor que armazenara a codificacao dos vertices no sistema unsigned long long int
	verticesCoding = new unsigned long long int [nRequests+1];
	for (int i = 1; i <= nRequests; ++i) verticesCoding[i] = pow (2, i);
	verticesCoding[0] = 0;
}

DSSRPDP::~DSSRPDP()
{
	ptrLabelDSSR p, aux;
	for ( int i = 1; i <= nVertices; ++i )
	{
		p = vetorLabels[i].cabeca;
		while ( p != NULL )
		{
			aux = p;
			p = p->prox;	
			delete aux;
		}
	}
	delete [] vetorLabels;
	delete [] verticesCoding;
}


int DSSRPDP::calculaCaminhoPDP(Grafo* g)
{
	int violado; int x = 0;
	do{
		encontrouRotaNegativa = false;
		violado = calculaCaminhos( g );
		if ( !encontrouRotaNegativa ) return -1;

		if ( violado ) //prepara as variaveis para uma proxima iteracao
		{
			//insere '1' na posicao correspondente ao vertice violado em verticesFixarCoding
			verticesFixarCoding = (verticesFixarCoding | verticesCoding[violado]);

			//libera labels obtidos da iteracao anterior
			ptrLabelDSSR p, aux;
			for ( int i = 1; i <= nVertices; ++i )
			{
				p = vetorLabels[i].cabeca;	
				while ( p != NULL )
				{
					aux = p;
					p = p->prox;	
					delete aux;
				}
			}
		}
	}while( violado );

	//caso alcance este ponto, tem-se uma solucao elementar de custo reduzido negativo
	//(garante-se que o label de menor custo do ESPPRC eh elementar, mas nao necessariamente todos obtidos)
	return 1;
}


int DSSRPDP::calculaCaminhos( Grafo* g )
{
	menorCustoObtido = MAIS_INFINITO;
	int depositoArtificial = nVertices+1;
	int cargaMaxima = g->getCapacVeiculo();
	int carga, numVisitados, numVisitadosU, menorIndice=1;
	unsigned long long int conjVisitados, conjVisitadosU;
	float custo, custoAteDestino;
	ptrLabelDSSR aux, it;

	//insiro os labels relativos ao caminho de 0 aos fornecedores adjacentes
	for (int i = 1; i <= nRequests; ++i)
	{
		carga = g->getCargaVertice( indiceVertDual[i] );
		aux = new LabelDSSR(verticesCoding[i], verticesCoding[i], 1, 1, i, carga, g->getCustoArestaDual(0, i), NULL, NULL);
		vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;
		vetorLabels[nRequests+i].cabeca = vetorLabels[nRequests+i].calda = vetorLabels[nRequests+i].posAtual = NULL;
	}

	//expande todos os labels associados a vertices fornecedores
	while(menorIndice > 0)
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		//Uma vez com o label 'it', verifica-se todos os adjacentes de 'it->ultimo' em busca 
		//de encontrar outros labels nao dominados para serem inseridos em vetorLabels[ultimoVertice]
		for (int i = 1; i <= nRequests; ++i)
		{
			if ( ( it->ultimoVertice != i ) && ( ( it->vertVisitadosU & ( verticesCoding[i] & verticesFixarCoding ) ) == 0 ) )
			{
				carga = it->cargaAcumulada + g->getCargaVertice( indiceVertDual[i] ); 
				if ( carga <= cargaMaxima )
				{
					conjVisitados = ( it->vertVisitados | verticesCoding[i] );
					numVisitados = it->numVisitados+1;
					conjVisitadosU = ( it->vertVisitadosU | verticesCoding[i] );
					numVisitadosU = it->numVisitadosU+1;
					custo = it->custoDual + g->getCustoArestaDual( it->ultimoVertice, i );

					//Antes de executar o teste da dominancia deste label, marca-se os vertices unreachable
					for ( int z = 1; z <= nRequests; ++z )
					{
						if ( ( ( conjVisitadosU & verticesCoding[z] ) == 0 )  && ( carga + g->getCargaVertice( indiceVertDual[z] ) > cargaMaxima ) )
						{
							conjVisitadosU = ( conjVisitadosU | verticesCoding[z] );
							++numVisitadosU;
						}
					}

					//Caso o label a ser criado seja nao-dominado, ele eh inserido no vetor de labels
					if ( !verificaLabelDominadoEDominaF(g, i, custo, carga, conjVisitados, conjVisitadosU) ) 
					{
						aux = new LabelDSSR(conjVisitados, conjVisitadosU, numVisitados, numVisitadosU, i, carga, custo, it, NULL);
						if ( vetorLabels[i].posAtual == NULL ) vetorLabels[i].posAtual = aux;
						if ( vetorLabels[i].cabeca == NULL ) vetorLabels[i].cabeca = aux;
						if ( vetorLabels[i].calda == NULL ) vetorLabels[i].calda = aux;
						else
						{
							vetorLabels[i].calda->prox = aux;
							vetorLabels[i].calda = aux;
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
	}

	//expande todos os labels nao-dominados de fornecedores para todos os possiveis consumidores
	for ( int i = 1; i <= nRequests; ++i )
	{
		it = vetorLabels[i].cabeca;
		while ( it != NULL )
		{
			for ( int j = nRequests+1; j <= nVertices; ++j )
			{
				if ( ( it->vertVisitados & verticesCoding[j-nRequests] ) > 0 )
				{
					custo = it->custoDual + g->getCustoArestaDual( it->ultimoVertice, j );
					conjVisitados = (it->vertVisitados & (~verticesCoding[j-nRequests]));
					numVisitados = it->numVisitados-1;

					//Caso o label a ser criado nao seja dominado, ele eh inserido no vetor de labels
					if ( !verificaLabelDominadoEDominaC(g, j, custo, numVisitados, conjVisitados) ) 
					{
						aux = new LabelDSSR(conjVisitados, 0, numVisitados, 0, j, 0, custo, it, NULL);
						if ( vetorLabels[j].posAtual == NULL ) vetorLabels[j].posAtual = aux;
						if ( vetorLabels[j].cabeca == NULL ) vetorLabels[j].cabeca = aux;
						if ( vetorLabels[j].calda == NULL ) vetorLabels[j].calda = aux;
						else
						{
							vetorLabels[j].calda->prox = aux;
							vetorLabels[j].calda = aux;
						}

						if ( conjVisitados == 0 ) //caso nao existam requisicoes 'abertas', verifica o custo do caminho
						{
							custoAteDestino = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice, depositoArtificial);
							if ( ( custoAteDestino < ( valorSubtrair - 0.01 ) ) && ( custoAteDestino < menorCustoObtido ) )
							{
								menorLabelObtido = aux;
								menorCustoObtido = custoAteDestino;
								encontrouRotaNegativa = true;
							}
						}

					}
				}
			}
			it = it->prox;
		}
	}

	//expande todos os labels associados a vertices consumidores
	menorIndice = nRequests+1;
	while( menorIndice > 0 )
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		for ( int i = nRequests+1; i <= nVertices; ++i )
		{
			if ( ( it->vertVisitados & verticesCoding[i-nRequests] ) > 0 ) //verifica se a requisicao esta 'aberta'
			{
				custo = it->custoDual + g->getCustoArestaDual( it->ultimoVertice, i );
				conjVisitados = (it->vertVisitados & (~verticesCoding[i-nRequests]));
				numVisitados = it->numVisitados-1;

				//Caso o label a ser criado nao seja dominado, ele eh inserido no vetor de labels
				if ( !verificaLabelDominadoEDominaC(g, i, custo, numVisitados, conjVisitados) ) 
				{
					aux = new LabelDSSR(conjVisitados, 0, numVisitados, 0, i, 0, custo, it, NULL);
					if ( vetorLabels[i].posAtual == NULL ) vetorLabels[i].posAtual = aux;
					if ( vetorLabels[i].cabeca == NULL ) vetorLabels[i].cabeca = aux;
					if ( vetorLabels[i].calda == NULL ) vetorLabels[i].calda = aux;
					else
					{
						vetorLabels[i].calda->prox = aux;
						vetorLabels[i].calda = aux;
					}

					if ( conjVisitados == 0 ) //caso nao existam requisicoes 'abertas', verifica o custo do caminho
					{
						custoAteDestino = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice, depositoArtificial);
						if ( ( custoAteDestino < ( valorSubtrair - 0.01 ) ) && ( custoAteDestino < menorCustoObtido ) )
						{
							menorLabelObtido = aux;
							menorCustoObtido = custoAteDestino;
							encontrouRotaNegativa = true;
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
	}

	if ( encontrouRotaNegativa == true )
	{
		//verifica se existe vertice que viola a elementaridade e retorna o mais violado, caso exista
		int maisViolado, maiorViolacao = 1;
		int* vetorViolacao = new int[nVertices+1];
		memset(vetorViolacao, 0, (nVertices+1)*sizeof(int));

		aux = menorLabelObtido;
		while ( aux != NULL )
		{
			++vetorViolacao[aux->ultimoVertice];
			if ( vetorViolacao[aux->ultimoVertice] > maiorViolacao )
			{
				maiorViolacao = vetorViolacao[aux->ultimoVertice];
				maisViolado = aux->ultimoVertice;
			}
			aux = aux->pai;
		}
		delete [] vetorViolacao;
		if ( maiorViolacao == 1 ) return 0;
		else return maisViolado;
	}
}


bool DSSRPDP::verificaLabelDominadoEDominaF(Grafo* g, int ult, float custo, int carga, unsigned long long int visit, unsigned long long int visitU)
{
	bool itUpdate, tmp;
	int domCarga, domCusto;
	ptrLabelDSSR aux, anterior = NULL;
	ptrLabelDSSR it = vetorLabels[ult].cabeca;

	while ( it != NULL )
	{
		//Verifico se a carga deste novo label domina o que ja estava em vLabel[ult]
		if ( carga < it->cargaAcumulada ) domCarga = 1;
		else if ( carga > it->cargaAcumulada ) domCarga = -1;
		else domCarga = 0;

		//Verifico se o custo deste novo label domina o label armazenado em vLabel[ult]
		if ( custo < it->custoDual ) domCusto = 1;
		else if ( custo > it->custoDual ) domCusto = -1;
		else domCusto = 0;

		//Verifico o numero de visitados e implemento o resto da dominancia
		itUpdate = true;
		if ( ( domCarga <= 0 ) && ( domCusto <= 0 ) ) //o label armazenado domina parcialmente o Novo label
		{
			tmp = false;
			for ( int j = 1; j <= nRequests; ++j )
			{
				if ( ( verticesFixarCoding & verticesCoding[j] ) > 0 )
				{
					if ( ( visitU & verticesCoding[j] ) < ( it->vertVisitados & verticesCoding[j] ) )
					{
						tmp = true;
						break;
					}
				}
				if ( ( visit & verticesCoding[j] ) < ( it->vertVisitados & verticesCoding[j] ) )
				{
					tmp = true;
					break;
				}
			}
			if ( !tmp ) return true; //o label armazenado em vLabel[ult] DOMINA COMPLETAMENTE o Novo label
		}
		else if ( ( domCarga >= 0 ) && ( domCusto >= 0 ) ) //Novo label domina parcialmente o label armazenado em vLabel[ult]
		{
			tmp = false;
			for ( int j = 1; j <= nRequests; ++j )
			{
				if ( ( verticesFixarCoding & verticesCoding[j] ) > 0 )
				{
					if ( ( it->vertVisitadosU & verticesCoding[j] ) < ( visit & verticesCoding[j] ) )
					{
						tmp = true;
						break;
					}
				}
				if ( ( it->vertVisitados & verticesCoding[j] ) < ( visit & verticesCoding[j] ) )
				{
					tmp = true;
					break;
				}
			}
			if ( !tmp ) //Novo Label DOMINA COMPLETAMENTE o label armazenado em vLabel[ult] que deve ser excluido
			{
				if ( vetorLabels[ult].posAtual == it ) vetorLabels[ult].posAtual = it->prox;
				if ( vetorLabels[ult].cabeca == it ) vetorLabels[ult].cabeca = it->prox;
				if ( vetorLabels[ult].calda == it ) vetorLabels[ult].calda = anterior;
				if ( anterior != NULL ) anterior->prox = it->prox;
				aux = it;
				it = it->prox;
				itUpdate = false;
				delete aux;
			}
		}

		//Atualiza os valores de it e anterior para continuar a verificacao em outros labels de vLabels[ult]
		if ( itUpdate )
		{
			anterior = it;
			it = it->prox;
		}
	}
	return false;
}


bool DSSRPDP::verificaLabelDominadoEDominaC(Grafo* g, int ult, float custo, int numVisit, unsigned long long int visit)
{
	bool itUpdate, tmp;
	ptrLabelDSSR aux, anterior = NULL;
	ptrLabelDSSR it = vetorLabels[ult].cabeca;

	while (it != NULL) 
	{
		itUpdate = true;
		if ( ( it->numVisitados < numVisit ) && ( it->custoDual <= custo ) ) //label armazenado pode dominar novo label
		{
			tmp = false;
			for ( int j = 1; j <= nRequests; ++j )
			{
				if ( ( it->vertVisitados & verticesCoding[j] ) > ( visit & verticesCoding[j] ) )
				{
					tmp = true;
					break;
				}
			}
			if ( !tmp ) return true;
		}
		else if ( ( numVisit < it->numVisitados ) && ( custo <= it->custoDual ) ) //novo label pode dominar label armazenado
		{
			tmp = false;
			for ( int j = 1; j <= nRequests; ++j )
			{
				if ( ( visit & verticesCoding[j] ) > ( it->vertVisitados & verticesCoding[j] ) )
				{
					tmp = true;
					break;
				}
			}
			if ( !tmp )
			{
				if ( vetorLabels[ult].posAtual == it ) vetorLabels[ult].posAtual = it->prox;
				if ( vetorLabels[ult].cabeca == it ) vetorLabels[ult].cabeca = it->prox;
				if ( vetorLabels[ult].calda == it ) vetorLabels[ult].calda = anterior;
				if ( anterior != NULL ) anterior->prox = it->prox;
				aux = it;
				it = it->prox;
				itUpdate = false;
				delete aux;
			}
		}
		else if ( it->vertVisitados == visit )
		{
			if ( it->custoDual < ( custo-0.001 ) ) return true;
			else if( custo < ( it->custoDual-0.001 ) )
			{
				if ( vetorLabels[ult].posAtual == it ) vetorLabels[ult].posAtual = it->prox;
				if ( vetorLabels[ult].cabeca == it ) vetorLabels[ult].cabeca = it->prox;
				if ( vetorLabels[ult].calda == it ) vetorLabels[ult].calda = anterior;
				if ( anterior != NULL ) anterior->prox = it->prox;
				aux = it;
				it = it->prox;
				itUpdate = false;
				delete aux;
			} 
		}

		if ( itUpdate )
		{
			anterior = it;
			it = it->prox;
		}
	}
	return false;
}


Rota** DSSRPDP::getRotaCustoMinimo(Grafo* g, float percentual){
	float limiteAceitacao, custoR;
	int depositoArtif = nVertices+1;

	//utiliza o percentual para obter rotas negativas (podem existir varias, pois nao parou na primeira)
	if ( menorCustoObtido < 0 ) limiteAceitacao = percentual * menorCustoObtido;
	else limiteAceitacao = (2-percentual) * menorCustoObtido;
	if ( limiteAceitacao > ( valorSubtrair - 0.01 ) ) limiteAceitacao = valorSubtrair - 0.01;

	int numRotasRetornadas = 0;
	//verifica todos os labels nao dominados (elementares) para alocar a quantidade de rotas a serem retornadas
	ptrLabelDSSR tmp, aux;
	for ( int i = nRequests+1; i <= nVertices; ++i )
	{
		aux = vetorLabels[i].cabeca;
		while ( aux != NULL )
		{
			if ( ( aux->vertVisitados == 0 ) && ( ( aux->custoDual + g->getCustoArestaDual(i, depositoArtif) ) < limiteAceitacao ) ) ++numRotasRetornadas;
			aux = aux->prox;
		}
	}

	if ( numRotasRetornadas > 0 )
	{
		Rota** r;
		r = new Rota*[numRotasRetornadas+1];
		int* vetorViolacao = new int[nVertices+1];
		numRotasRetornadas = 0;

		for ( int i = nRequests+1; i <= nVertices; ++i )
		{
			aux = vetorLabels[i].cabeca;
			while ( aux != NULL )
			{
				custoR = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice, depositoArtif);
				if ( ( aux->vertVisitados == 0 ) && ( custoR < limiteAceitacao ) )
				{
					memset(vetorViolacao, 0, (nVertices+1)*sizeof(int));
					r[numRotasRetornadas] = new Rota();
					r[numRotasRetornadas]->inserirVerticeFim(depositoArtif);
					tmp = aux;
					while ( tmp != NULL )
					{
						r[numRotasRetornadas]->inserirVerticeInicio(indiceVertDual[tmp->ultimoVertice]);
						++vetorViolacao[tmp->ultimoVertice];
						if ( vetorViolacao[tmp->ultimoVertice] > 1 )
						{
							delete r[numRotasRetornadas];
							break;
						}
						tmp = tmp->pai;
					}
					if ( tmp == NULL )
					{
						r[numRotasRetornadas]->inserirVerticeInicio(0);
						r[numRotasRetornadas]->setCustoReduzido(custoR);
						r[numRotasRetornadas]->setCustoRota(g, true);
						++numRotasRetornadas;
					}
				}
				aux = aux->prox;
			}
		}
		r[numRotasRetornadas] = NULL;
		delete [] vetorViolacao;
		return r;
	}
	else return NULL;
}

