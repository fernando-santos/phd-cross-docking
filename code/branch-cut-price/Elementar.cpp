#include "Elementar.h"

Elementar::Elementar(Grafo* g, bool camForn, float valorSub) : valorSubtrair(valorSub),	menorCustoObtido(MAIS_INFINITO){
	numCommodities = g->getNumCmdt();
	ajuste = camForn ? 0 : numCommodities;
	
	//Aloca memoria para vetorLabels (que tera os mesmos labels de listaLabels, mas será utilizado por questoes de eficiencia)
	vetorLabels = new vLabel[numCommodities+1];

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


Elementar::~Elementar(){
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
	delete [] verticesCodingCompl;
}


int Elementar::calculaCaminhoElementar(Grafo* g, bool fullTime){
	int depositoArtificial = g->getNumVertices()-1;
	int cargaMaxima = g->getCapacVeiculo();
	int carga, numVisitados, menorIndice;
	float custo, custoAteDestino;
	unsigned long long int conjVisitados;
	ptrLabel aux, it;
	bool encontrouRotaNegativa = false;

	//captura o tempo para controlar o processamento do pricing
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	int hora, min, seg, micro, tempo;
	float tempoTotal, limiteTempoPricing = (float) depositoArtificial / 20;
	if (!fullTime){
		gettimeofday(&tv, &tz);
		tm=localtime(&tv.tv_sec);
		hora = tm->tm_hour;
		min = tm->tm_min;
		seg = tm->tm_sec;
		micro = tv.tv_usec;
	}
	
	//insiro os labels relativos ao caminho de 0 aos vertices adjacentes
	//para depois processar todos os labels existentes em listaLabels,
	//armazenando apenas aqueles que chegam em 0' que sejam nao-dominados
	for (int i = 1; i <= numCommodities; ++i){
		carga = g->getCargaVertice(i+ajuste);
		if (carga <= cargaMaxima){
			aux = new Label(verticesCoding[i], 1, i, 0, carga,	g->getCustoArestaDual(0, i+ajuste), NULL);
			aux->pai = NULL;
			vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;
			
			//verifica se estes primeiros labels dao origem a rotas de custo reduzido negativo
			custoAteDestino = aux->custoDual + g->getCustoArestaDual(i+ajuste, depositoArtificial);
			if ((custoAteDestino < (valorSubtrair - 0.05)) && (custoAteDestino < menorCustoObtido)){
				menorCustoObtido = custoAteDestino;
				menorLabelObtido = aux;
				encontrouRotaNegativa = true;
			}
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
			carga = it->cargaAcumulada + g->getCargaVertice(i+ajuste); 
			if (((it->vertVisitados & verticesCoding[i]) == 0) && (carga <= cargaMaxima)){
				conjVisitados = (it->vertVisitados | verticesCoding[i]);
				custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice+ajuste, i+ajuste);
				numVisitados = it->numVertVisitados+1;

				//Antes de executar o teste da dominancia deste label, verifico se existem vertices adjacentes 
				//para os quais ele eh unreachable, colocando 1 em sua posicao e incrementando numVisitados
				for (int z = 1; z <= numCommodities; ++z){
					if (((conjVisitados & verticesCoding[z]) == 0)  && (carga + g->getCargaVertice(z+ajuste) > cargaMaxima)){
						conjVisitados = (conjVisitados | verticesCoding[z]);
						++numVisitados;
					}
				}
				
				//Se os valores forem nao dominados com relacao aos que estao
				//inseridos no vertice, entao insere o novo label
				//Obs.: Um Label que seja igual a outro tbm eh considerado dominado
				if (!verificaLabelDominadoEDomina(g, i, custo, carga, numVisitados, conjVisitados)){
					aux = new Label(conjVisitados, numVisitados, i, it->ultimoVertice, carga, custo, NULL);
					aux->pai = it;
					if (vetorLabels[i].posAtual == NULL) vetorLabels[i].posAtual = aux;
					vetorLabels[i].calda->prox = aux;
					vetorLabels[i].calda = aux;
					
					custoAteDestino = custo + g->getCustoArestaDual(i+ajuste, depositoArtificial);
					if ((custoAteDestino < (valorSubtrair - 0.05)) && (custoAteDestino < menorCustoObtido)){
						menorCustoObtido = custoAteDestino;
						menorLabelObtido = aux;
						encontrouRotaNegativa = true;
					}
				}
			}

			if ((vetorLabels[i].posAtual != NULL) && (vetorLabels[i].posAtual->custoDual < menorCustoAtual)){
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}

		if (!fullTime){
			gettimeofday(&tv, &tz);
			tm=localtime(&tv.tv_sec);
			tempo = (tm->tm_hour - hora) * 3600;
			tempo += (tm->tm_min - min) * 60;
			tempo += tm->tm_sec - seg;
			tempo += tempo * 1000000;
			tempo += tv.tv_usec - micro;
			tempoTotal = tempo;
			tempoTotal /= 1000000;
			if (tempoTotal  > limiteTempoPricing){
				if (encontrouRotaNegativa){
					return 1;
				}else{
					return 0;
				}
			}
		}
	}
	
	if (encontrouRotaNegativa){
		return 1;
	}else{
		return -1;
	}
}


bool Elementar::verificaLabelDominadoEDomina(Grafo* g, int ult, float custo, int carga, int numVisit, unsigned long long int visit){
	ptrLabel aux, anterior; //sera necessario na exclusao do label apontado por 'it' (caso este seja dominado)
	bool itUpdate;
	ptrLabel it = vetorLabels[ult].cabeca;

	int domCarga, domCusto, domVisit1, domVisit2, tmp;
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
		
		//Verifica se para todo vertice i, novoLabel->visit[i] <= labelArmazenado->visit[i] 
		//(sendo que 0 = nao visitado e 1 = visitado)
		itUpdate = true;
		domVisit1 = (numVisit < it->numVertVisitados) ? 1 : 0;
		domVisit2 = (numVisit > it->numVertVisitados) ? 1 : 0;

		if (domVisit1 == 1){
			
			if ((domCarga >= 0) && (domCusto >= 0) && ((domCarga + domCusto + domVisit1) > 0)){//Novo Label DOMINA

				for (int j = 1; j <= numCommodities; ++j){
					v1 = visit & verticesCoding[j];
					v2 = it->vertVisitados & verticesCoding[j];
					if (v2 < v1){
						domVisit2 = 1;
						break;
					}
				}

				if (domVisit2 == 0){
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
		}else if (domVisit2 == 1){
			
			if ((domCarga <= 0) && (domCusto <= 0)){

				for (int j = 1; j <= numCommodities; ++j){
					v1 = visit & verticesCoding[j];
					v2 = it->vertVisitados & verticesCoding[j];
					if (v1 < v2){
						domVisit1 = 1;
						break;
					}
				}
				if (domVisit1 == 0) return true;
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
			
			if ((domCarga <= 0) && (domCusto <= 0) && (domVisit1 == 0)){
				return true;
			}else if (((domCarga >= 0) && (domCusto >= 0) && (domVisit2 == 0) && ((domCarga + domCusto + domVisit1) > 0))){
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


Rota** Elementar::getRotaCustoMinimo(Grafo* g, float percentual){
	float limiteAceitacao, custoR;
	int depositoArtif = g->getNumVertices()-1;

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
						r[numRotasRetornadas]->inserirVerticeInicio(tmp->ultimoVertice+ajuste);
						tmp = tmp->pai;
					}
					r[numRotasRetornadas]->inserirVerticeInicio(tmp->ultimoVertice+ajuste);
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


void Elementar::imprimeLabel(ptrLabel aux){
	char* bin = decToBin(aux->vertVisitados);
	std::cout<< "[" << bin << "'" << aux->numVertVisitados << "', " << aux->verticeAntecessor << "(a), "
				<< aux->ultimoVertice << "(u), " << aux->cargaAcumulada << "(c), " << aux->custoDual << "($)]" << endl;
	delete [] bin;
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
