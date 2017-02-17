#include "ElementarM2.h"

Elementar::Elementar(Grafo* g, float* vetXi, float valorSubtrair) : vetorXi(vetXi), maiorXi(MAIS_INFINITO), alfa(valorSubtrair), 
																	menorLabelObtido(NULL), menorCustoObtido(MAIS_INFINITO){
	nRequests = g->getNumReqs();
	nVertices = 2*nRequests;
	vetorLabels = new vLabel[nVertices+1];
	vetorCargaProcessado = new int[nRequests+1];

	//Processa as informacoes de peso dos vertices armazenando-as em 'vetorPesoProcessado' (posicao[k] = \sum_{i = 1}^{i = k} {i-esima menor carga})
	int auxCarga, *tempCarga = new int[nRequests+1]; //copia as informacoes de carga para o vetor tempCarga
	for (int i = 1; i <= nRequests; ++i){
		tempCarga[i] = g->getCargaVertice(i);
	}
	for (int i = 1; i < nRequests; ++i){ //ordena em ordem crescente
		for (int j = i+1; j <= nRequests; ++j){
			if (tempCarga[j] < tempCarga[i]){
				auxCarga = tempCarga[i];
				tempCarga[i] = tempCarga[j];
				tempCarga[j] = auxCarga;
			}
		}
		if (vetorXi[i] < maiorXi){ //pega o maior valor do vetorXi
			maiorXi = vetorXi[i];
		}
	}
	for (int i = 0; i <= nRequests; ++i){
		vetorCargaProcessado[i] = 0;
		for (int j = 1; j <= i; ++j){
			vetorCargaProcessado[i] += tempCarga[j];
		}
	}

	//Aloca e inicializa os vetores que armazenam a codificacao dos vertices no sistema 'unsigned long long int'
	verticesCoding = new unsigned long long int [nRequests+1];
	verticesCoding[0] = 0;
	for (int i = 1; i <= nRequests; ++i){
		verticesCoding[i] = pow (2, i);
	}
	delete [] tempCarga;
}

Elementar::~Elementar(){
	ptrLabel p, aux;
	for (int i = 1; i <= nVertices; ++i){
		p = vetorLabels[i].cabeca;
		while (p != NULL){
			aux = p;
			p = p->prox;
			delete aux;
		}
	}
	delete [] vetorLabels;
	delete [] verticesCoding;
	delete [] vetorCargaProcessado;
}

