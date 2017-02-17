#include "StateSpaceRelax.h"

StateSpaceRelax::StateSpaceRelax(Grafo* g, bool camForn, float valorSub) : verticesFixarCoding(0), numVerticesFixar(0), valorSubtrair(valorSub){
	ajuste = camForn ? 0 : g->getNumCmdt();
	numCommodities = camForn ? g->getNumFornDuais() : g->getNumConsDuais();

	//Aloca memoria para vetorLabels (que tera os mesmos labels de listaLabels, mas será utilizado por questoes de eficiencia)
	vetorLabels = new vLabel[numCommodities+1];

	//Aloco o vetor que armazena os indices dos vertices que devem ser fixados
	verticesFixar = new short int[numCommodities];

	//Aloca e inicializa o vetor que armazenara a codificacao dos vertices no sistema unsigned long long int
	verticesCoding = new unsigned long long int [numCommodities+1];
	verticesCoding[0] = 0;
	for (int i = 1; i <= numCommodities; ++i){
		verticesCoding[i] = pow (2, i);
	}
}

StateSpaceRelax::~StateSpaceRelax(){
	ptrLabel p, aux;
	//Exclui a memoria ainda alocada
	//vetorLabels[0] <=> listaLabels (aponta para todos os labels, de todos os vertices)
	for (int i = 1; i <= numCommodities; ++i){
		p = vetorLabels[i].cabeca;		
		while (p != NULL){
			aux = p;
			p = p->prox;	
			delete aux;
		}
	}
	delete [] vetorLabels;
	delete [] verticesCoding;
	delete [] verticesFixar;
}


int StateSpaceRelax::calculaCaminhoElementarSSR(Grafo* g, int classe){
	int violado;

	do{
		encontrouRotaNegativa = false;
		violado = calculaCaminhos(g, classe);
		if (!encontrouRotaNegativa) return -1;

		if (violado){ //prepara as variaveis para uma proxima iteracao

			//insere o vertice a ser fixo no vetor de vertices fixo
			verticesFixar[numVerticesFixar++] = violado;

			//insere '1' na posicao correspondente ao vertice violado em verticesFixarCoding
			verticesFixarCoding = (verticesFixarCoding | verticesCoding[violado]);

			//libera labels obtidos da iteracao anterior
			ptrLabel p, aux;
			for (int i = 1; i <= numCommodities; ++i){
				p = vetorLabels[i].cabeca;		
				while (p != NULL){
					aux = p;
					p = p->prox;	
					delete aux;
				}
			}

		}
	}while(violado);

	//caso alcance este ponto, tem-se uma solucao elementar de custo reduzido negativo
	//(garante-se que o label de menor custo do ESPPRC eh elementar, mas nao necessariamente todos obtidos)
	return 1;
}


