#include "ElementarTW.h"

Rota** geraPoolRotas(Grafo* g, bool camForn, int inicioJanela, int fimJanela, int numVeicHeuristica, int& numRotasRetornadas){
	float t;
	bool inseriuVertice;
	int it, k, v, cargaTotal;
	int nCommodities = g->getNumCmdt();
	int capacidade = g->getCapacVeiculo();
	int depositoArtificial = 2*nCommodities+1;
	int ajuste = (camForn) ? 0 : nCommodities;
	char grupo = (camForn) ? 'F' : 'C';
	int numVerticesDescobertos;
	ptrRotaInicial* rotas = new ptrRotaInicial[numVeicHeuristica];
	int* vertices = new int[nCommodities+1];

	for (it = 0; it < 1000000; ++it){
		//Inicializa os valores do vetor de vertices a serem cobertos para a iteracao
		numVerticesDescobertos = nCommodities;
		for (int i = 1; i <= nCommodities; ++i){
			vertices[i] = i;
		}

		//A principio, cria uma rota com um unico vertice (escolhido aleatoriamente) para cada veiculo
		//Supondo que ao chegar neste ponto, a janela de cada vertice pode ser atendida partindo-se do deposito
		for (k = 0; k < numVeicHeuristica; ++k){
			v = (rand() % numVerticesDescobertos) + 1;
			t = inicioJanela + g->getCustoAresta(0, vertices[v]+ajuste);
			if (t < g->getInicioJanela(vertices[v]+ajuste)) t = g->getInicioJanela(vertices[v]+ajuste);
			rotas[k] = new RotaInicial();
			rotas[k]->vertice = vertices[v];
			rotas[k]->tempoChegada = t;
			rotas[k]->prox = NULL;

			//Substitue o vertice coberto pelo ultimo do vetor
			vertices[v] = vertices[numVerticesDescobertos];
			--numVerticesDescobertos;
		}

		//Partindo-se das rotas singulares acima, tenta-se inserir aleatoriamente os vertices nas rotas ate cobrir todos
		k = 0;
		ptrRotaInicial aux1, aux2, tmp;
		while(numVerticesDescobertos > 0){
			inseriuVertice = false;
			v = (rand() % numVerticesDescobertos) + 1; //escolhe-se o indice de um vertice (vertices[v] representa o vertice)

			//Tenta inserir o vertice escolhido em um veiculo, passando para o proximo, caso nao seja possivel inserir no atual
			for (int veic = k; veic < (k+numVeicHeuristica); ++veic){

				//primeiro verifica se a cargaTotal excede a capacidade do veiculo
				cargaTotal = g->getPesoCommoditySemCorrigirPosicao(vertices[v]+ajuste);
				tmp = rotas[veic % numVeicHeuristica];
				while(tmp != NULL){
					cargaTotal += g->getPesoCommoditySemCorrigirPosicao(tmp->vertice+ajuste);
					tmp = tmp->prox;
				}

				if (cargaTotal <= capacidade){

					//A primeira coisa eh verificar se eh possivel inserir o vertice entre 0 e rota[k]
					tmp = rotas[veic % numVeicHeuristica];
					t = inicioJanela + g->getCustoAresta(0, vertices[v]+ajuste);
					if (t < g->getInicioJanela(vertices[v]+ajuste)) t = g->getInicioJanela(vertices[v]+ajuste);
					t += g->getCustoAresta(vertices[v]+ajuste, tmp->vertice+ajuste);

					//O novo tempo de chegada em tmp sera sempre maior ou igual ao anterior devido a desigualdade triangular
					//Para assegurar viabilidade, deve-se verificar se o tempo de chegada continua viavel nos outros vertices da rota
					if (t < g->getFimJanela(tmp->vertice+ajuste)){
						while (tmp->prox != NULL){
							t += g->getCustoAresta(tmp->vertice+ajuste, tmp->prox->vertice+ajuste);
							if (t < g->getInicioJanela(tmp->prox->vertice+ajuste)) t = g->getInicioJanela(tmp->prox->vertice+ajuste);
							if (t > g->getFimJanela(tmp->prox->vertice+ajuste)){
								break;
							}
							tmp = tmp->prox;
						}
						if ((tmp->prox == NULL) && (t + g->getCustoAresta(tmp->vertice+ajuste, depositoArtificial) < fimJanela)){
							//significa que o vertice pode ser inserido entre 0 e 'aux2->vertice'
							t = inicioJanela + g->getCustoAresta(0, vertices[v]+ajuste);
							if (t < g->getInicioJanela(vertices[v]+ajuste)) t = g->getInicioJanela(vertices[v]+ajuste);
							tmp = new RotaInicial();
							tmp->vertice = vertices[v];
							tmp->tempoChegada = t;
							tmp->prox = rotas[veic % numVeicHeuristica];
							rotas[veic % numVeicHeuristica] = tmp;

							//A janela de tempo de todos os vertices posteriores na rota pode ser alterada com a insercao do novo vertice
							while(tmp->prox != NULL){
								t = tmp->tempoChegada + g->getCustoAresta(tmp->vertice+ajuste, tmp->prox->vertice+ajuste);
								if (t < g->getInicioJanela(tmp->prox->vertice+ajuste)) t = g->getInicioJanela(tmp->prox->vertice+ajuste);
								tmp->prox->tempoChegada = t;
								tmp = tmp->prox;
							}

							vertices[v] = vertices[numVerticesDescobertos];
							k = (veic % numVeicHeuristica) + 1;
							--numVerticesDescobertos;
							inseriuVertice = true;
							break;
						}
					}

					//Caso nao seja possivel inserir entre 0 seu adjacente, tenta inserir entre os vertices da rota (exceto 0 e deposito artificial)
					aux1 = rotas[veic % numVeicHeuristica];
					aux2 = aux1->prox;
					while(aux2 != NULL){

						t = aux1->tempoChegada + g->getCustoAresta(aux1->vertice+ajuste, vertices[v]+ajuste);
						if (t <= g->getFimJanela(vertices[v]+ajuste)){
							if (t < g->getInicioJanela(vertices[v]+ajuste)) t = g->getInicioJanela(vertices[v]+ajuste);
							t += g->getCustoAresta(vertices[v]+ajuste, aux2->vertice+ajuste);

							//O novo tempo de chegada em tmp sera sempre maior ou igual ao anterior devido a desigualdade triangular
							//Para assegurar viabilidade, deve-se verificar se o tempo de chegada continua viavel nos outros vertices da rota
							if (t < g->getFimJanela(aux2->vertice+ajuste)){
								tmp = aux2;
								while (tmp->prox != NULL){
									t += g->getCustoAresta(tmp->vertice+ajuste, tmp->prox->vertice+ajuste);
									if (t < g->getInicioJanela(tmp->prox->vertice+ajuste)) t = g->getInicioJanela(tmp->prox->vertice+ajuste);
									if (t > g->getFimJanela(tmp->prox->vertice+ajuste)){
										break;
									}
									tmp = tmp->prox;
								}
								if ((tmp->prox == NULL) && (t + g->getCustoAresta(tmp->vertice+ajuste, depositoArtificial) < fimJanela)){
									//significa que o vertice pode ser inserido entre 0 e 'aux2->vertice'
									t = aux1->tempoChegada + g->getCustoAresta(aux1->vertice+ajuste, vertices[v]+ajuste);
									if (t < g->getInicioJanela(vertices[v]+ajuste)) t = g->getInicioJanela(vertices[v]+ajuste);
									tmp = new RotaInicial();
									tmp->vertice = vertices[v];
									tmp->tempoChegada = t;
									tmp->prox = aux2;
									aux1->prox = tmp;

									while(tmp->prox != NULL){
										t = tmp->tempoChegada + g->getCustoAresta(tmp->vertice+ajuste, tmp->prox->vertice+ajuste);
										if (t < g->getInicioJanela(tmp->prox->vertice+ajuste)) t = g->getInicioJanela(tmp->prox->vertice+ajuste);
										tmp->prox->tempoChegada = t;
										tmp = tmp->prox;
									}

									vertices[v] = vertices[numVerticesDescobertos];
									k = (veic % numVeicHeuristica) + 1;
									--numVerticesDescobertos;
									inseriuVertice = true;
									break;
								}
							}
						}

						aux1 = aux2;
						aux2 = aux2->prox;
					}

					//significa que ainda nao encontrou uma posicao para inserir o vertice na rota[k]
					//realiza a ultima tentativa: inserir o vertice entre o ultimo vertice e o depositoArtificial

					if (inseriuVertice){
						break;
					}

					aux1 = rotas[veic % numVeicHeuristica];
					while(aux1->prox != NULL){
						aux1 = aux1->prox;
					}

					//aux1 aponta para o ultimo vertice da rota, basta verificar se o vertice pode ser inserido entre ele e o depositoArtificial
					t = aux1->tempoChegada + g->getCustoAresta(aux1->vertice+ajuste, vertices[v]+ajuste);
					if (t <= g->getFimJanela(vertices[v]+ajuste)){
						if (t < g->getInicioJanela(vertices[v]+ajuste)) t = g->getInicioJanela(vertices[v]+ajuste);

						if (t + g->getCustoAresta(vertices[v]+ajuste, depositoArtificial) < fimJanela){
							//significa que o vertice pode ser inserido entre 0 e 'aux2->vertice' (posInsercao = 1)
							tmp = new RotaInicial();
							tmp->vertice = vertices[v];
							tmp->tempoChegada = t;
							tmp->prox = NULL;
							aux1->prox = tmp;

							vertices[v] = vertices[numVerticesDescobertos];
							k = (veic % numVeicHeuristica) + 1;
							--numVerticesDescobertos;
							inseriuVertice = true;
							break;
						}
					}
				}
			}

			if (!inseriuVertice){
				//Essa tentativa nao deu certo, reinicia os vetores e ponteiros e passa para a proxima tentativa
				break;
			}
		}

		if (numVerticesDescobertos == 0){ //Significa que obteve as rotas viaveis
			break;
		}else{ //Limpa as rotas sem sucesso obtidas, para iniciar uma nova iteracao procurando por outras rotas
			for (int k = 0; k < numVeicHeuristica; ++k){
				tmp = rotas[k];
				while(tmp != NULL){
					delete tmp;
					tmp = tmp->prox;
				}
			}

		}
	}

	if (numVerticesDescobertos == 0){
		ptrRotaInicial aux;
		numRotasRetornadas = numVeicHeuristica;//2*nCommodities;
		Rota** poolRotas = new Rota*[numRotasRetornadas];

		//Insere as rotas associadas a solucao viavel encontrada
		for (int k = 0; k < numVeicHeuristica; ++k){
			poolRotas[k] = new Rota();
			while(rotas[k] != NULL){
				poolRotas[k]->inserirVerticeFim(rotas[k]->vertice+ajuste);
				aux = rotas[k];
				rotas[k] = rotas[k]->prox;
				delete aux;
			}
			poolRotas[k]->inserirVerticeInicio(0);
			poolRotas[k]->inserirVerticeFim(depositoArtificial);
			poolRotas[k]->setCusto(g);
		}

		g->setCapacVeiculo(capacidade);
		return poolRotas;

	}else{
		cout << "Não encontrou rotas viaveis para k = " << numVeicHeuristica << " - " << grupo << endl;
		exit(0);
	}
}