int Elementar::calculaCaminhoElementar(Grafo* g){
	ptrLabel aux, it;
	float custo, custoAteDestino;
	int depositoArtificial = nVertices+1;
	int cargaMaxima = g->getCapacVeiculo();
	int carga, menorIndice, numVisitadosF, numVisitadosC;
	unsigned long long int conjVisitadosF, conjVisitadosC, repetido;
	bool dominado;

	//Cria os primeiros labels, com caminhos de 0 a todos os FORNECEDORES
	for ( int i = 1; i <= nRequests; ++i )
	{
		carga = g->getCargaVertice(i);
		if ( ( g->existeAresta( 0, i ) ) && ( carga <= cargaMaxima ) )
		{
			aux = new Label(verticesCoding[i], 0, 1, 0, i, 0, carga, g->getCustoArestaDual(0,i), NULL);
			aux->pai = NULL;
			vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;

			//estende cada label a todos os possiveis consumidores (necessario para colocar o primeiro label em vetorLabels e evitar segfault)
			for ( int j = nRequests+1; j <= nVertices; ++j )
			{
				carga = g->getCargaVertice(j);
				if ( ( g->existeAresta( i, j ) ) && ( carga <= cargaMaxima ) )
				{
					custo = aux->custoDual + g->getCustoArestaDual(i,j);
					if (i != (j-nRequests)) custo += vetorXi[j-nRequests];
					it = new Label(verticesCoding[i], verticesCoding[j-nRequests], 1, 1, j, i, carga, custo, NULL);
					it->pai = aux;
					vetorLabels[j].cabeca = vetorLabels[j].calda = vetorLabels[j].posAtual = it;

					//verifica se este label tem custo reduzido negativo, caso tenha, ele sera usado para compor uma rota
					custoAteDestino = it->custoDual + g->getCustoArestaDual(j, depositoArtificial);
					if ( ( custoAteDestino < ( menorCustoObtido - 0.001 ) && (custoAteDestino < ( alfa - 0.001 ) ) ) )
					{
						menorLabelObtido = it;
						menorCustoObtido = custoAteDestino;
					}
				}
			}
		}
	}

	menorIndice = 1;
	while(menorIndice > 0){
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		//Uma vez com o label 'it', verifica-se todos os adjacentes de 'it->ultimo' em busca de outros labels nao dominados.
		//OBS1.: Labels de FORNECEDORES podem ser estendidos para FORNECEDORES ou CONSUMIDORES
		//OBS2.: Labels de CONSUMIDORES podem ser estendidos para CONSUMIDORES ou DEPOSITO ARTIFICIAL
		for ( int i = 1; i <= nVertices; ++i )
		{
			if ( g->existeAresta(it->ultimoVertice, i) )
			{
				carga = ((it->ultimoVertice <= nRequests) && (i > nRequests)) ? g->getCargaVertice(i) : it->cargaAcumulada + g->getCargaVertice(i);
				repetido = (i > nRequests) ? (it->vertVisitadosC & verticesCoding[i-nRequests]) : (it->vertVisitadosF & verticesCoding[i]);

				if ( ( repetido == 0 ) && ( carga <= cargaMaxima ) )
				{
					custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice, i);

					if ( i > nRequests )
					{
						if ((it->vertVisitadosF & verticesCoding[i-nRequests]) == 0) custo += vetorXi[i-nRequests];
						conjVisitadosF = it->vertVisitadosF;
						numVisitadosF = it->numVertVisitadosF;
						conjVisitadosC =  (it->vertVisitadosC | verticesCoding[i-nRequests]);
						numVisitadosC = it->numVertVisitadosC+1;

						//Antes de executar o teste da dominancia, marco aqueles vertices 'unreachable'
						for ( int j = 1; j <= nRequests; ++j )
						{
							if ( ( ( conjVisitadosC & verticesCoding[j] ) == 0 ) && ( ( carga + g->getCargaVertice( j+nRequests ) ) > cargaMaxima ) )
							{
								conjVisitadosC = (conjVisitadosC | verticesCoding[j]);
								++numVisitadosC;
							}
						}
						dominado = verificaLabelDominadoEDominaC(g, conjVisitadosF, conjVisitadosC, numVisitadosF, numVisitadosC, i, custo, carga);
					}
					else
					{
						conjVisitadosF = (it->vertVisitadosF | verticesCoding[i]);
						numVisitadosF = it->numVertVisitadosF+1;
						conjVisitadosC =  0;
						numVisitadosC = 0;

						//Para os fornecedores, nao posso marcar os vertices 'unreachable', pois atrapalha a regra de dominancia
						dominado = verificaLabelDominadoEDominaF(g, conjVisitadosF, numVisitadosF, i, custo, carga);
					}

					if ( !dominado )
					{
						aux = new Label(conjVisitadosF, conjVisitadosC, numVisitadosF, numVisitadosC, i, it->ultimoVertice, carga, custo, NULL);
						aux->pai = it;
						if (vetorLabels[i].posAtual == NULL) vetorLabels[i].posAtual = aux;
						vetorLabels[i].calda->prox = aux;
						vetorLabels[i].calda = aux;

						if ( i > nRequests )
						{
							custoAteDestino = aux->custoDual + g->getCustoArestaDual(i, depositoArtificial);
							if ( ( custoAteDestino < ( menorCustoObtido - 0.001 ) && (custoAteDestino < ( alfa - 0.001 ) ) ) )
							{
								menorLabelObtido = aux;
								menorCustoObtido = custoAteDestino;
							}
						}
					}
				}
			}

			//Pega o menor label para ser o proximo a ser processado (essa estrategia acelera a busca por rotas negativas)
			if ( ( vetorLabels[i].posAtual != NULL ) && ( vetorLabels[i].posAtual->custoDual < menorCustoAtual ) )
			{
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}
	}
}