int StateSpaceRelax::calculaCaminhos(Grafo* g, int classe){
	int depositoArtificial = g->getNumVertices()-1;
	int cargaMaxima = g->getCapacVeiculo(classe);
	int carga, numVisitados, menorIndice;
	unsigned long long int conjVisitados;
	float custo, custoAteDestino;
	ptrLabel aux, it;

	//insiro os labels relativos ao caminho de 0 aos vertices adjacentes
	//para depois processar todos os labels existentes em listaLabels,
	//armazenando apenas aqueles que chegam em 0' que sejam nao-dominados
	for (int i = 1; i <= numCommodities; ++i){
		carga = g->getPesoCommodity(i+ajuste);
		if (carga <= cargaMaxima){
			aux = new Label(verticesCoding[i], 1, i, 0, carga, g->getCustoArestaDual(0, i+ajuste), NULL);
			aux->pai = NULL;
			vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;
		}
	}

	menorIndice = 1;
	while(menorIndice > 0){
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		//Uma vez com o label 'it', verifica-se todos os adjacentes de 'it->ultimo' em busca 
		//de encontrar outros labels nao dominados para serem inseridos em vetorLabels[ultimoVertice]
		for (int i = 1; i <= numCommodities; ++i){
			carga = it->cargaAcumulada + g->getPesoCommodity(i+ajuste); 
			if (((it->vertVisitados & (verticesCoding[i] & verticesFixarCoding)) == 0) && (carga <= cargaMaxima)){
				conjVisitados = (it->vertVisitados | verticesCoding[i]);
				custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice+ajuste, i+ajuste);
				numVisitados = it->numVertVisitados+1;

				//Antes de executar o teste da dominancia deste label, verifico se existem vertices adjacentes 
				//para os quais ele eh unreachable, colocando 1 em sua posicao e incrementando numVisitados
				for (int z = 1; z <= numCommodities; ++z){
					if (((conjVisitados & verticesCoding[z]) == 0)  && (carga + g->getPesoCommodity(z+ajuste) > cargaMaxima)){
						conjVisitados = (conjVisitados | verticesCoding[z]);
						++numVisitados;
					}
				}

				//Caso o label a ser criado nao seja dominado, ele eh inserido no vetor de labels
				if (!verificaLabelDominadoEDomina(g, i, custo, carga, numVisitados, conjVisitados)){
					aux = new Label(conjVisitados, numVisitados, i, it->ultimoVertice, carga, custo, NULL);
					aux->pai = it;
					if (vetorLabels[i].posAtual == NULL) vetorLabels[i].posAtual = aux;
					vetorLabels[i].calda->prox = aux;
					vetorLabels[i].calda = aux;
				}
			}

			if ((vetorLabels[i].posAtual != NULL) && (vetorLabels[i].posAtual->custoDual < menorCustoAtual)){
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}
		if ( ( time(0) - Grafo::timeInicio ) > Grafo::timeLimit ) exit(0);
	}

	//verifica se a rota de menor custo eh negativa
	//(as demais implementacoes fazem isto a cada label criado, mas nesta implementacao isto falha)
	menorCustoObtido = MAIS_INFINITO;
	for (int i = 1; i <= numCommodities; ++i){
		aux = vetorLabels[i].cabeca;
		while (aux != NULL){
			custoAteDestino = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice+ajuste, depositoArtificial);
			if ((custoAteDestino < (valorSubtrair - 0.05)) && (custoAteDestino < menorCustoObtido)){
				menorCustoObtido = custoAteDestino;
				menorLabelObtido = aux;
				encontrouRotaNegativa = true;
			}
			aux = aux->prox;
		}
	}

	if (encontrouRotaNegativa == true){
		//verifica se existe vertice que viola a elementaridade e retorna o mais violado, caso exista
		int maisViolado, maiorViolacao = 1;
		int* vetorViolacao = new int[numCommodities+1];
		memset(vetorViolacao, 0, (numCommodities+1)*sizeof(int));

		aux = menorLabelObtido;
		while (aux != NULL){
			++vetorViolacao[aux->ultimoVertice];
			if (vetorViolacao[aux->ultimoVertice] > maiorViolacao){
				maiorViolacao = vetorViolacao[aux->ultimoVertice];
				maisViolado = aux->ultimoVertice;
			}
			aux = aux->pai;
		}

		if (maiorViolacao == 1) return 0;
		else return maisViolado;
	}
}


bool StateSpaceRelax::verificaLabelDominadoEDomina(Grafo* g, int ult, float custo, int carga, int numVisit, unsigned long long int visit){
	ptrLabel aux, anterior; //sera necessario na exclusao do label apontado por 'it' (caso este seja dominado)
	bool itUpdate;
	ptrLabel it = vetorLabels[ult].cabeca;
	int domCarga, domCusto, domVisit, tmp, tmp2;
	unsigned long long int v1, v2;

	while (it != NULL){ //Verifica a dominancia para todos os labels, ou seja, 
						//o novo label so sera inserido se nao houver nenhum q o domina (ou seja igual)

		//Verifico se a carga deste novo label domina o que ja estava em vLabel[ult]
		if (carga < it->cargaAcumulada){
			domCarga = 1;
		}else if (carga > it->cargaAcumulada){
			domCarga = -1;
		}else{
			domCarga = 0;
		}

		//Verifico se o custo deste novo label domina o label armazenado em vLabel[ult]
		if (custo < it->custoDual){
			domCusto = 1;
		}else if (custo > it->custoDual){
			domCusto = -1;
		}else{
			domCusto = 0;
		}

		//Verifico o numero de visitados e implemento o resto da dominancia
		itUpdate = true;
		if (numVisit < it->numVertVisitados){
			if ((domCarga >= 0) && (domCusto >= 0)){ //Novo label domina parcialmente o label armazenado em vLabel[ult]
				tmp = 0;
				for (int j = 0; j < numVerticesFixar; ++j){
					v1 = visit & verticesCoding[verticesFixar[j]];
					v2 = it->vertVisitados & verticesCoding[verticesFixar[j]];
					if (v2 < v1){
						tmp = 1;
						break;
					}
				}
				if (tmp == 0){ //Novo Label DOMINA COMPLETAMENTE o label armazenado em vLabel[ult] que deve ser excluido
					if (vetorLabels[ult].posAtual == it){
						vetorLabels[ult].posAtual = it->prox;
					}
					if (vetorLabels[ult].calda == it){
						vetorLabels[ult].calda = anterior;
					}
					anterior->prox = it->prox;
					aux = it;
					it = it->prox;
					itUpdate = false;
					delete aux;
				}
			}

		}else if (numVisit > it->numVertVisitados){
			if ((domCarga <= 0) && (domCusto <= 0)){ //o label armazenado em vLabel[ult] domina parcialmente o Novo label
				tmp = 0;
				for (int j = 0; j < numVerticesFixar; ++j){
					v1 = visit & verticesCoding[verticesFixar[j]];
					v2 = it->vertVisitados & verticesCoding[verticesFixar[j]];
					if (v1 < v2){
						tmp = 1;
						break;
					}
				}
				if (tmp == 0) return true; //o label armazenado em vLabel[ult] DOMINA COMPLETAMENTE o Novo label
			}

		//numero de vertices visitados eh igual, verifica os outros recursos para ver se existe dominancia
		}else{
			tmp = 0; tmp2 = 0;
			for (int j = 0; j < numVerticesFixar; ++j){
				v1 = visit & verticesCoding[verticesFixar[j]];
				v2 = it->vertVisitados & verticesCoding[verticesFixar[j]];
				if (v1 < v2){
					tmp = 1;
				}else if (v2 < v1){
					tmp2 = 1;
				}
			}

			if ((domCarga <= 0) && (domCusto <= 0) && (tmp == 0)){
				return true;
			}else if (((domCarga >= 0) && (domCusto >= 0) && (tmp2 == 0) && ((domCarga + domCusto + tmp) > 0))){
				//o novo label domina o armazenado que deve ser excluido
				if (vetorLabels[ult].posAtual == it){
					vetorLabels[ult].posAtual = it->prox;
				}
				if (vetorLabels[ult].calda == it){
					vetorLabels[ult].calda = anterior;
				}
				anterior->prox = it->prox;
				aux = it;
				it = it->prox;
				itUpdate = false;
				delete aux;
			}			

		}

		//Atualiza os valores de it e anterior para continuar a verificacao em outros labels de vLabels[ult]
		if (itUpdate){
			anterior = it;
			it = it->prox;
		}
	}
	return false;
}


