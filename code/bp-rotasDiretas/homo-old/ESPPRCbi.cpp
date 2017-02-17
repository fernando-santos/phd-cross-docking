#include "ESPPRCbi.h"

ESPPRCbi::ESPPRCbi(Grafo* g, float* vetXi, float valorSubtrair) : vetorXi(vetXi){
	lixo = NULL;
	nRequests = g->getNumReqsDual();
	indiceVertDual = g->getIndiceVerticesDual();
	nVertices = 2*nRequests;
	vetorLabels = new vLabelBi[nVertices+1];
	alfa = valorSubtrair - 0.001;

	//Aloca e inicializa os vetores que armazenaram a codificacao dos vertices no sistema 'unsigned long long int'
	verticesCoding = new unsigned long long int [nRequests+1];
	for (int i = 1; i <= nRequests; ++i) verticesCoding[i] = pow (2, i);
	verticesCoding[0] = 0;
}

ESPPRCbi::~ESPPRCbi(){
	ptrLabelBi p, aux;
	for (int i = 1; i <= nVertices; ++i)
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

int ESPPRCbi::calculaCaminhoElementarBi(Grafo* g){
	float custo;
	bool dominado;
	ptrLabelBi aux, it;
	int depositoArtificial = nVertices+1;
	int cargaMaxima = g->getCapacVeiculo();
	int carga, menorIndice, numVisit, numVisitU;
	unsigned long long int conjVisitados, conjVisitadosU, reachable;

	for (int i = 1; i <= nRequests; ++i)
	{
		//Cria labels com caminhos de 0 a todos os FORNECEDORES
		carga = g->getCargaVertice(indiceVertDual[i]);
		aux = new LabelBi(verticesCoding[i], verticesCoding[i], 1, 1, i, carga, g->getCustoArestaDual(0, i), NULL, NULL);
		vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;

		//Cria labels com caminhos de todos os CONSUMIDORES a 0'
		carga = g->getCargaVertice(indiceVertDual[nRequests+i]);
		aux = new LabelBi(verticesCoding[i], verticesCoding[i], 1, 1, nRequests+i, carga, g->getCustoArestaDual(nRequests+i, depositoArtificial), NULL, NULL);
		vetorLabels[nRequests+i].cabeca = vetorLabels[nRequests+i].calda = vetorLabels[nRequests+i].posAtual = aux;
	}

	menorIndice = 1;
	while( menorIndice > 0 ) //estende todos os possiveis labels de fornecedores
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		//Uma vez com o label 'it', verifica-se todos os adjacentes de 'it->ultimo' em busca de outros labels nao dominados.
		for ( int i = 1; i <= nRequests; ++i )
		{
			if ( g->existeArestaDual(it->ultimoVertice, i) )
			{
				carga = it->cargaAcumulada + g->getCargaVertice(indiceVertDual[i]);
				reachable = ( it->vertVisitadosU & verticesCoding[i] );

				if ( ( reachable == 0 ) && ( carga <= cargaMaxima ) )
				{
					custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice, i);
					conjVisitados = (it->vertVisitados | verticesCoding[i]);
					numVisit = it->numVisitados+1;
					conjVisitadosU = (it->vertVisitadosU | verticesCoding[i]);
					numVisitU = it->numVisitadosU+1;

					//Antes de executar o teste da dominancia, marco aqueles vertices 'unreachable'
					for ( int j = 1; j <= nRequests; ++j )
					{
						if ( ( ( conjVisitadosU & verticesCoding[j] ) == 0 ) && ( ( carga + g->getCargaVertice(indiceVertDual[j]) ) > cargaMaxima ) )
						{
							conjVisitadosU = (conjVisitadosU | verticesCoding[j]);
							++numVisitU;
						}
					}

					if ( !verificaLabelDominadoEDomina(g, conjVisitados, conjVisitadosU, numVisit, numVisitU, i, custo) )
					{
						aux = new LabelBi(conjVisitados, conjVisitadosU, numVisit, numVisitU, i, carga, custo, it, NULL);
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

			//Pega o menor label para ser o proximo a ser processado (essa estrategia acelera a busca por rotas negativas)
			if ((vetorLabels[i].posAtual != NULL) && (vetorLabels[i].posAtual->custoDual < menorCustoAtual))
			{
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}
	}

	menorIndice = nRequests+1;
	while( menorIndice > 0 ) //estende todos os possiveis labels de consumidores
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		//Uma vez com o label 'it', verifica-se todos os adjacentes de 'it->ultimo' em busca de outros labels nao dominados.
		for ( int i = nRequests+1; i <= nVertices; ++i )
		{
			if ( g->existeArestaDual(i, it->ultimoVertice) )
			{
				carga = it->cargaAcumulada + g->getCargaVertice(indiceVertDual[i]);
				reachable = ( it->vertVisitadosU & verticesCoding[i-nRequests] );

				if ( ( reachable == 0 ) && ( carga <= cargaMaxima ) )
				{
					custo = it->custoDual + g->getCustoArestaDual(i, it->ultimoVertice);
					conjVisitados = (it->vertVisitados | verticesCoding[i-nRequests]);
					numVisit = it->numVisitados+1;
					conjVisitadosU = (it->vertVisitadosU | verticesCoding[i-nRequests]);
					numVisitU = it->numVisitadosU+1;

					//Antes de executar o teste da dominancia, marco aqueles vertices 'unreachable'
					for ( int j = 1; j <= nRequests; ++j )
					{
						if ( ( ( conjVisitadosU & verticesCoding[j] ) == 0 ) && ( ( carga + g->getCargaVertice(indiceVertDual[nRequests+j]) ) > cargaMaxima ) )
						{
							conjVisitadosU = (conjVisitadosU | verticesCoding[j]);
							++numVisitU;
						}
					}

					if ( !verificaLabelDominadoEDomina(g, conjVisitados, conjVisitadosU, numVisit, numVisitU, i, custo) )
					{
						aux = new LabelBi(conjVisitados, conjVisitadosU, numVisit, numVisitU, i, carga, custo, it, NULL);
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

			//Pega o menor label para ser o proximo a ser processado (essa estrategia acelera a busca por rotas negativas)
			if ((vetorLabels[i].posAtual != NULL) && (vetorLabels[i].posAtual->custoDual < menorCustoAtual))
			{
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}
	}

	//neste ponto, todos os labels nao-dominados para fornecedores e consumidores estao armazenados.
	//Realiza-se o 'join' dos labels para juntar os caminhos (incluindo o custo de troca)
	menorCustoObtido = MAIS_INFINITO;
	for ( int i = 1; i <= nRequests; ++i )
	{
		aux = vetorLabels[i].cabeca;
		while ( aux != NULL )
		{
			for ( int j = nRequests+1; j <= nVertices; ++j )
			{
				it = vetorLabels[j].cabeca;
				while ( it != NULL )
				{
					custo = g->getCustoArestaDual(i, j);
					for ( int k = 1; k <= nRequests; ++k )
					{
						if ( ( it->vertVisitados & verticesCoding[k] ) > ( aux->vertVisitados & verticesCoding[k] ) ) custo += vetorXi[indiceVertDual[k]];
					}
					custo += ( aux->custoDual + it->custoDual );
					if ( custo < alfa )
					{
						violados.push_back(new rotaBi(custo, aux, it));
						if ( custo < menorCustoObtido ) menorCustoObtido = custo;
					}
					it = it->prox;
				}
			}
			aux = aux->prox;
		}
	}

	if ( menorCustoObtido < MAIS_INFINITO )	return 1;
	else return -1;
}

bool ESPPRCbi::verificaLabelDominadoEDomina(Grafo* g, unsigned long long int visit, unsigned long long int visitU, int numVisit, int numVisitU, int ult, float custo){
	bool itUpdate, domVisit;
	ptrLabelBi aux, anterior = NULL;
	ptrLabelBi it = vetorLabels[ult].cabeca;
	float possivelCustoTroca, possivelCustoTroca2;

	while (it != NULL)
	{
		itUpdate = true;
		if ( ( it->numVisitados < numVisitU ) && ( it->custoDual <= custo ) )
		{
			domVisit = true;
			possivelCustoTroca = 0;
			for ( int i = 1; i <= nRequests; ++i )
			{
				if ( ( it->vertVisitados & verticesCoding[i] ) > ( visitU & verticesCoding[i] ) )
				{
					domVisit = false;
					break;
				}
				else if ( ( it->vertVisitados & verticesCoding[i] ) < ( visit & verticesCoding[i] ) )
				{
					possivelCustoTroca += vetorXi[indiceVertDual[i]];
				}
			}
			if ( ( domVisit ) && ( ( it->custoDual + possivelCustoTroca ) <= custo ) ) return true; //novo label dominado
		}
		else if ( ( numVisit < it->numVisitadosU ) && ( custo <= it->custoDual ) )
		{
			domVisit = true;
			possivelCustoTroca = 0;
			for ( int i = 1; i <= nRequests; ++i )
			{
				if ( ( visit & verticesCoding[i] ) > ( it->vertVisitadosU & verticesCoding[i] ) )
				{
					domVisit = false;
					break;
				}
				else if ( ( visit & verticesCoding[i] ) < ( it->vertVisitados & verticesCoding[i] ) )
				{
					possivelCustoTroca += vetorXi[indiceVertDual[i]];
				}
			}
			if ( ( domVisit ) && ( ( custo + possivelCustoTroca ) <= it->custoDual ) ) //o novo label domina o armazenado que deve ser excluido
			{
				if (vetorLabels[ult].posAtual == it) vetorLabels[ult].posAtual = it->prox;
				if (vetorLabels[ult].calda == it) vetorLabels[ult].calda = anterior;
				anterior->prox = it->prox;
				aux = it;
				it = it->prox;
				itUpdate = false;
				//delete aux;
				
				aux->prox = lixo;
				lixo = aux;
			}
		}
		else if ( it->vertVisitados == visit )
		{
			if ( it->custoDual <= custo ) return true;
			else
			{
				if (vetorLabels[ult].posAtual == it) vetorLabels[ult].posAtual = it->prox;
				if (vetorLabels[ult].calda == it) vetorLabels[ult].calda = anterior;
				anterior->prox = it->prox;
				aux = it;
				it = it->prox;
				itUpdate = false;
				//delete aux;
				
				aux->prox = lixo;
				lixo = aux;
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

Rota** ESPPRCbi::getRotaCustoMinimo(Grafo* g, float percentual){
	ptrLabelBi itF, itB;
	int depositoArtif = nVertices+1, numRotas = 0;
	float limiteAceitacao, custoR, menorCusto = MAIS_INFINITO;

	if ( menorCustoObtido < MAIS_INFINITO )
	{
		limiteAceitacao = ( menorCustoObtido < 0 ) ? percentual*menorCustoObtido : (2-percentual)*menorCustoObtido;
		if ( limiteAceitacao > ( alfa - 0.001 ) ) limiteAceitacao = alfa - 0.001;

		Rota** rr = new Rota*[violados.size()+1];
		for ( int i = 0; i < violados.size(); ++i )
		{
			if ( violados[i]->custo < limiteAceitacao )
			{
				itF = violados[i]->forward;
				itB = violados[i]->backward;
				rr[numRotas] = new Rota();
				while ( itF != NULL )
				{
					rr[numRotas]->inserirVerticeInicio(indiceVertDual[itF->ultimoVertice]);
					itF = itF->pai;
				}
				rr[numRotas]->inserirVerticeInicio(0);
				while ( itB != NULL )
				{
					rr[numRotas]->inserirVerticeFim(indiceVertDual[itB->ultimoVertice]);
					itB = itB->pai;
				}
				rr[numRotas]->inserirVerticeFim(depositoArtif);
				rr[numRotas]->setCustoReduzido(violados[i]->custo);
				rr[numRotas]->setCustoRota(g, false);
				++numRotas;
			}
		}		
		rr[numRotas] = NULL;
		return rr;
	}
	else
	{
		return NULL;
	}
}