ElementarTW::ElementarTW(Grafo* g, bool camForn, float valorSub) : valorSubtrair(valorSub), menorCustoObtido(MAIS_INFINITO){
	ajuste = camForn ? 0 : g->getNumCmdt();
	numCommodities = camForn ? g->getNumFornDuais() : g->getNumConsDuais();
	
	//Aloca memoria para vetorLabels (que tera os mesmos labels de listaLabels, mas será utilizado por questoes de eficiencia)
	vetorLabels = new vLabelTW[numCommodities+1];

	//Aloca e inicializa o vetor que armazenara a codificacao dos vertices no sistema unsigned long long int
	verticesCoding = new unsigned long long int [numCommodities+1];
	verticesCoding[0] = 0;
	for (int i = 1; i <= numCommodities; ++i){
		verticesCoding[i] = pow (2, i);
	}
	//Armazeno tambem o complemento da representacao acima, que sera usada para calcular os labels
	verticesCodingCompl = new unsigned long long int [numCommodities+1];
	for (int i = 0; i <= numCommodities; ++i){
		verticesCodingCompl[i] = ~verticesCoding[i];
	}
}


ElementarTW::~ElementarTW(){
	ptrLabelTW p, aux;
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
	delete [] verticesCodingCompl;
}


int ElementarTW::calculaCaminhoElementar(Grafo* g, int inicioJanelaDeposito, int fimJanelaDeposito){
	int depositoArtificial = g->getNumVertices()-1;
	int cargaMaxima = g->getCapacVeiculo();
	int carga, numVisitados, menorIndice = 0;
	float custo, tempo, custoAteDestino;
	unsigned long long int conjVisitados;
	ptrLabelTW aux, it;
	bool encontrouRotaNegativa = false;

	//Crio um vetor que armazena o tempo gasto de cada vertice ate o deposito artificial
	//Este vetor sera verificado antes de se criar qualquer label, caso o tempo deste label
	//somado ao tempo necessario para se alcancar o destino exceda a janela do deposito
	//este label nao precisa ser criado, pois ele nao representara um caminho viavel]
	//APROVEITO E CRIO TAMBEM UM VETOR PARA ARMAZENAR A JANELA DE TEMPO, POR QUESTOES DE EFICIENCIA
	float *iAteDeposito = new float[numCommodities+1];
	int *tempoServico = new int[numCommodities+1];
	int **janelaTempo = new int*[numCommodities+1];
	janelaTempo[0] = new int[2];
	janelaTempo[0][0] = inicioJanelaDeposito;
	janelaTempo[0][1] = fimJanelaDeposito;	
	for (int i = 1; i <= numCommodities; ++i){
		janelaTempo[i] = new int[2];
		janelaTempo[i][0] = g->getInicioJanela(i+ajuste);
		janelaTempo[i][1] = g->getFimJanela(i+ajuste);

		tempoServico[i] = g->getTempoServico(i+ajuste);
		iAteDeposito[i] = g->getCustoAresta(i+ajuste, depositoArtificial);
	}
	
	//insiro os labels relativos ao caminho de 0 aos vertices adjacentes
	//para depois processar todos os labels existentes em listaLabels,
	//armazenando apenas aqueles que chegam em 0' que sejam nao-dominados
	for (int i = 1; i <= numCommodities; ++i){
		carga = g->getPesoCommodity(i+ajuste);
		tempo = inicioJanelaDeposito + g->getCustoAresta(0, i+ajuste);		
		if ((carga <= cargaMaxima) && (tempo <= janelaTempo[i][1])){
			if (tempo < janelaTempo[i][0]) tempo = janelaTempo[i][0];
			tempo += tempoServico[i];
			if ((tempo + iAteDeposito[i]) <= janelaTempo[0][1]){ //Cria o label apenas se ele for viavel ate o deposito artificial
				aux = new LabelTW(verticesCoding[i], 1, i, 0, carga, tempo, g->getCustoArestaDual(0, i+ajuste), NULL);
				aux->pai = NULL;
				vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;
			
				//verifica se estes primeiros labels dao origem a rotas de custo reduzido negativo
				custoAteDestino = aux->custoDual + g->getCustoArestaDual(i+ajuste, depositoArtificial);
				if (custoAteDestino < menorCustoObtido){
					menorCustoObtido = custoAteDestino;
					menorIndice = i;

					if (custoAteDestino < (valorSubtrair - 0.005)){
						menorLabelObtido = aux;
						encontrouRotaNegativa = true;
					}
				}
			}

		}
	}

	while(menorIndice > 0){
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		//Uma vez com o label 'it', verifica-se todos os adjacentes de 'it->ultimo' em busca 
		//de encontrar outros labels nao dominados para serem inseridos em vetorLabels[ultimoVertice]
		for (int i = 1; i <= numCommodities; ++i){
			carga = it->cargaAcumulada + g->getPesoCommodity(i+ajuste);
			tempo = it->tempoAcumulado + g->getCustoAresta(it->ultimoVertice+ajuste, i+ajuste);

			if (((it->vertVisitados & verticesCoding[i]) == 0) && (carga <= cargaMaxima) && (tempo <= janelaTempo[i][1])){
				if (tempo < janelaTempo[i][0]) tempo = janelaTempo[i][0];
				tempo += tempoServico[i];
				
				if ((tempo + iAteDeposito[i]) <= janelaTempo[0][1]){ //Cria o label apenas se ele for viavel ate o deposito artificial
					conjVisitados = (it->vertVisitados | verticesCoding[i]);
					custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice+ajuste, i+ajuste);
					numVisitados = it->numVertVisitados+1;

					//Antes de executar o teste de dominancia deste label, verifico se existem vertices adjacentes 
					//para os quais ele eh unreachable, colocando 1 em sua posicao e incrementando numVisitados
					for (int z = 1; z <= numCommodities; ++z){
						if (((conjVisitados & verticesCoding[z]) == 0) && 
							((carga + g->getPesoCommodity(z+ajuste) > cargaMaxima) || (tempo + g->getCustoAresta(i+ajuste, z+ajuste) > janelaTempo[z][1]))){
							conjVisitados = (conjVisitados | verticesCoding[z]);
							++numVisitados;
						}
					}
					
					//Se os valores forem nao dominados com relacao aos que estao
					//inseridos no vertice, entao insere o novo label
					//Obs.: Um Label que seja igual a outro tbm eh considerado dominado
					if (!verificaLabelDominadoEDomina(g, i, custo, carga, tempo, numVisitados, conjVisitados)){
						aux = new LabelTW(conjVisitados, numVisitados, i, it->ultimoVertice, carga, tempo, custo, NULL);
						aux->pai = it;
						if (vetorLabels[i].posAtual == NULL) vetorLabels[i].posAtual = aux;
						vetorLabels[i].calda->prox = aux;
						vetorLabels[i].calda = aux;
						
						custoAteDestino = custo + g->getCustoArestaDual(i+ajuste, depositoArtificial);
						if ((custoAteDestino < (valorSubtrair - 0.005)) && (custoAteDestino < menorCustoObtido)){
							menorCustoObtido = custoAteDestino;
							menorLabelObtido = aux;
							encontrouRotaNegativa = true;
						}
					}
				}
			}

			if ((vetorLabels[i].posAtual != NULL) && (vetorLabels[i].posAtual->custoDual < menorCustoAtual)){
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}
	}

	//deleta a memoria alocada para os vetores auxiliares
	for (int i = 0; i <= numCommodities; ++i){
		delete [] janelaTempo[i];
	}
	delete [] janelaTempo;
	delete [] iAteDeposito;
	delete [] tempoServico;

	if (encontrouRotaNegativa){
		return 1;
	}else{
		return -1;
	}
}