Rota** StateSpaceRelax::getRotaCustoMinimo(Grafo* g, int classe, float percentual){
	float limiteAceitacao, custoR;
	int depositoArtif = g->getNumVertices()-1;

	//Antes de comecar a construir as rotas, obtenho vetor com a posicao original de cada vertice
	int *posicaoOriginal = g->getVetorPosicaoOriginal();

	//utiliza o percentual para obter rotas negativas (podem existir varias, pois nao parou na primeira)
	if (menorCustoObtido < 0){
		limiteAceitacao = percentual * menorCustoObtido;
	}else{
		limiteAceitacao = (2 - percentual) * menorCustoObtido;
	}
	if (limiteAceitacao > (valorSubtrair - 0.05)){
		limiteAceitacao = valorSubtrair - 0.05;
	}
	
	int numRotasRetornadas = 0;
	//Neste momento, verifica todos os labels nao dominados encontrados para alocar a quantidade de rotas a serem retornadas
	ptrLabel tmp, aux;
	for (int i = 1; i <= numCommodities; ++i){
		aux = vetorLabels[i].cabeca;
		while (aux != NULL){
			if ((aux->custoDual + g->getCustoArestaDual(i+ajuste, depositoArtif)) < limiteAceitacao){
				++numRotasRetornadas;
			}
			aux = aux->prox;
		}
	}

	if (numRotasRetornadas > 0){
		Rota** r;
		r = new Rota*[numRotasRetornadas+1];
		int* vetorViolacao = new int[numCommodities+1];
		numRotasRetornadas = 0;

		for (int i = 1; i <= numCommodities; ++i){
			aux = vetorLabels[i].cabeca;
			while (aux != NULL){
				custoR = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice+ajuste, depositoArtif);
				if (custoR < limiteAceitacao){
					memset(vetorViolacao, 0, (numCommodities+1)*sizeof(int));

					r[numRotasRetornadas] = new Rota();
					r[numRotasRetornadas]->inserirVerticeFim(depositoArtif);
					tmp = aux;
					while (tmp != NULL){
						r[numRotasRetornadas]->inserirVerticeInicio(posicaoOriginal[tmp->ultimoVertice+ajuste]);
						++vetorViolacao[tmp->ultimoVertice];
						if (vetorViolacao[tmp->ultimoVertice] > 1){
							delete r[numRotasRetornadas];
							break;
						}
						tmp = tmp->pai;
					}
					if (tmp == NULL){
						r[numRotasRetornadas]->inserirVerticeInicio(0);
						r[numRotasRetornadas]->setCustoReduzido(custoR);
						r[numRotasRetornadas]->setCusto(g, classe);
						++numRotasRetornadas;
					}
				}
				aux = aux->prox;
			}
		}		
		r[numRotasRetornadas] = NULL;
		return r;
	}else{
		return NULL;
	}
}