bool Elementar::verificaLabelDominadoEDominaF(Grafo* g, unsigned long long int visitF, int numVisitF, int ult, float custo, int carga){
	bool itUpdate;
	ptrLabel aux, anterior, it = vetorLabels[ult].cabeca;
	int domCarga, domCusto;

	while (it != NULL) //o novo label so sera inserido se nao houver nenhum q o domina (ou seja igual)
	{
		//Verifico se a carga deste novo label domina o que ja estava em vLabel[ult]
		if ( carga < it->cargaAcumulada )
		{
			domCarga = 1;
		}
		else if ( carga > it->cargaAcumulada )
		{
			domCarga = -1;
		}
		else
		{
			domCarga = 0;
		}

		//Verifico se o custo deste novo label domina o label armazenado em vLabel[ult]
		if ( custo < it->custoDual )
		{
			domCusto = 1;
		}
		else if ( custo > it->custoDual )
		{
			domCusto = -1;
		}
		else
		{
			domCusto = 0;
		}
		
		//Verifica se para todo vertice i, novoLabel->visit[i] <= labelArmazenado->visit[i] 
		//(sendo que 0 = nao visitado e 1 = visitado)
		itUpdate = true;
		if ( numVisitF < it->numVertVisitadosF )
		{
			if ( ( domCarga >= 0 ) && ( domCusto >= 0 ) ) //Novo Label DOMINA parcialmente
			{
				bool domVisit = true;
				float custoTrocaDominado = 0;
				for ( int j = 1; j <= nRequests; ++j )
				{
					if ( ( it->vertVisitadosF & verticesCoding[j] ) < ( visitF & verticesCoding[j] ) )
					{
						domVisit = false;
						break;
					}
					else if ( ( it->vertVisitadosF & verticesCoding[j] ) > ( visitF & verticesCoding[j] ) )
					{
						custoTrocaDominado += vetorXi[j];
					}
				}
				if ( ( domVisit ) && ( custo < ( it->custoDual - custoTrocaDominado ) ) ) //Novo Label domina totalmente
				{
					if ( vetorLabels[ult].posAtual == it )
					{
						vetorLabels[ult].posAtual = it->prox;
					}
					if (vetorLabels[ult].calda == it)
					{
						vetorLabels[ult].calda = anterior;
					}
					anterior->prox = it->prox;
					aux = it;
					it = it->prox;
					itUpdate = false;
					delete aux;
				}
			}
		}
		else if ( numVisitF > it->numVertVisitadosF )
		{
			if ( ( domCarga <= 0 ) && ( domCusto <= 0 ) ) //Dominancia parcial do label armazenado
			{
				bool domVisit = true;
				float custoTrocaDominado = 0;
				for ( int j = 1; j <= nRequests; ++j )
				{
					if ( ( visitF & verticesCoding[j] ) < ( it->vertVisitadosF & verticesCoding[j] ) )
					{
						domVisit = false;
						break;
					}
					else if ( ( visitF & verticesCoding[j] ) > ( it->vertVisitadosF & verticesCoding[j] ) )
					{
						custoTrocaDominado += vetorXi[j];
					}
				}
				if ( ( domVisit ) && ( it->custoDual < ( custo - custoTrocaDominado ) ) ) return true; //dominancia total do label armazenado
			}
		}
		else
		{
			if ( visitF == it->vertVisitadosF )
			{
				if ((domCarga <= 0) && (domCusto <= 0) )
				{
					return true;
				}
				else if ( ( domCarga >= 0 ) && ( domCusto >= 0) )
				{
					//o novo label domina o armazenado que deve ser excluido
					if ( vetorLabels[ult].posAtual == it )
					{
						vetorLabels[ult].posAtual = it->prox;
					}
					if ( vetorLabels[ult].calda == it )
					{
						vetorLabels[ult].calda = anterior;
					}
					anterior->prox = it->prox;
					aux = it;
					it = it->prox;
					itUpdate = false;
					delete aux;
				}
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

bool Elementar::verificaLabelDominadoEDominaC(Grafo* g, unsigned long long int visitF, unsigned long long int visitC, 
														int numVisitF, int numVisitC, int ult, float custo, int carga){
	bool itUpdate;
	ptrLabel aux, anterior, it = vetorLabels[ult].cabeca;
	int domCarga, domCusto, domNumVisitF, domNumVisitC, tmp;
	float possivelCustoTrocaNovo, possivelCustoTrocaArmazenado;
	int maxPossiveisTrocas, cargaDisponivel = g->getCapacVeiculo() - carga;

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

		//dominancia com relacao ao numero de vertices visitados nos fornecedores
		//OBS.: O CRITERIO DE DOMINANCIA DESTE RECURSO EH INVERTIDO COM RELACAO AOS DEMAIS
		//POIS AQUELE LABEL QUE TEM UM NUMERO MAIOR DE VERTICES VISITADOS DOMINA AQUELE COM
		//UM NUMERO MENOR, JA QUE QUANTO MAIS VERTICES FORNECEDORES VISITADOS, MENOR O NUMERO DE 
		//VERTICES CONSUMIDORES NECESSARIOS A SE PAGAR O CUSTO DE TROCA NA VISITA
		if (numVisitF < it->numVertVisitadosF){
			domNumVisitF = -1;
		}else if (numVisitF > it->numVertVisitadosF){
			domNumVisitF = 1;
		}else{
			domNumVisitF = 0;
		}

		//o criterio de dominancia deste recurso eh feito do modo convencional, quanto menos vertices visitar
		//melhor, pois existirao vertices cuja visita podera diminuir o custo do label devido ao seu custo reduzido
		if (numVisitC < it->numVertVisitadosC){
			domNumVisitC = 1;
		}else if (numVisitC > it->numVertVisitadosC){
			domNumVisitC = -1;
		}else{
			domNumVisitC = 0;
		}

		itUpdate = true;
		if ((domCusto <= 0) && (domCarga <= 0) && (domNumVisitF <= 0) && (domNumVisitC <= 0)) //Label armazenado DOMINA PARCIALMENTE Novo label
		{
			tmp = 0; maxPossiveisTrocas = 0;
			//Verifica se existe dominancia com relacao ao conjunto de vertices visitados (fornecedores e consumidores)
			for (int i = 1; i <= nRequests; ++i)
			{
				if ((visitF & verticesCoding[i]) < (it->vertVisitadosF & verticesCoding[i])) //'label armazenado' visita FORNECEDOR nao visitado por 'novo label'
				{
					tmp = 1;
					break;
				}
				else if (((it->vertVisitadosC & verticesCoding[i]) == 0) && ((visitF & verticesCoding[i]) > (it->vertVisitadosF & verticesCoding[i])))
				{
					++maxPossiveisTrocas;
					if ((it->custoDual + vetorXi[i]) > custo)
					{
						tmp = 1;
						break;
					}
				}
				if ((visitC & verticesCoding[i]) < (it->vertVisitadosC & verticesCoding[i])) //'label armazenado' visita CONSUMIDOR nao visitado por 'novo label'
				{
					tmp = 1;
					break;
				}
			}

			if (tmp == 0)
			{
				while(vetorCargaProcessado[maxPossiveisTrocas] > cargaDisponivel) --maxPossiveisTrocas;
				if ((it->custoDual + maxPossiveisTrocas*maiorXi) < custo) return true;
			}
		}
		else if ((domCusto >= 0) && (domCarga >= 0) && (domNumVisitF >= 0) && (domNumVisitC >= 0)) //Novo label DOMINA PARCIALMENTE label armazenado
		{
			tmp = 0; maxPossiveisTrocas = 0;
			//Verifica se existe dominancia com relacao ao conjunto de vertices visitados (fornecedores e consumidores)
			for (int i = 1; i <= nRequests; ++i){
				if ((it->vertVisitadosF & verticesCoding[i]) < (visitF & verticesCoding[i])){ //'novo label' visita um FORNECEDOR nao visitado pelo 'label armazenado'
					tmp = 1; break;
				}else if (((visitC & verticesCoding[i]) == 0) && (it->vertVisitadosF & verticesCoding[i]) > (visitF & verticesCoding[i])){
					++maxPossiveisTrocas;
					if ((custo + vetorXi[i]) > it->custoDual){ //nao dominancia por troca
						tmp = 1; break;
					}
				}
				if ((it->vertVisitadosC & verticesCoding[i]) < (visitC & verticesCoding[i])){ //'label armazenado' visita um CONSUMIDOR nao visitado pelo 'novo label'
					tmp = 1; break;
				}
			}
			if (tmp == 0){
				while(vetorCargaProcessado[maxPossiveisTrocas] > cargaDisponivel) --maxPossiveisTrocas;
				if ((custo + maxPossiveisTrocas*maiorXi) < it->custoDual){
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
		} //Caso nao satisfaca nenhum dos casos acima, nao eh possivel estabelecer dominancia entre 'novo label' e 'label armazenado'

		//Atualiza os valores de it e anterior para continuar a verificacao em outros labels de vLabels[ult]
		if (itUpdate){
			anterior = it;
			it = it->prox;
		}
	}
	return false;
}

Rota* Elementar::getRotaCustoMinimo(Grafo* g){
	if (menorLabelObtido == NULL){
		return NULL;
	}else{
		Rota* r = new Rota();
		r->inserirVerticeFim(nVertices+1);
		while (menorLabelObtido != NULL){
			r->inserirVerticeInicio(menorLabelObtido->ultimoVertice);
			menorLabelObtido = menorLabelObtido->pai;
		}
		r->inserirVerticeInicio(0);
		r->setCustoRota(g);
		r->setCustoReduzido(menorCustoObtido);
		return r;
	}
}


void Elementar::imprimeLabel(ptrLabel aux){
	char* binF = decToBin(aux->vertVisitadosF);
	char* binC = decToBin(aux->vertVisitadosC);
	cout<< "[" << binF << "-" << binC << ", '" << aux->numVertVisitadosF << "'-'" << aux->numVertVisitadosC << "', " << aux->verticeAntecessor
			 << "(a), "<< aux->ultimoVertice << "(u), " << aux->cargaAcumulada << "(c), " << aux->custoDual << "($)]" << endl;
	delete [] binF;
	delete [] binC;
}

char* Elementar::decToBin(unsigned long long int dec){
	char tmp[70];
	char* binario = new char[70];
	int i = 0;
	while(dec > 0){
		if(dec % 2 == 0){
			tmp[i] = '0';
		}else{
			tmp[i] = '1';
		}
		++i;
		dec = dec / 2;
	}
	tmp[i] = '\0';
	binario[i] = '\0';

	int j = 0;
	while(tmp[j] != '\0'){
		binario[--i] = tmp[j++];
	}

	return binario;
}