bool ElementarTW::verificaLabelDominadoEDomina(Grafo* g, int ult, float custo, int carga, float tempo, int numVisit, unsigned long long int visit){	
	ptrLabelTW aux, anterior; //sera necessario na exclusao do label apontado por 'it' (caso este seja dominado)
	bool itUpdate;
	ptrLabelTW it = vetorLabels[ult].cabeca;

	int domCarga, domCusto, domTempo, domVisit1, domVisit2, tmp;
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

		//Verifico se o tempo deste novo label domina o que ja estava em vLabel[ult]
		if (tempo < it->tempoAcumulado){
			domTempo = 1;
		}else if (tempo > it->tempoAcumulado){
			domTempo = -1;
		}else{
			domTempo = 0;
		}

		//Verifico se o custo deste novo label domina o label armazenado em vLabel[ult]
		if (custo < it->custoDual){
			domCusto = 1;
		}else if (custo > it->custoDual){
			domCusto = -1;
		}else{
			domCusto = 0;
		}
		
		//Verifica se para todo vertice i, novoLabel->visit[i] <= labelArmazenado->visit[i] 
		//(sendo que 0 = nao visitado e 1 = visitado)
		itUpdate = true;
		domVisit1 = (numVisit < it->numVertVisitados) ? 1 : 0;
		domVisit2 = (numVisit > it->numVertVisitados) ? 1 : 0;

		if (domVisit1 == 1){

			if ((domCarga >= 0) && (domTempo >= 0) && (domCusto >= 0) && ((domCarga + domCusto + domTempo) > 0)){//Label DOMINA PARCIALMENTE

				for (int j = 1; j <= numCommodities; ++j){
					v1 = visit & verticesCoding[j];
					v2 = it->vertVisitados & verticesCoding[j];
					if (v2 < v1){
						domVisit2 = 1;
						break;
					}
				}
				if (domVisit2 == 0){ //Novo Label DOMINA TOTALMENTE 

					//o novo label domina o label apontado por 'it', que deve ser excluido
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

		}else if (domVisit2 == 1){

			if ((domCarga <= 0) && (domTempo <= 0) && (domCusto <= 0)){ //Label apontado por 'it' DOMINA PARCIALMENTE o novo Label

				for (int j = 1; j <= numCommodities; ++j){
					v1 = visit & verticesCoding[j];
					v2 = it->vertVisitados & verticesCoding[j];
					if (v1 < v2){
						domVisit1 = 1;
						break;
					}
				}
				if (domVisit1 == 0) return true; //DOMINANCIA TOTAL do novo Label pelo label apontado por 'it'
			}

		}else{
		
			for (int j = 1; j <= numCommodities; ++j){
				v1 = visit & verticesCoding[j];
				v2 = it->vertVisitados & verticesCoding[j];
				if (v1 < v2){
					domVisit1 = 1;
				}else if (v2 < v1){
					domVisit2 = 1;
				}
			}
			
			if ((domCarga <= 0) && (domTempo <= 0) && (domCusto <= 0) && (domVisit1 == 0)){
				return true;
			}else if (((domCarga >= 0) && (domTempo >= 0) && (domCusto >= 0) && (domVisit2 == 0) && ((domCarga + domCusto + domTempo + domVisit1) > 0))){
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


Rota** ElementarTW::getRotaCustoMinimo(Grafo* g, float percentual){
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
	if (limiteAceitacao > (valorSubtrair - 0.005)){
		limiteAceitacao = valorSubtrair - 0.005;
	}
	
	int numRotasRetornadas = 0;
	//Neste momento, verifica todos os labels nao dominados encontrados para alocar a quantidade de rotas a serem retornadas
	ptrLabelTW tmp, aux;
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
		numRotasRetornadas = 0;
		for (int i = 1; i <= numCommodities; ++i){
			aux = vetorLabels[i].cabeca;
			while (aux != NULL){
				custoR = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice+ajuste, depositoArtif);
				if (custoR < limiteAceitacao){
					r[numRotasRetornadas] = new Rota();
					r[numRotasRetornadas]->inserirVerticeFim(depositoArtif);
					tmp = aux;
					while (tmp->pai != NULL){
						r[numRotasRetornadas]->inserirVerticeInicio(posicaoOriginal[tmp->ultimoVertice+ajuste]);
						tmp = tmp->pai;
					}
					r[numRotasRetornadas]->inserirVerticeInicio(posicaoOriginal[tmp->ultimoVertice+ajuste]);
					r[numRotasRetornadas]->inserirVerticeInicio(0);
					r[numRotasRetornadas]->setCustoReduzido(custoR);
					r[numRotasRetornadas]->setCusto(g);
					++numRotasRetornadas;
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

