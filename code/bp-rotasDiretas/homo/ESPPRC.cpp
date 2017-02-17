#include "ESPPRC.h"

ESPPRC::ESPPRC(Grafo* g, float* vetXi, float valorSubtrair) : vetorXi(vetXi), alfa(valorSubtrair), menorCustoObtido(MAIS_INFINITO), menorLabelObtido(NULL){
	lixo = NULL;
	nRequests = g->getNumReqsDual();
	indiceVertDual = g->getIndiceVerticesDual();
	nVertices = 2*nRequests;
	vetorLabels = new vLabel[nVertices+1];

	//Aloca e inicializa os vetores que armazenaram a codificacao dos vertices no sistema 'unsigned long long int'
	verticesCoding = new unsigned long long int [nRequests+1];
	verticesCoding[0] = 0;
	for (int i = 1; i <= nRequests; ++i)
	{
		verticesCoding[i] = pow (2, i);
	}
}

ESPPRC::~ESPPRC(){
	ptrLabel p, aux;
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

int ESPPRC::calculaCaminhoElementar(Grafo* g){
	bool dominado;
	ptrLabel aux, it;
	float custo, custoAteDestino;
	int depositoArtificial = nVertices+1;
	int cargaMaxima = g->getCapacVeiculo();
	int carga, menorIndice, numVisitF, numVisitC;
	unsigned long long int conjVisitadosF, conjVisitadosF2, conjVisitadosC, repetido;

	//Cria os primeiros labels, com caminhos de 0 a todos os FORNECEDORES
	for (int i = 1; i <= nRequests; ++i)
	{
		carga = g->getCargaVertice(indiceVertDual[i]);
		if ( carga <= cargaMaxima )
		{
			aux = new Label(verticesCoding[i], verticesCoding[i], 0, 1, 0, i, 0, carga, g->getCustoArestaDual(0,i), NULL, NULL);
			vetorLabels[i].cabeca = vetorLabels[i].calda = vetorLabels[i].posAtual = aux;
			vetorLabels[nRequests+i].cabeca = vetorLabels[nRequests+i].calda = vetorLabels[nRequests+i].posAtual = NULL;
		}
	}

	menorIndice = 1;
	while(menorIndice > 0)
	{
		it = vetorLabels[menorIndice].posAtual;
		vetorLabels[menorIndice].posAtual = it->prox;
		menorCustoAtual = MAIS_INFINITO;
		menorIndice = 0;

		//Uma vez com o label 'it', verifica-se todos os adjacentes de 'it->ultimo' em busca de outros labels nao dominados.
		//OBS1.: Labels de FORNECEDORES podem ser estendidos para FORNECEDORES ou CONSUMIDORES
		//OBS2.: Labels de CONSUMIDORES podem ser estendidos para CONSUMIDORES ou DEPOSITO ARTIFICIAL
		for ( int i = 1; i <= nVertices; ++i )
		{
			if ( g->existeArestaDual(it->ultimoVertice, i) )
			{
				carga = ( ( it->ultimoVertice <= nRequests ) && ( i > nRequests ) ) ? g->getCargaVertice(indiceVertDual[i]) : it->cargaAcumulada + g->getCargaVertice(indiceVertDual[i]);
				repetido = (i > nRequests) ? (it->vertVisitadosC & verticesCoding[i-nRequests]) : (it->vertVisitadosF & verticesCoding[i]);

				if ((repetido == 0) && (carga <= cargaMaxima))
				{
					custo = it->custoDual + g->getCustoArestaDual(it->ultimoVertice, i);
					if (i <= nRequests)
					{
						conjVisitadosF = (it->vertVisitadosF | verticesCoding[i]);
						conjVisitadosF2 = (it->vertVisitadosF2 | verticesCoding[i]);
						numVisitF = it->numVisitadosF+1;
						conjVisitadosC =  0;
						numVisitC = 0;

/*						//Antes de executar o teste da dominancia, marco aqueles vertices 'unreachable'
						for (int j = 1; j <= nRequests; ++j)
						{
							if ( ( ( conjVisitadosF & verticesCoding[j] ) == 0 ) && ( ( carga + g->getCargaVertice(indiceVertDual[j]) ) > cargaMaxima ) )
							{
								conjVisitadosF = (conjVisitadosF | verticesCoding[j]);
								++numVisitF;
							}
						}
*/					}
					else
					{
						conjVisitadosF = it->vertVisitadosF;
						conjVisitadosF2 = it->vertVisitadosF2;
						numVisitF = it->numVisitadosF;
						conjVisitadosC =  (it->vertVisitadosC | verticesCoding[i-nRequests]);
						numVisitC = it->numVisitadosC+1;
						if ( ( conjVisitadosF2 & verticesCoding[i-nRequests] ) == 0 ) custo += vetorXi[indiceVertDual[i-nRequests]];

						//Antes de executar o teste da dominancia, marco aqueles vertices 'unreachable'
						for (int j = 1; j <= nRequests; ++j)
						{
							if ( ( ( conjVisitadosC & verticesCoding[j] ) == 0 ) && ( ( carga + g->getCargaVertice(indiceVertDual[j+nRequests]) ) > cargaMaxima ) )
							{
								conjVisitadosC = (conjVisitadosC | verticesCoding[j]);
								++numVisitC;
							}
						}
					}

					if ( !verificaLabelDominadoEDomina(g, conjVisitadosF, conjVisitadosF2, conjVisitadosC, numVisitF, numVisitC, i, custo) )
					{
						aux = new Label(conjVisitadosF, conjVisitadosF2, conjVisitadosC, numVisitF, numVisitC, i, it->ultimoVertice, carga, custo, it, NULL);
						if ( vetorLabels[i].posAtual == NULL ) vetorLabels[i].posAtual = aux;
						if ( vetorLabels[i].cabeca == NULL ) vetorLabels[i].cabeca = aux;
						if ( vetorLabels[i].calda == NULL ) vetorLabels[i].calda = aux;
						else
						{
							vetorLabels[i].calda->prox = aux;
							vetorLabels[i].calda = aux;
						}

						if (i > nRequests)
						{
							custoAteDestino = custo + g->getCustoArestaDual(i, depositoArtificial);
							if ((custoAteDestino < (alfa - 0.01)) && (custoAteDestino < menorCustoObtido))
							{
								menorCustoObtido = custoAteDestino;
								menorLabelObtido = aux;
							}
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

	if ( menorLabelObtido != NULL )	return 1;
	else return -1;
}

bool ESPPRC::verificaLabelDominadoEDomina(Grafo* g, unsigned long long int visitF, unsigned long long int visitF2, unsigned long long int visitC, 
											int numVisitF, int numVisitC, int ult, float custo){
	bool itUpdate, domVisit;
	ptrLabel aux, anterior = NULL;
	ptrLabel it = vetorLabels[ult].cabeca;
	float possivelCustoTroca, possivelCustoTroca2;

	while (it != NULL)
	{
		itUpdate = true;
		if ( ult <= nRequests )
		{
			if ( ( it->numVisitadosF < numVisitF ) && ( it->custoDual <= custo ) )
			{
				domVisit = true;
				possivelCustoTroca = 0;
				for ( int i = 1; i <= nRequests; ++i )
				{
					if ( ( it->vertVisitadosF & verticesCoding[i] ) > ( visitF & verticesCoding[i] ) )
					{
						domVisit = false;
						break;
					}
					else if ( ( it->vertVisitadosF2 & verticesCoding[i] ) < ( visitF2 & verticesCoding[i] ) )
					{
						possivelCustoTroca += vetorXi[indiceVertDual[i]];
					}
				}
				if ( ( domVisit ) && ( ( it->custoDual + possivelCustoTroca ) <= custo ) ) return true; //novo label dominado
			}
			else if ( ( numVisitF < it->numVisitadosF ) && ( custo <= it->custoDual ) )
			{
				domVisit = true;
				possivelCustoTroca = 0;
				for ( int i = 1; i <= nRequests; ++i )
				{
					if ( ( visitF & verticesCoding[i] ) > ( it->vertVisitadosF & verticesCoding[i] ) )
					{
						domVisit = false;
						break;
					}
					else if ( ( visitF2 & verticesCoding[i] ) < ( it->vertVisitadosF2 & verticesCoding[i] ) )
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
			else if ( it->vertVisitadosF == visitF )
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
		}
		else
		{
			if ( ( it->numVisitadosC < numVisitC ) && ( it->custoDual <= custo ) )
			{
				domVisit = true;
				possivelCustoTroca = 0;
				for ( int i = 1; i <= nRequests; ++i )
				{
					if ( ( it->vertVisitadosC & verticesCoding[i] ) > ( visitC & verticesCoding[i] ) )
					{
						domVisit = false;
						break;
					}
					else if ( ( ( it->vertVisitadosF2 & verticesCoding[i] ) < ( visitF2 & verticesCoding[i] ) ) && ( ( it->vertVisitadosC & verticesCoding[i] ) == 0 ) )
					{
						possivelCustoTroca += vetorXi[indiceVertDual[i]];
					}
				}
				if ( ( domVisit ) && ( ( it->custoDual + possivelCustoTroca ) <= custo ) ) return true; //novo label dominado
			}
			else if ( ( numVisitC < it->numVisitadosC ) && ( custo <= it->custoDual ) )
			{
				domVisit = true;
				possivelCustoTroca = 0;
				for ( int i = 1; i <= nRequests; ++i )
				{
					if ( ( visitC & verticesCoding[i] ) > ( it->vertVisitadosC & verticesCoding[i] ) )
					{
						domVisit = false;
						break;
					}
					else if ( ( ( visitF2 & verticesCoding[i] ) < ( it->vertVisitadosF2 & verticesCoding[i] ) ) && ( ( visitC & verticesCoding[i] ) == 0 ) )
					{
						possivelCustoTroca += vetorXi[indiceVertDual[i]];
					}
				}
				if ( ( domVisit ) && ( ( custo + possivelCustoTroca ) <= it->custoDual ) ) //o novo label domina o armazenado que deve ser excluido
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
			else if ( it->vertVisitadosC == visitC )
			{
				possivelCustoTroca = 0;
				possivelCustoTroca2 = 0;
				for ( int i = 1; i <= nRequests; ++i )
				{
					if ( ( visitC & verticesCoding[i] ) == 0 )
					{
						if ( ( it->vertVisitadosF2 & verticesCoding[i] ) < ( visitF2 & verticesCoding[i] ) )
						{
							possivelCustoTroca += vetorXi[indiceVertDual[i]];
						}
						else if ( ( visitF2 & verticesCoding[i] ) < ( it->vertVisitadosF2 & verticesCoding[i] ) )
						{
							possivelCustoTroca2 += vetorXi[indiceVertDual[i]];
						}
					}
				}

				if ( ( it->custoDual + possivelCustoTroca ) <= custo ) return true;
				else if ( ( custo + possivelCustoTroca2 ) <= it->custoDual )
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
		}
		if ( itUpdate )
		{
			anterior = it;
			it = it->prox;
		}
	}
	return false;
}

Rota** ESPPRC::getRotaCustoMinimo(Grafo* g, float percentual){
	float limiteAceitacao, custoR, menorCusto = MAIS_INFINITO;
	int numRotasRetornadas = 0, depositoArtif = nVertices+1;
	ptrLabel tmp, aux;

	if ( menorLabelObtido != NULL )
	{
		limiteAceitacao = ( menorCustoObtido < 0 ) ? percentual*menorCustoObtido : (2-percentual)*menorCustoObtido;
		if ( limiteAceitacao > ( alfa - 0.01 ) ) limiteAceitacao = alfa - 0.01;

		for ( int i = nRequests+1; i < depositoArtif; ++i )
		{
			aux = vetorLabels[i].cabeca;
			while ( aux != NULL )
			{
				if ( ( aux->custoDual + g->getCustoArestaDual( i, depositoArtif ) ) < limiteAceitacao )
				{
					++numRotasRetornadas;
				}
				aux = aux->prox;
			}
		}

		Rota** rr = new Rota*[numRotasRetornadas+1]; numRotasRetornadas = 0;
		for ( int i = nRequests+1; i < depositoArtif; ++i )
		{
			aux = vetorLabels[i].cabeca;
			while ( aux != NULL )
			{
				custoR = aux->custoDual + g->getCustoArestaDual( i, depositoArtif );
				if ( custoR < limiteAceitacao )
				{
					rr[numRotasRetornadas] = new Rota();
					rr[numRotasRetornadas]->inserirVerticeFim(indiceVertDual[depositoArtif]);
					tmp = aux;
					while ( tmp != NULL )
					{
						rr[numRotasRetornadas]->inserirVerticeInicio(indiceVertDual[tmp->ultimoVertice]);
						tmp = tmp->pai;
					}
					rr[numRotasRetornadas]->inserirVerticeInicio(0);
					rr[numRotasRetornadas]->setCustoReduzido(custoR);
					rr[numRotasRetornadas]->setCustoRota(g, false);
					++numRotasRetornadas;
				}
				aux = aux->prox;
			}
		}		
		rr[numRotasRetornadas] = NULL;
		return rr;
	}
	else
	{
		return NULL;
	}
}

void ESPPRC::imprimeLabel(ptrLabel aux){
	char* binF = decToBin(aux->vertVisitadosF);
	char* binC = decToBin(aux->vertVisitadosC);
	cout<< "[" << binF << "-" << binC << ", '" << aux->numVisitadosF << "'-'" << aux->numVisitadosC << "', " << aux->verticeAntecessor
			 << "(a), "<< aux->ultimoVertice << "(u), " << aux->cargaAcumulada << "(c), " << aux->custoDual << "($)]" << endl;
	delete [] binF;
	delete [] binC;
}

char* ESPPRC::decToBin(unsigned long long int dec){
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
