#include "ElementarRD.h"

ElementarRD::ElementarRD(Grafo* g, float valorSub) : valorSubtrair(valorSub), menorLabelObtido(NULL), menorCustoObtido(MAIS_INFINITO){
	numCommodities = g->getNumCmdt();

	//Aloca memoria para vetorLabels (que tera os mesmos labels de listaLabels, mas será utilizado por questoes de eficiencia)
	vetorLabels = new vLabelRD[2*numCommodities+1];

	//Aloca e inicializa o vetor que armazenara a codificacao dos vertices no sistema unsigned long long int
	verticesCoding = new unsigned long long int [numCommodities+1];
	verticesCoding[0] = 0;
	for (int i = 1; i <= numCommodities; ++i)
	{
		verticesCoding[i] = pow (2, i);
	}
	
	lixo = NULL;
}


ElementarRD::~ElementarRD(){
	ptrLabelRD p, aux;
	//Exclui a memoria ainda alocada
	 //vetorLabels[0] <=> listaLabels (aponta para todos os labels, de todos os vertices)
	for (int i = 1; i <= 2*numCommodities; ++i)
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


int ElementarRD::calculaCaminhoElementar(Grafo* g, int classe, int tempo){
	int depositoArtificial = g->getNumVertices()-1;
	int cargaMaxima = g->getCapacVeiculo(classe);
	int carga, numVisitados1, numVisitados2, menorIndice;
	unsigned long long int conjVisitados1, conjVisitados2;
	float custo, custoAteDestino;
	ptrLabelRD aux, it;

	//insiro os labels relativos ao caminho de 0 aos vertices adjacentes para depois processar 
	//os demais labels existentes em vetorLabels, armazenando apenas aqueles que chegam em 0' nao-dominados
	for (int i = 1; i <= numCommodities; ++i)
	{
		carga = g->getPesoCommodity(i);
		aux = new LabelRD(verticesCoding[i], verticesCoding[i], 1, 1, i, 0, carga, g->getCustoArestaDual(0, i), NULL);
		aux->pai = NULL;
		vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;
		vetorLabels[numCommodities+i].cabeca = vetorLabels[numCommodities+i].calda = vetorLabels[numCommodities+i].posAtual = NULL;
	}

	//obtem todos os labels nao-dominados para visitar os FORNECEDORES
	menorIndice = 1;
	while(menorIndice > 0)
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;
		
		for (int i = 1; i <= numCommodities; ++i)
		{
			carga = it->cargaAcumulada + g->getPesoCommodity(i);
			if ( ( ( it->vertVisitados2 & verticesCoding[i] ) == 0 ) && ( carga <= cargaMaxima ) )
			{
				custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice, i);
				conjVisitados1 = (it->vertVisitados1 | verticesCoding[i]);
				numVisitados1 = it->numVertVisitados1+1;

				//Marca aqueles vertices 'unreachable' com relacao ao label que esta sendo criado
				conjVisitados2 = (it->vertVisitados2 | verticesCoding[i]);
				numVisitados2 = it->numVertVisitados2+1;
				for (int z = 1; z <= numCommodities; ++z)
				{
					if ( ( ( conjVisitados2 & verticesCoding[z] ) == 0 )  && ( carga + g->getPesoCommodity(z) > cargaMaxima ) )
					{
						conjVisitados2 = (conjVisitados2 | verticesCoding[z]);
						++numVisitados2;
					}
				}

				//Se os valores forem nao dominados com relacao aos que estao inseridos no vertice, entao insere o novo label
				if ( !verificaLabelDominadoEDominaF( g, conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, i, custo ) )
				{
					aux = new LabelRD(conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, i, it->ultimoVertice, carga, custo, NULL);
					aux->pai = it;
					if (vetorLabels[i].posAtual == NULL) vetorLabels[i].posAtual = aux;
					vetorLabels[i].calda->prox = aux;
					vetorLabels[i].calda = aux;
				}
			}

			if ( ( vetorLabels[i].posAtual != NULL ) && ( vetorLabels[i].posAtual->custoDual < menorCustoAtual ) )
			{
				menorIndice = i;
				menorCustoAtual = vetorLabels[i].posAtual->custoDual;
			}
		}
		if (( time(0) - tempo ) > 10800 ) return -1;
	}
	
	//baseando-se nos labels nao-dominados para os fornecedores, obtem os labels nao dominados para os CONSUMIDORES
	for ( int i = 1; i <= numCommodities; ++i)
	{
		it = vetorLabels[i].cabeca;
		while( it != NULL )
		{
			for (int j = numCommodities+1; j < depositoArtificial; ++j )
			{
				if ( ( it->vertVisitados1 & verticesCoding[j-numCommodities] ) > 0 ) //estende o label para todos os consumidores possiveis
				{
					numVisitados1 = 1;
					conjVisitados1 = verticesCoding[j-numCommodities];
					conjVisitados2 = it->vertVisitados1;
					numVisitados2 = it->numVertVisitados1;
					custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice, j);

					if ( !verificaLabelDominadoEDominaC( g, conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, j, custo ) )
					{
						aux = new LabelRD(conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, j, it->ultimoVertice, 0, custo, NULL);
						aux->pai = it;
						if ( vetorLabels[j].posAtual == NULL ) vetorLabels[j].posAtual = aux;
						if ( vetorLabels[j].cabeca == NULL ) vetorLabels[j].cabeca = aux;
						if ( vetorLabels[j].calda == NULL ) vetorLabels[j].calda = aux;
						else
						{
							vetorLabels[j].calda->prox = aux;
							vetorLabels[j].calda = aux;
						}
						
						if ( conjVisitados1 == conjVisitados2 ) //Todos os delivery vertices foram visitados
						{
							custoAteDestino = custo + g->getCustoArestaDual(j, depositoArtificial);
							if ( ( custoAteDestino < ( valorSubtrair - 0.001 ) ) && ( custoAteDestino < menorCustoObtido ) )
							{
								menorCustoObtido = custoAteDestino;
								menorLabelObtido = aux;
							}
						}
					}
				}
			}
			it = it->prox;
		}
		if (( time(0) - tempo ) > 10800 ) return -1;
	}

	//procura pelos labels nao-dominados considerando apenas os CONSUMIDORES
	menorIndice = numCommodities+1;
	while(menorIndice > 0)
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		for ( int i = numCommodities+1; i < depositoArtificial; ++i )
		{
			if ( ( ( it->vertVisitados2 & verticesCoding[i-numCommodities] ) > 0 ) && ( ( it->vertVisitados1 & verticesCoding[i-numCommodities] ) == 0 ) )
			{
				conjVisitados1 = (it->vertVisitados1 | verticesCoding[i-numCommodities]);
				numVisitados1 = it->numVertVisitados1 + 1;
				conjVisitados2 = it->vertVisitados2;
				numVisitados2 = it->numVertVisitados2;
				custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice, i);

				if ( !verificaLabelDominadoEDominaC( g, conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, i, custo ) )
				{
					aux = new LabelRD(conjVisitados1, conjVisitados2, numVisitados1, numVisitados2, i, it->ultimoVertice, 0, custo, NULL);
					aux->pai = it;
					if ( vetorLabels[i].posAtual == NULL ) vetorLabels[i].posAtual = aux;
					if ( vetorLabels[i].cabeca == NULL ) vetorLabels[i].cabeca = aux;
					if ( vetorLabels[i].calda == NULL ) vetorLabels[i].calda = aux;
					else
					{
						vetorLabels[i].calda->prox = aux;
						vetorLabels[i].calda = aux;
					}
					
					if ( conjVisitados1 == conjVisitados2 ) //Todos os delivery vertices foram visitados
					{
						custoAteDestino = custo + g->getCustoArestaDual(i, depositoArtificial);
						if ( ( custoAteDestino < ( valorSubtrair - 0.001 ) ) && ( custoAteDestino < menorCustoObtido ) )
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
		if (( time(0) - tempo ) > 10800 ) return -1;
	}

	if (menorLabelObtido != NULL)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}


bool ElementarRD::verificaLabelDominadoEDominaF(Grafo* g, unsigned long long int visit1, unsigned long long int visit2, int numVisit1, int numVisit2, int ult, float custo ){
	int domCusto;
	bool itUpdate, domVisit;
	ptrLabelRD aux, anterior; //necessario na exclusao do label apontado por 'it' (caso seja dominado)
	ptrLabelRD it = vetorLabels[ult].cabeca;

	while (it != NULL) //Verifica a dominancia para todos os labels (o novo label so sera inserido se nao houver nenhum q o domina - ou seja igual)
	{
		//Verifico se o custo deste novo label domina o label armazenado em vLabel[ult]
		if (custo < it->custoDual) domCusto = 1;
		else if (custo > it->custoDual) domCusto = -1;
		else domCusto = 0;

		itUpdate = true;
		if ( ( numVisit2 < it->numVertVisitados2 ) && ( domCusto >= 0 ) ) //Novo Label DOMINA parcialmente
		{
			domVisit = true;
			for ( int j = 1; j <= numCommodities; ++j )
			{
				if ( ( it->vertVisitados2 & verticesCoding[j] ) < ( visit2 & verticesCoding[j] ) )
				{
					domVisit = false;
					break;
				}
			}
			if ( domVisit )
			{
				if ( numVisit1 < it->numVertVisitados1 )
				{
					for ( int j = 1; j <= numCommodities; ++j )
					{
						if ( ( it->vertVisitados1 & verticesCoding[j] ) < ( visit1 & verticesCoding[j] ) )
						{
							domVisit = false;
							break;
						}
					}
				}
				else if ( it->vertVisitados1 != visit1 ) domVisit = false;

				if ( domVisit ) //Novo Label domina TOTAL o label armazenado (que sera excluido)
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
					//delete aux;
					
					aux->prox = lixo;
					lixo = aux;
				}
			}
		}
		else if ( ( numVisit2 > it->numVertVisitados2 ) && ( domCusto <= 0 ) ) //Label armazenado DOMINA parcialmente
		{
			domVisit = true;
			for ( int j = 1; j <= numCommodities; ++j )
			{
				if ( ( visit2 & verticesCoding[j] ) < ( it->vertVisitados2 & verticesCoding[j] ) )
				{
					domVisit = false;
					break;
				}
			}
			if ( domVisit )
			{
				if ( it->numVertVisitados1 < numVisit1 )
				{
					for ( int j = 1; j <= numCommodities; ++j )
					{
						if ( ( visit1 & verticesCoding[j] ) < ( it->vertVisitados1 & verticesCoding[j] ) )
						{
							domVisit = false;
							break;
						}
					}
				}
				else if ( it->vertVisitados1 != visit1 ) domVisit = false;
				
				if ( domVisit ) return true; //Label armazenado domina TOTAL o novo label
			}
		}
		else if ( visit2 == it->vertVisitados2 )
		{
			if ( domCusto <= 0 ) 
			{
				domVisit = true;
				if ( it->numVertVisitados1 < numVisit1 )
				{
					for ( int j = 1; j <= numCommodities; ++j )
					{
						if ( ( visit1 & verticesCoding[j] ) < ( it->vertVisitados1 & verticesCoding[j] ) )
						{
							domVisit = false;
							break;
						}
					}
				}
				else if ( it->vertVisitados1 != visit1 ) domVisit = false;
				
				if ( domVisit ) return true; //Label armazenado domina TOTAL o novo label
			}
			if ( domCusto >= 0 )
			{
				domVisit = true;
				if ( numVisit1 < it->numVertVisitados1 )
				{
					for ( int j = 1; j <= numCommodities; ++j )
					{
						if ( ( it->vertVisitados1 & verticesCoding[j] ) < ( visit1 & verticesCoding[j] ) )
						{
							domVisit = false;
							break;
						}
					}
				}
				else if ( it->vertVisitados1 != visit1 ) domVisit = false;

				if ( domVisit ) //Novo Label domina TOTAL o label armazenado (que sera excluido)
				{
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
					//delete aux;
					
					aux->prox = lixo;
					lixo = aux;
				}
			}
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


bool ElementarRD::verificaLabelDominadoEDominaC(Grafo* g, unsigned long long int visit1, unsigned long long int visit2, 
												int numVisit1, int numVisit2, int ult, float custo ){
	int domCusto;
	bool itUpdate, domVisit;
	ptrLabelRD aux, anterior = NULL; //necessario na exclusao do label apontado por 'it' (caso seja dominado)
	ptrLabelRD it = vetorLabels[ult].cabeca;

	while (it != NULL) //Verifica a dominancia para todos os labels (o novo label so sera inserido se nao houver nenhum q o domina - ou seja igual)
	{
		if (custo < it->custoDual) domCusto = 1;
		else if (custo > it->custoDual) domCusto = -1;
		else domCusto = 0;

		itUpdate = true;
		if ( ( ( numVisit2 - numVisit1 ) < ( it->numVertVisitados2 - it->numVertVisitados1 ) ) && ( domCusto >= 0 ) ) //Novo label domina PARCIALMENTE label armazenado
		{ 
			domVisit = true;
			for ( int j = 1; j <= numCommodities; ++j )
			{
				if ( ( ( it->vertVisitados2 & ~it->vertVisitados1 ) & verticesCoding[j] ) < ( ( visit2 & ~visit1 ) & verticesCoding[j] ) )
				{
					domVisit = false;
					break;
				}
			}
			if ( domVisit ) ////Novo label domina TOTALMENTE label armazenado (que sera excluido)
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
		}
		else if ( ( ( it->numVertVisitados2 - it->numVertVisitados1 ) < ( numVisit2 - numVisit1 ) ) && ( domCusto <= 0 ) ) //Label armazenado domina PARCIALMENTE novo label
		{
			domVisit = true;
			for ( int j = 1; j <= numCommodities; ++j )
			{
				if ( ( ( visit2 & ~visit1 ) & verticesCoding[j] ) < ( ( it->vertVisitados2 & ~it->vertVisitados1 ) & verticesCoding[j] ) )
				{
					domVisit = false;
					break;
				}
			}
			if ( domVisit ) return true; //Label armazenado domina TOTALMENTE o novo label
		}
		else if ( ( visit2 & ~visit1 ) == ( it->vertVisitados2 & ~it->vertVisitados1 ) )
		{
			if ( domCusto <= 0 )
			{
				return true;
			}
			else if ( domCusto >= 0 )
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

Rota** ElementarRD::getRotaCustoMinimo(Grafo* g, int classe, float percentual){
	float limiteAceitacao, custoR;
	int depositoArtif = g->getNumVertices()-1;

	//utiliza o percentual para obter rotas negativas (podem existir varias, pois nao parou na primeira)
	if (menorCustoObtido < 0)
	{
		limiteAceitacao = percentual * menorCustoObtido;
	}
	else
	{
		limiteAceitacao = (2 - percentual) * menorCustoObtido;
	}

	if (limiteAceitacao > (valorSubtrair - 0.001))
	{
		limiteAceitacao = valorSubtrair - 0.001;
	}
	
	int numRotasRetornadas = 0;
	//Neste momento, verifica todos os labels nao dominados encontrados para alocar a quantidade de rotas a serem retornadas
	ptrLabelRD tmp, aux, p;
	for (int i = numCommodities+1; i < depositoArtif; ++i)
	{
		aux = vetorLabels[i].cabeca;
		while (aux != NULL)
		{
			if ( ( aux->vertVisitados1 == aux->vertVisitados2 ) && ( ( aux->custoDual + g->getCustoArestaDual(i, depositoArtif)) < limiteAceitacao ) )
			{
				++numRotasRetornadas;
			}
			aux = aux->prox;
		}
	}

	if (numRotasRetornadas > 0)
	{
		Rota** r;
		r = new Rota*[numRotasRetornadas+1];
		numRotasRetornadas = 0;
		for (int i = numCommodities+1; i < depositoArtif; ++i)
		{
			aux = vetorLabels[i].cabeca;
			while (aux != NULL)
			{
				custoR = aux->custoDual + g->getCustoArestaDual(aux->ultimoVertice, depositoArtif);
				if ( ( aux->vertVisitados1 == aux->vertVisitados2 ) && ( custoR < limiteAceitacao ) )
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
					r[numRotasRetornadas]->setCusto(g, classe);
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


void ElementarRD::imprimeLabel(ptrLabelRD aux){
	char* bin1 = decToBin(aux->vertVisitados1);
	char* bin2 = decToBin(aux->vertVisitados2);
	cout<< "[" << bin1 << "-" << bin2 << ", '" << aux->numVertVisitados1 << "'-'" << aux->numVertVisitados2 << "', " << aux->verticeAntecessor
			 << "(a), "<< aux->ultimoVertice << "(u), " << aux->cargaAcumulada << "(c), " << aux->custoDual << "($)]" << endl;
	delete [] bin1;
	delete [] bin2;
}

char* ElementarRD::decToBin(unsigned long long int dec){
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
