#include "GRASP.h"

GRASP::GRASP(Grafo* G, int nReqs, int depArtif, float* cCD, float parametroAlfa)
{
	g = G;
	nRequisicoes = nReqs;
	depArtificial = depArtif;
	alfa = parametroAlfa;
	custoTrocaCD = cCD;
	verticesForaRota = new vector<int>();
}

GRASP::~GRASP()
{
	delete verticesForaRota;
}

RotaG* GRASP::solucaoInicial()
{
	vector<int> lrc;
	RotaG* r = new RotaG();
	int* verticeVisitado = new int[depArtificial+1];
	float* estimativaCusto = new float[depArtificial+1];
	float custoVisita, limiteF, limiteC;
	float menorCustoF, menorCustoC, maiorCustoF, maiorCustoC;
	int tmp, antec, ultimoVertice, tentativas = 0, capacVeiculo = g->getCapacVeiculo();

	do
	{
		r->vertices.push_back(0);
		memset(verticeVisitado, 0, (depArtificial+1)*sizeof(int));

		//PRIMEIRA ETAPA: VERTICE QUE SAI DO DEPOSITO 0 (a LRC deve ser composta apenas por fornecedores)
		menorCustoF = MAIS_INFINITO;
		maiorCustoF = -MAIS_INFINITO;
		for (int i = 1; i <= nRequisicoes; ++i)
		{
			estimativaCusto[i] = MAIS_INFINITO;
			if ( g->existeAresta( 0 , i ) )
			{
				for (int j = 1; j <= nRequisicoes; ++j)
				{
					if ( g->existeAresta( i , j ) )
					{
						custoVisita = g->getCustoArestaDual( 0 , i ) + g->getCustoArestaDual( i , j );
						if (custoVisita < estimativaCusto[i]) estimativaCusto[i] = custoVisita;
					}
				}

				if (estimativaCusto[i] < menorCustoF) menorCustoF = estimativaCusto[i];
				if (estimativaCusto[i] > maiorCustoF) maiorCustoF = estimativaCusto[i];
			}
		}

		//lista restrita de candidatos: elementos com custo entre [menorCusto, menorCusto + alfa*(maiorCusto-menorCusto)]
		limiteF = menorCustoF + alfa*(maiorCustoF-menorCustoF);
		for (int i = 1; i <= nRequisicoes; ++i){
			if (estimativaCusto[i] <= limiteF){ //solucao deve entrar na lista restrita de candidatos
				lrc.push_back(i);
			}
		}

		//insere um dos vertices da lista restrica de candidatos na rota e parte para a proxima etapa da construcao
		ultimoVertice = lrc[rand() % lrc.size()];
		verticeVisitado[ultimoVertice] = 1;
		r->vertices.push_back(ultimoVertice);
		r->custo = g->getCustoArestaDual(0, ultimoVertice);
		r->cargaForn = g->getCargaVertice(ultimoVertice);
		lrc.clear();

		//SEGUNDA ETAPA: VERTICES QUE SAEM DE UM FORNECEDOR PARA OUTRO FORNECEDOR OU PARA UM CONSUMIDOR
		do{
			//passa pelos FORNECEDORES e marca com (-1 : unreachable) aqueles que poderiam ser visitados, mas excedem a capacidade do veiculo
			for ( int i = 1; i <= nRequisicoes; ++i )
			{
				if ( ( verticeVisitado[i] == 0 ) && ( ( r->cargaForn + g->getCargaVertice( i ) ) > capacVeiculo ) ) verticeVisitado[i] = -1;
			}

			menorCustoF = MAIS_INFINITO; menorCustoC = MAIS_INFINITO;
			maiorCustoF = -MAIS_INFINITO; maiorCustoC = -MAIS_INFINITO;
			for (int i = 1; i < depArtificial; ++i)
			{
				estimativaCusto[i] = MAIS_INFINITO;
				if ( ( verticeVisitado[i] != 0 ) || ( g->existeAresta( ultimoVertice ,  i ) == false ) ) continue;

				if ( i <= nRequisicoes )
				{
					for (int j = 1; j < depArtificial; ++j)
					{
						if ( ( verticeVisitado[j] == 0 ) && ( g->existeAresta( i , j ) ) )
						{
							custoVisita = g->getCustoArestaDual( ultimoVertice , i ) + g->getCustoAresta( i , j );
							if (custoVisita < estimativaCusto[i]) estimativaCusto[i] = custoVisita;
						}
					}
				}
				else
				{
					//inclui o custo de troca, caso i seja um consumidor e o respectivo fornecedor nao tenha sido visitado
					tmp = (verticeVisitado[i-nRequisicoes] == 1) ? 0 : custoTrocaCD[i-nRequisicoes];

					for ( int j = (nRequisicoes+1); j < depArtificial; ++j )
					{
						if ( ( verticeVisitado[j] == 0 ) && g->existeAresta( i , j ) )
						{
							custoVisita = g->getCustoArestaDual( ultimoVertice , i ) + g->getCustoArestaDual( i , j ) + tmp;
							if (custoVisita < estimativaCusto[i]) estimativaCusto[i] = custoVisita;
						}
					}
				}

				//menor e maior custos diferentes para Fornecedores e Consumidores (necessario para construir a LRC com vertices de ambos)
				if (estimativaCusto[i] < MAIS_INFINITO)
				{
					if ( i <= nRequisicoes )
					{
						if (estimativaCusto[i] < menorCustoF) menorCustoF = estimativaCusto[i];
						if (estimativaCusto[i] > maiorCustoF) maiorCustoF = estimativaCusto[i];
					}
					else
					{
						if (estimativaCusto[i] < menorCustoC) menorCustoC = estimativaCusto[i];
						if (estimativaCusto[i] > maiorCustoC) maiorCustoC = estimativaCusto[i];
					}
				}
			}

			//a LRC sera composta por vertices fornecedores e consumidores, seguindo os limites de aceitacao independentes
			limiteF = ( menorCustoF < MAIS_INFINITO ) ? ( menorCustoF + alfa * ( maiorCustoF - menorCustoF ) ) : ( -MAIS_INFINITO );
			limiteC = ( menorCustoC < MAIS_INFINITO ) ? ( menorCustoC + alfa * ( maiorCustoC - menorCustoC ) ) : ( -MAIS_INFINITO );
			for (int i = 1; i <= nRequisicoes; ++i)
			{
				if ( estimativaCusto[i] <= limiteF ) lrc.push_back(i);
			}

			for ( int i = (nRequisicoes+1); i < depArtificial; ++i )
			{
				if ( estimativaCusto[i] <= limiteC ) lrc.push_back(i);
			}
			
			if ( lrc.size() > 0 )
			{
				//insere aleatoriamente um vertice da LRC (que pode ser consumidor ou fornecedor)
				antec = ultimoVertice;
				ultimoVertice = lrc[rand() % lrc.size()];
				verticeVisitado[ultimoVertice] = 1;
				r->vertices.push_back( ultimoVertice );
				r->custo += g->getCustoArestaDual( antec , ultimoVertice );
				if (ultimoVertice <= nRequisicoes) r->cargaForn += g->getCargaVertice( ultimoVertice );
				lrc.clear();
			}
			else //significa que essa tentativa de solucao inicial deve ser abortada e uma nova tentativa iniciada
			{
				r->vertices.clear();
				break;
			}

		}while(ultimoVertice <= nRequisicoes);

		if (++tentativas > 10) 
		{
			delete r;
			delete [] verticeVisitado;
			delete [] estimativaCusto;
			return NULL;
		}

	} while( ultimoVertice <= nRequisicoes );

	//NESTE PONTO TEM-SE A ROTA COM OS FORNECEDORES E UM CONSUMIDOR: EXECUTA-SE A ULTIMA ETAPA DA CONSTRUCAO
	tentativas = 0;
	float ultimoCustoForn = r->custo;
	r->cargaCons = g->getCargaVertice( ultimoVertice );
	do{
		//inclui o custo de troca, caso o fornecedor do consumidor escolhido nao tenha sido visitado
		if ( verticeVisitado[ultimoVertice - nRequisicoes] != 1 ) r->custo += custoTrocaCD[ultimoVertice - nRequisicoes];

		//passa pelos CONSUMIDORES e marca com (-1 : unreachable) aqueles que poderiam ser visitados, mas excedem a capacidade do veiculo
		for (int i = (nRequisicoes+1); i < depArtificial; ++i)
		{
			if ( ( verticeVisitado[i] == 0 ) && ( ( r->cargaCons + g->getCargaVertice( i ) ) > capacVeiculo ) ) verticeVisitado[i] = -1;
		}

		menorCustoC = MAIS_INFINITO; maiorCustoC = -MAIS_INFINITO;
		for ( int i = (nRequisicoes+1); i < depArtificial; ++i )
		{
			estimativaCusto[i] = MAIS_INFINITO;
			if ( ( verticeVisitado[i] != 0 ) || ( g->existeAresta( ultimoVertice , i ) == false ) ) continue;

			//inclue o custo de troca, caso o respectivo fornecedor de i nao tenha sido visitado
			tmp = ( verticeVisitado[i-nRequisicoes] == 1 ) ? 0 : custoTrocaCD[i-nRequisicoes];
			for ( int j = (nRequisicoes+1); j <= depArtificial; ++j )
			{
				if ( ( verticeVisitado[j] == 0 ) && ( g->existeAresta( i , j ) ) )
				{
					custoVisita = g->getCustoArestaDual( ultimoVertice , i ) + g->getCustoArestaDual( i , j ) + tmp;
					if (custoVisita < estimativaCusto[i]) estimativaCusto[i] = custoVisita;
				}
			}

			if (estimativaCusto[i] < MAIS_INFINITO)
			{
				if ( estimativaCusto[i] < menorCustoC ) menorCustoC = estimativaCusto[i];
				if ( estimativaCusto[i] > maiorCustoC ) maiorCustoC = estimativaCusto[i];
			}
		}

		//calcula a estimativa do depositoArtificial separadamente, devido as suas particularidades
		if ( g->existeAresta( ultimoVertice , depArtificial ) )
		{
			estimativaCusto[depArtificial] = g->getCustoArestaDual( ultimoVertice , depArtificial );
			if ( estimativaCusto[depArtificial] < menorCustoC ) menorCustoC = estimativaCusto[depArtificial];
			if ( estimativaCusto[depArtificial] > maiorCustoC ) maiorCustoC = estimativaCusto[depArtificial];
		}
		else
		{
			estimativaCusto[depArtificial] = MAIS_INFINITO;
		}

		//seleciona os consumidores com melhores estimativas para entrar na lista restrita de candidatos
		limiteC = menorCustoC + alfa * ( maiorCustoC - menorCustoC );
		
		for ( int i = (nRequisicoes+1); i <= depArtificial; ++i )
		{
			if (  estimativaCusto[i] <= limiteC ) lrc.push_back(i);
		}

		if ( lrc.size() > 0 )
		{
			//insere aleatoriamente um vertice da LRC
			antec = ultimoVertice;
			ultimoVertice = lrc[rand() % lrc.size()];
			verticeVisitado[ultimoVertice] = 1;
			r->vertices.push_back( ultimoVertice );
			r->custo += g->getCustoArestaDual( antec, ultimoVertice );
			if (ultimoVertice != depArtificial) r->cargaCons += g->getCargaVertice( ultimoVertice );
			lrc.clear();
		}
		else //significa que essa tentativa de solucao inicial deve ser abortada e uma nova tentativa iniciada
		{
			while ( r->vertices[r->vertices.size()-2] > nRequisicoes ) r->vertices.pop_back();
			ultimoVertice = r->vertices[r->vertices.size()-1];
			r->cargaCons = g->getCargaVertice( ultimoVertice );
			r->custo = ultimoCustoForn;
			for (int i = (nRequisicoes+1); i < depArtificial; ++i) verticeVisitado[i] = 0;
			verticeVisitado[ultimoVertice] = 1;

			if (++tentativas > 10) 
			{
				delete r;
				delete [] verticeVisitado;
				delete [] estimativaCusto;
				return NULL;
			}
		}

	}while( ultimoVertice != depArtificial );

	//preeche o vector de vertices fora da rota que sera usado na BL insercao (por eficiencia)
	preencheVerticesForaRota( r );

	delete [] verticeVisitado;
	delete [] estimativaCusto;
	return r;	
}

void GRASP::preencheVerticesForaRota(RotaG* r)
{
	verticesForaRota->clear();
	int tmp, tam = r->vertices.size();
	for ( int i = 1; i < depArtificial; ++i )
	{
		tmp = 1;
		while( tmp < tam ) 
		{
			if (r->vertices[tmp] == i) break;
			++tmp;
		}
		if (tmp == tam) verticesForaRota->push_back(i);
	}
}

int GRASP::remocao(RotaG* r, int i) //o valor de i passado como parametro indica onde comeca a tentar remover
{
	float sai, entra, custoTroca;
	int tmp, inicioCons = 1;
	vector<int>::iterator it = r->vertices.begin() + 1;
	while( r->vertices[inicioCons] <= nRequisicoes ) ++inicioCons;

	//fornecedores
	while( *it <= nRequisicoes )
	{
		if ( g->existeAresta( *(it-1) , *(it+1) ) )
		{
			sai = g->getCustoArestaDual( *(it-1) , *it ) + g->getCustoArestaDual( *it , *(it+1) );
			entra = g->getCustoArestaDual( *(it-1) , *(it+1) );
			if (entra < sai){ //significa que o custo da solucao que remove i eh menor

				//verifica se a remocao de *it implica em algum consumidor que antes nao pagava custo de troca passar a pagar
				custoTroca = 0; tmp = inicioCons;
				while( r->vertices[tmp] != depArtificial )
				{
					if ( r->vertices[tmp] == ( *it + nRequisicoes ) )
					{
						custoTroca = custoTrocaCD[*it];
						break;
					}
					++tmp;
				}

				if ( ( entra + custoTroca ) < ( sai - 0.01 ) )
				{
					r->cargaForn -= g->getCargaVertice( *it );
					verticesForaRota->insert( verticesForaRota->begin(), *it );
					r->vertices.erase( it );
					r->custo -= sai;
					r->custo += entra;
					r->custo += custoTroca;
					return 1;
				}
			}
		}

		++it;
	}

	//consumidores
	while( *it != depArtificial )
	{
		if ( g->existeAresta( *(it-1) , *(it+1) ) )
		{
			sai = g->getCustoArestaDual( *(it-1) , *it ) + g->getCustoArestaDual( *it , *(it+1) );
			entra = g->getCustoArestaDual( *(it-1) , *(it+1) );

			if ( entra < ( sai + custoTrocaCD[(*it) - nRequisicoes] ) ) //caso i pague custo de troca, retirar i eh lucrativo
			{

				//verifica se *it tem ou nao custo de troca [a comparacao anterior eh para poupar processamento, 
				//pois caso (entra >= (sai + custoTrocaCD)), tambem sera (entra >= sai + 0), evitando calcular o custo de troca]
				custoTroca = custoTrocaCD[(*it) - nRequisicoes]; tmp = 1;
				while( r->vertices[tmp] <= nRequisicoes )
				{
					if (r->vertices[tmp] == ( (*it) - nRequisicoes ) )
					{
						custoTroca = 0;
						break;
					}
					++tmp;
				}

				if ( entra < ( sai + custoTroca - 0.01 ) )
				{
					r->cargaCons -= g->getCargaVertice( *it );
					verticesForaRota->push_back( *it );
					r->vertices.erase( it );
					r->custo -= sai;
					r->custo -= custoTroca;
					r->custo += entra;
					return 1;
				}
			}
		}

		++it;
	}
	return 0;
}

int GRASP::insercao(RotaG* r, int i)
{
	float sai, entra, custoTroca;
	vector<int>::iterator itInicioConsForaRota, itfr, itr = r->vertices.begin();
	int tmp, inicioCons = 1, capacidadeCarga = g->getCapacVeiculo();
	while( r->vertices[inicioCons] <= nRequisicoes ) ++inicioCons;

	//fornecedores
	while( *itr <= nRequisicoes )
	{
		itfr = verticesForaRota->begin();

		while ((  *itfr <= nRequisicoes ) && ( itfr != verticesForaRota->end() ) )
		{
			if ( ( ( r->cargaForn + g->getCargaVertice( *itfr ) ) <= capacidadeCarga ) && 
					( g->existeAresta( *itr , *itfr ) ) &&  ( g->existeAresta( *itfr , *(itr+1) ) ) )
			{
				sai = g->getCustoArestaDual( *itr , *(itr+1) );
				entra = g->getCustoArestaDual( *itr , *itfr ) + g->getCustoArestaDual( *itfr , *(itr+1) );
				if ( ( entra - custoTrocaCD[*itfr]) < sai){ //ao inserir *itfr, PODE SER que o custo de troca do respectivo consumidor DEIXE DE SER PAGO

					//certifica se ao inserir *itfr o respectivo consumidor que pagava custo de troca deixara de pagar
					custoTroca = 0; tmp = inicioCons;
					while( r->vertices[tmp] != depArtificial )
					{
						if ( r->vertices[tmp] == ( *itfr + nRequisicoes ) )
						{
							custoTroca = custoTrocaCD[*itfr];
							break;
						}
						++tmp;
					}

					if ( ( entra - custoTroca ) < ( sai - 0.01 ) )
					{
						r->cargaForn += g->getCargaVertice( *itfr );
						r->vertices.insert( itr+1 , *itfr );
						verticesForaRota->erase( itfr );
						r->custo -= sai;
						r->custo -= custoTroca;
						r->custo += entra;
						return 1;
					}
				}
			}
			++itfr;
		}
		++itr;
	}

	//consumidores
	itInicioConsForaRota = itfr;
	while( itr != r->vertices.end() )
	{
		itfr = itInicioConsForaRota;

		while ( itfr != verticesForaRota->end() )
		{
			if ( ( ( r->cargaCons + g->getCargaVertice( *itfr ) ) <= capacidadeCarga ) && 
					( g->existeAresta( *(itr-1) , *itfr ) ) &&  ( g->existeAresta( *itfr , *itr ) ) )
			{
				sai = g->getCustoArestaDual( *(itr-1) , *itr );
				entra = g->getCustoArestaDual( *(itr-1) , *itfr ) + g->getCustoArestaDual( *itfr , *itr );

				if (entra < sai){ //significa que ao incluir o vertice apontado por it2, o custo da solucao diminue

					//verifica se it2 tem ou nao custo de troca [a comparacao anterior eh para poupar processamento, 
					//pois caso (entra >= sai), tambem sera (entra + custoTrocaCD >= sai), evitando calcular o custo de troca]
					custoTroca = custoTrocaCD[(*itfr) - nRequisicoes]; tmp = 1;
					while ( r->vertices[tmp] <= nRequisicoes )
					{
						if ( r->vertices[tmp] == ( (*itfr) - nRequisicoes ) )
						{
							custoTroca = 0;
							break;
						}
						++tmp;
					}

					if ( ( entra + custoTroca ) < ( sai - 0.01 ) )
					{
						r->cargaCons += g->getCargaVertice( *itfr );
						r->vertices.insert( itr, *itfr );
						verticesForaRota->erase( itfr );
						r->custo -= sai;
						r->custo += entra;
						r->custo += custoTroca;
						return 1;
					}
				}
			}
			++itfr;
		}
		++itr;
	}
	return 0;
}

int GRASP::swap(RotaG* r){
	float entra, sai;
	int tmp, inicioCons = 1;
	while(r->vertices[inicioCons] <= nRequisicoes) ++inicioCons;

	for ( int i = 1; i < inicioCons; ++i )
	{
		for ( int j = i+1; j < inicioCons; ++j )
		{
			if ( j == ( i+1 ) )
			{
				if (( g->existeAresta( r->vertices[i-1] , r->vertices[j] ) == false ) || 
					( g->existeAresta( r->vertices[j] , r->vertices[i] ) == false ) || 
					( g->existeAresta( r->vertices[i] , r->vertices[j+1] ) == false ) ) continue;
			}
			else
			{
				if (( g->existeAresta( r->vertices[i-1] , r->vertices[j] ) == false ) || 
					( g->existeAresta( r->vertices[j] , r->vertices[i+1] ) == false ) || 
					( g->existeAresta( r->vertices[j-1] , r->vertices[i] ) == false ) || 
					( g->existeAresta( r->vertices[i] , r->vertices[j+1] ) == false ) ) continue;
			}

			//remove as arestas (i-1)->(i) , (i)->(i+1) , (j-1)->(j) , (j)->(j+1) [exceto se i+1 == j]
			if ( j == ( i+1 ) )
			{
				sai = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[i] ) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[i+1] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[j+1] );
			}
			else
			{
				sai = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[i] ) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[i+1] ) + 
						g->getCustoArestaDual( r->vertices[j-1] , r->vertices[j] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[j+1] );
			}

			//inclue as arestas (i-1)->(j) , (j)->(i+1) , (j-1)->(i) , (i)->(j+1) [exceto se i+1 == j]
			if ( j == ( i+1 ) )
			{
				entra = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[j] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[i] ) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[j+1] );
			}
			else
			{
				entra = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[j] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[i+1] ) +
						g->getCustoArestaDual( r->vertices[j-1] , r->vertices[i] ) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[j+1] );
			}

			if ( entra < ( sai - 0.01 ) ) //troca i e j na rota, atualiza seu custo e executa nova iteracao
			{
				tmp = r->vertices[i];
				r->vertices[i] = r->vertices[j];
				r->vertices[j] = tmp;
				r->custo -= sai;
				r->custo += entra;
				return 1;		
			}
		}
	}

	for ( int i = inicioCons; r->vertices[i] != depArtificial; ++i )
	{
		for ( int j = i+1; r->vertices[j] != depArtificial; ++j )
		{
			if ( j == ( i+1 ) )
			{
				if (( g->existeAresta( r->vertices[i-1] , r->vertices[j] ) == false ) || 
					( g->existeAresta( r->vertices[j] , r->vertices[i] ) == false ) || 
					( g->existeAresta( r->vertices[i] , r->vertices[j+1] ) == false ) ) continue;
			}
			else
			{
				if (( g->existeAresta( r->vertices[i-1] , r->vertices[j] ) == false ) || 
					( g->existeAresta( r->vertices[j] , r->vertices[i+1] ) == false ) || 
					( g->existeAresta( r->vertices[j-1] , r->vertices[i] ) == false ) || 
					( g->existeAresta( r->vertices[i] , r->vertices[j+1] ) == false ) ) continue;
			}

			//remove as arestas (i-1)->(i) , (i)->(i+1) , (j-1)>(j) , (j)->(j+1) [exceto se i+1 == j]
			if ( j == (i+1) )
			{
				sai = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[i]) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[i+1] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[j+1] );
			}
			else
			{
				sai = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[i] ) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[i+1] ) + 
						g->getCustoArestaDual( r->vertices[j-1] , r->vertices[j] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[j+1] );
			}

			//inclue as arestas (i-1)->(j) , (j)->(i+1) , (j-1)->(i) , (i)->(j+1) [exceto se i+1 == j]
			if ( j == (i+1) )
			{
				entra = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[j] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[i] ) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[j+1] );
			}
			else
			{
				entra = g->getCustoArestaDual( r->vertices[i-1] , r->vertices[j] ) + 
						g->getCustoArestaDual( r->vertices[j] , r->vertices[i+1] ) +
						g->getCustoArestaDual( r->vertices[j-1] , r->vertices[i] ) + 
						g->getCustoArestaDual( r->vertices[i] , r->vertices[j+1] );
			}

			if ( entra < (sai - 0.01) ) //troca i e j na rota, atualiza seu custo e executa nova iteracao
			{ 
				tmp = r->vertices[i];
				r->vertices[i] = r->vertices[j];
				r->vertices[j] = tmp;
				r->custo -= sai;
				r->custo += entra;
				return 1;
			}
		}
	}
	return 0;
}

vector < Movimento > GRASP::movimentosPR(RotaG* solution, RotaG* guiding){ 	//todos os movimentos devem ser feitos na viabilidade
	int i, j, ajusteS, ajusteG;
	vector <int> sol, guide;
	vector < Movimento > mvForn, mvCons, mvAll;

	for (i = 1; solution->vertices[i] <= nRequisicoes; ++i) sol.push_back(solution->vertices[i]);
	for (j = 1; guiding->vertices[j] <= nRequisicoes; ++j) guide.push_back(guiding->vertices[j]);
	//obtem os possiveis movimentos dos fornecedores
	mvForn = match(sol, guide, solution->cargaForn, 1, 1);

	sol.clear(); guide.clear(); ajusteS = i; ajusteG = j;
	for ( ; solution->vertices[i] != depArtificial; ++i) sol.push_back(solution->vertices[i]);
	for ( ; guiding->vertices[j] != depArtificial; ++j) guide.push_back(guiding->vertices[j]);
	//obtem os possiveis movimentos dos consumidores
	mvCons = match(sol, guide, solution->cargaCons, ajusteS, ajusteG);

	//junta os movimentos obtidos dos fornecedores e dos consumidores em um unico vector e retorna
	for ( int k = 0; k < mvForn.size(); ++k ) mvAll.push_back(mvForn[k]);
	for ( int k = 0; k < mvCons.size(); ++k ) mvAll.push_back(mvCons[k]);

	return mvAll;
}

vector < Movimento > GRASP::match(vector <int>& s, vector <int>& e, int cargaS, int ajusteS, int ajusteE){
	vector < Movimento > movimentos;
	int aux, i, j, tamS = s.size(), tamE = e.size(), capacidade = g->getCapacVeiculo();

	//primeiro verifica todos os FORNECEDORES que nao estao em solution e deveultimoVerticem ser incluidos
	//ou aqueles que estao em solution mas nao na posicao correta (deve ser feito swap)
	for ( i = 0; i < tamE; ++i )
	{
		for ( j = 0; j < tamS; ++j )
		{
			if ( e[i] == s[j] )
			{
				//ajusta o indice para que o swap nao seja feito entre consumidores e fornecedores e vice-versa
				if ( ( ( i+ajusteE ) >= ajusteS ) && ( ( i+ajusteE ) < ( tamS+ajusteS ) ) ) 
				{
					aux = i+ajusteE;
				} else if ((i+ajusteE) >= ajusteS) {
					aux = tamS+ajusteS-1;
				} else if ( ( i+ajusteE ) <= ( tamS+ajusteS ) ) {
					aux = ajusteS;
				}

				if ( ( j != i ) && ( ( j+ajusteS ) != aux ) ) //realiza SWAP
				{
					Movimento mv ('S', j+ajusteS, aux);
					movimentos.push_back(mv);
				}
				break;
			}
		}
		if (j == tamS) //realiza a insercao do vertice que nao esta na rota
		{
			if (cargaS + g->getCargaVertice(e[i]) <= capacidade) 
			{
				//ajusta o indice para que a INSERCAO nao inclua um fornecedor em um grupo de consumidores e vice-versa
				if (((i+ajusteE) >= ajusteS) && ((i+ajusteE) <= (tamS+ajusteS))) 
				{
					aux = i+ajusteE;
				} else if ((i+ajusteE) >= ajusteS) {
					aux = tamS+ajusteS;
				} else if ((i+ajusteE) <= (tamS+ajusteS)) {
					aux = ajusteS;
				}

				Movimento mv ('I', e[i], aux);
				movimentos.push_back(mv);
			}
		}
	}

	//verifica quais vertices devem ser REMOVIDOS
	for (i = 0; i < tamS; ++i)
	{
		for (j = 0; j < tamE; ++j)
		{
			if (s[i] == e[j]) break;
		}

		if (j == tamE) //significa que s[i] nao esta em guiding e deve ser removido
		{
			Movimento mv ('R', i+ajusteS);
			movimentos.push_back(mv);
		}
	}

	return movimentos;
}

RotaG* GRASP::pathRelinking(RotaG* rt1, RotaG* rt2){
	bool inviavel = false;
	int i, j , aux, tmp;
	vector < Movimento > m;
	RotaG* melhorRotaDoCaminho = new RotaG();
	float custoMv, sai, entra, menor = MAIS_INFINITO;

	do{
		//atualiza a melhorRotaDoCaminho com as informacoes da rota cujo custo eh o menor encontrado
		if ( rt1->custo < ( menor - 0.01 ) )
		{
			melhorRotaDoCaminho->vertices.clear();
			melhorRotaDoCaminho->vertices.assign(rt1->vertices.begin(), rt1->vertices.end());
			melhorRotaDoCaminho->cargaForn = rt1->cargaForn;
			melhorRotaDoCaminho->cargaCons = rt1->cargaCons;
			melhorRotaDoCaminho->custo = rt1->custo;
			menor = rt1->custo;
		}		

		m.clear();		
		m = movimentosPR(rt1, rt2);

		if ( m.size() > 0 ) 
		{			
			aux = defineIndiceMovimento(m, rt1, custoMv);
			if (m[aux].tipo == 'S') //SWAP
			{
				//realiza o movimento de swap e atualiza o custo da rota
				tmp = rt1->vertices[m[aux].arg1];
				rt1->vertices[m[aux].arg1] = rt1->vertices[m[aux].arg2];
				rt1->vertices[m[aux].arg2] = tmp;
				rt1->custo += custoMv;

			} else if (m[aux].tipo == 'I') { //INSERCAO

				//realiza o movimento de insercao e atualiza a carga e o custo da rota
				vector<int>::iterator it = rt1->vertices.begin() + m[aux].arg2;
				if ( m[aux].arg1 <= nRequisicoes )
				{
					rt1->cargaForn += g->getCargaVertice(m[aux].arg1);
				} else {
					rt1->cargaCons += g->getCargaVertice(m[aux].arg1);
				}
				rt1->vertices.insert(it, m[aux].arg1);
				rt1->custo += custoMv;

			} else { //REMOCAO

				//realiza o movimento de remocao e atualiza a carga e o custo da rota
				vector<int>::iterator it = rt1->vertices.begin() + m[aux].arg1;
				if ( *it <= nRequisicoes ) 
				{
					rt1->cargaForn -= g->getCargaVertice(*it);
				} else {
					rt1->cargaCons -= g->getCargaVertice(*it);
				}
				rt1->vertices.erase(it);
				rt1->custo += custoMv;
			}

			//verifica se o movimento realizado tornou a rota inviavel
			//a rota se tornara viavel naturalmente, pois a rota guiding eh viavel
			if (custoMv > 1000000)
			{
				inviavel = true;

			}
			else if ( inviavel ) 
			{
				bool viabilizou = true;
				vector<int>::iterator it = (rt1->vertices.begin()+1);
				while ( it != rt1->vertices.end() ) 
				{
					if ( g->existeAresta( *(it-1) , *it ) == false )
					{
						viabilizou = false;
						break;
					}
					++it;
				}

				if ( viabilizou )
				{					
					//re-calcula o custo da rota baseando-se nos valores das arestas (o custo anterior da rota eh ignorado)
					inviavel = false;
					int j, inicioCons = 1;
					while (rt1->vertices[inicioCons] <= nRequisicoes) ++inicioCons;

					it = (rt1->vertices.begin()+1);
					float custoCD = 0, custoRota = 0;
					while ( it != rt1->vertices.end() ) 
					{
						custoRota += g->getCustoArestaDual(*(it-1), *it);
						if ( *(it-1) >= nRequisicoes )
						{
							for (j = 1; j < inicioCons; ++j)
							{
								if (*(it-1) == (rt1->vertices[j] + nRequisicoes)) break;
							}
							if (j == inicioCons) custoCD += custoTrocaCD[*(it-1) - nRequisicoes];
						}
						++it;
					}
					rt1->custo = custoRota + custoCD;
				}
			}
		}
	} while ( m.size() > 0 );

	return melhorRotaDoCaminho;
}

int GRASP::defineIndiceMovimento(vector < Movimento >& mv, RotaG* sol, float& custo){
	bool movimentoViavel;
	int i, j, tmp, iMenorMv, tam = mv.size();
	float *custoMovimentos = new float[tam];
	float sai, entra, menorMv = 100*MAIS_INFINITO;

	for (int k = 0; k < tam; ++k){
		if (mv[k].tipo == 'S') //SWAP
		{
			movimentoViavel = true;

			//pega os valores dos parametros do movimento
			if (mv[k].arg1 < mv[k].arg2)
			{
				i = mv[k].arg1;
				j = mv[k].arg2;
			} else {
				i = mv[k].arg2;
				j = mv[k].arg1;
			}

			//verifica se o movimento sera feito usando arestas do grafo (resultando uma rota viavel)
			if ( j == ( i+1 ) )
			{
				if (( g->existeAresta( sol->vertices[i-1] , sol->vertices[j] ) == false ) || 
					( g->existeAresta( sol->vertices[j] , sol->vertices[i] ) == false ) || 
					( g->existeAresta( sol->vertices[i] , sol->vertices[j+1] ) == false ) ) movimentoViavel = false;
			}
			else
			{
				if (( g->existeAresta( sol->vertices[i-1] , sol->vertices[j] ) == false ) || 
					( g->existeAresta( sol->vertices[j] , sol->vertices[i+1] ) == false ) || 
					( g->existeAresta( sol->vertices[j-1] , sol->vertices[i] ) == false ) || 
					( g->existeAresta( sol->vertices[i] , sol->vertices[j+1] ) == false ) ) movimentoViavel = false;
			}

			if ( movimentoViavel ) 
			{ 
				//remove as arestas (i-1)->(i) , (i)->(i+1) , (j-1)->(j) , (j)->(j+1) [exceto se i+1 == j]
				if ( j == ( i+1 ) )
				{
					sai = g->getCustoArestaDual( sol->vertices[i-1] , sol->vertices[i] ) + 
							g->getCustoArestaDual( sol->vertices[i] , sol->vertices[i+1] ) + 
							g->getCustoArestaDual( sol->vertices[j] , sol->vertices[j+1] );
				}
				else
				{
					sai = g->getCustoArestaDual( sol->vertices[i-1] , sol->vertices[i] ) + 
							g->getCustoArestaDual( sol->vertices[i] , sol->vertices[i+1] ) + 
							g->getCustoArestaDual( sol->vertices[j-1] , sol->vertices[j] ) + 
							g->getCustoArestaDual( sol->vertices[j] , sol->vertices[j+1] );
				}

				//inclue as arestas (i-1)->(j) , (j)->(i+1) , (j-1)->(i) , (i)->(j+1) [exceto se i+1 == j]
				if ( j == ( i+1 ) )
				{
					entra = g->getCustoArestaDual( sol->vertices[i-1] , sol->vertices[j] ) + 
							g->getCustoArestaDual( sol->vertices[j] , sol->vertices[i] ) + 
							g->getCustoArestaDual( sol->vertices[i] , sol->vertices[j+1] );
				}
				else
				{
					entra = g->getCustoArestaDual( sol->vertices[i-1] , sol->vertices[j] ) + 
							g->getCustoArestaDual( sol->vertices[j] , sol->vertices[i+1] ) +
							g->getCustoArestaDual( sol->vertices[j-1] , sol->vertices[i] ) + 
							g->getCustoArestaDual( sol->vertices[i] , sol->vertices[j+1] );
				}

				//armazena o custo do movimento
				custoMovimentos[k] = (entra - sai);

			} else {
				custoMovimentos[k] = MAIS_INFINITO;
			}

		} else if (mv[k].tipo == 'I') { //INSERCAO

			//custo de arcos com o movimento
			vector<int>::iterator it = sol->vertices.begin() + mv[k].arg2;
			if ( ( g->existeAresta( *(it-1) , mv[k].arg1) ) && ( g->existeAresta ( mv[k].arg1 , *it ) ) ) 
			{
				sai = g->getCustoArestaDual( *(it-1) , *it );
				entra = g->getCustoArestaDual( *(it-1) , mv[k].arg1 ) + g->getCustoArestaDual( mv[k].arg1 , *it );

				//o custo de troca no CD pode diminuir, caso o vertice inserido seja fornecedor de algum consumidor na rota
				//ou pode tambem aumentar, caso o vertice inserido seja um consumidor cujo fornecedor nao esteja na rota
				float custoCD;
				if (mv[k].arg1 <= nRequisicoes){ 
					custoCD = 0;
					for (int i = 1; sol->vertices[i] != depArtificial; ++i){
						if (mv[k].arg1 == (sol->vertices[i] - nRequisicoes)) { custoCD = -custoTrocaCD[mv[k].arg1]; break; }
					}
				}else{
					custoCD = custoTrocaCD[mv[k].arg1 - nRequisicoes];
					for (int i = 1; sol->vertices[i] <= nRequisicoes; ++i){
						if (mv[k].arg1 == (sol->vertices[i] + nRequisicoes)) { custoCD = 0; break; }
					}
				}
				//armazena o custo do movimento
				custoMovimentos[k] = (entra - sai + custoCD);

			} else {
				custoMovimentos[k] = MAIS_INFINITO;
			}

		}else{ //REMOCAO

			//custo de arcos com o movimento
			vector<int>::iterator it = sol->vertices.begin() + mv[k].arg1;
			if ( g->existeAresta( *(it-1) , *(it+1) ) )
			{
				sai = g->getCustoArestaDual( *(it-1) , *it ) + g->getCustoArestaDual( *it , *(it+1) );
				entra = g->getCustoArestaDual( *(it-1) , *(it+1) );

				//o custo de troca no CD pode diminuir, caso o vertice removido seja consumidor que pavaga tal custo
				//ou pode tambem aumentar, caso o vertice removido seja um fornecedor cujo consumidor nao esteja na rota
				float custoCD;
				if ( *it <= nRequisicoes )
				{ 
					custoCD = 0;
					for ( int i = 1; sol->vertices[i] != depArtificial; ++i )
					{
						if ( *it == ( sol->vertices[i] - nRequisicoes ) ) { custoCD = custoTrocaCD[*it]; break; }
					}
				} else {
					custoCD = -custoTrocaCD[*it - nRequisicoes];
					for ( int i = 1; sol->vertices[i] <= nRequisicoes; ++i )
					{
						if ( *it == ( sol->vertices[i] + nRequisicoes ) ) { custoCD = 0; break; }
					}
				}
				//atualiza o custo do movimento
				custoMovimentos[k] = (entra - sai + custoCD);

			} else {
				custoMovimentos[k] = MAIS_INFINITO;
			}
		}

		if (custoMovimentos[k] < menorMv){
			menorMv = custoMovimentos[k];
			iMenorMv = k;
		}
	}

	//caso o menor movimento seja Swap, abre a possibilidade de se escolher aleatoriamente ou este movimento, ou insercao ou remocao (caso existam)
	//este procedimento evita loop infinito ao escolher o swap, uma vez que este movimento nao muda a estrutura da rota quanto a presenca dos vertices
	if (mv[iMenorMv].tipo != 'S') {
		custo = custoMovimentos[iMenorMv];

	}else{
		tmp = (rand() % 2);
		if (tmp == 0){ //retorna o movimento de swap mesmo
			custo = custoMovimentos[iMenorMv];

		}else { //procura pelo movimento de insercao ou remocao de menor custo, caso nao encontre, retorna o swap mesmo
			menorMv = 100*MAIS_INFINITO; tmp = -1;
			for (int k = 0; k < tam; ++k){
				if (((mv[k].tipo == 'I') || (mv[k].tipo == 'R'))  && (custoMovimentos[k] < menorMv)){
					menorMv = custoMovimentos[k];
					tmp = k;
				}
			}
			if (tmp >= 0){
				custo = custoMovimentos[tmp];
				iMenorMv =  tmp;
			}else{
				custo = custoMovimentos[iMenorMv];
			}
		}
	}

	delete [] custoMovimentos;
	return iMenorMv;
}

Rota* GRASP::getRotaConvertida(int index)
{
	//calcula o custo real da rota (sem custo de troca, devido ao modelo), pois o GRASP lida com os custos duais
	int ultimo = rotasNegativas[index]->vertices.size() - 2;
	float custo = g->getCustoAresta(0, rotasNegativas[index]->vertices[1]);
	for (int i = 1; i <= ultimo; ++i) custo += g->getCustoAresta(rotasNegativas[index]->vertices[i], rotasNegativas[index]->vertices[i+1]);

	Rota* r = new Rota();
	r->setVertices(rotasNegativas[index]->vertices);
	r->setCustoReduzido(rotasNegativas[index]->custo);
	r->setCusto(custo);
	delete rotasNegativas[index];
	return r;
}

int GRASP::run(int it_max, float valorSub, Rota* rotaCplex){
	int aux, it = 0;
	RotaG *r, *melhorPR, *guiding;
	float menorCusto = MAIS_INFINITO;

	if (rotaCplex != NULL){ //vai explorar a rota obtida pelo branch and cut do cplex

		//primeiro limpa as rotas obtidas da execucao da heuristica (caso existam)
		for (int i = 0; i < rotasNegativas.size(); ++i) delete rotasNegativas[i];
		rotasNegativas.clear();

		//cria uma RotaG baseando-se no objeto Rota retornado pelo cplex
		RotaG* rCplex = new RotaG();
		rCplex->vertices = rotaCplex->getVertices();
		rCplex->custo = rotaCplex->getCustoReduzido();
		rCplex->cargaForn = 0; rCplex->cargaCons = 0;
		for (int i = 1; rCplex->vertices[i] != depArtificial; ++i){
			if (rCplex->vertices[i] <= nRequisicoes){
				rCplex->cargaForn += g->getCargaVertice(rCplex->vertices[i]);
			}else{
				rCplex->cargaCons += g->getCargaVertice(rCplex->vertices[i]);
			}
		}
		preencheVerticesForaRota(rCplex);
		insereRotaNegativa(rCplex, 'C');
		//executa a busca local na solucao do cplex
		while(true){
			if ( !remocao( rCplex ) ) {
				if ( !insercao( rCplex )) {
					if ( !swap( rCplex ) ) {
						break;
					}
				}
			}
		}

		if (rCplex->custo < (rotasNegativas[0]->custo - 0.01)) insereRotaNegativa(rCplex, 'C');
		guiding = rCplex;
		menorCusto = rCplex->custo;
	}

	while(it <= it_max){

		do
		{
			++it;
			r = solucaoInicial();
		}
		while(r == NULL);

		while(true)
		{
			if ( !remocao( r ) )
			{ 
				if ( !insercao( r ) )
				{
					if ( !swap( r ) )
					{
						break;
					}
				}
			}
		}

		if (r->custo < (menorCusto - 0.01))
		{
			guiding = r;
			menorCusto = r->custo;
		}

		//inclue a rota obtida pela Busca Local, caso ela tenha o custo reduzido negativo
		if (r->custo < ( valorSub - 0.01 ) ) insereRotaNegativa(r, 'C');

		melhorPR = pathRelinking(r, guiding);
		if (melhorPR->custo < (menorCusto - 0.01)) 
		{ 
			//preeche o vector de vertices fora da rota que sera usado na insercao
			preencheVerticesForaRota(melhorPR);
			while(true)
			{
				if ( !remocao( melhorPR ) )
				{
					if ( !insercao( melhorPR ))
					{
						if ( !swap(melhorPR) )
						{
							break;
						}
					}
				}
			}
			delete guiding;
			guiding = melhorPR;
			menorCusto = melhorPR->custo;
			if (guiding->custo < (valorSub - 0.01)) insereRotaNegativa(guiding, 'C');
		}

		//inclui a rota obtida pelo Path Relinking, caso ela tenha o custo reduzido negativo e seja diferente de guiding
		if ( ( ( melhorPR->custo > ( guiding->custo + 0.01 ) ) || ( melhorPR->custo < ( guiding->custo - 0.01 ) ) ) && ( melhorPR->custo < ( valorSub - 0.01 ) ) )
		{
			insereRotaNegativa(melhorPR, 'P');
		}
		else if (melhorPR != guiding)
		{
			delete melhorPR;
		}

		if (r != guiding) delete r;
	}

	delete guiding;
	return rotasNegativas.size();
}

void GRASP::insereRotaNegativa(RotaG* r, char origem){
	//se a rota r for diferente das outras rotas armazenadas em v, ela sera armazenada
	bool iguais;
	int j, i = -1, tam = rotasNegativas.size();
	while(++i < tam)
	{
		iguais = false;
		if ((r->cargaForn == rotasNegativas[i]->cargaForn) && (r->cargaCons == rotasNegativas[i]->cargaCons)) //eh igual na carga, mas pode nao ser igual no custo
		{
			if ((r->custo < (rotasNegativas[i]->custo + 0.01)) && (r->custo > (rotasNegativas[i]->custo - 0.01))) // eh igual no custo tambem, mas pode nao ser na rota
			{
				if (r->vertices.size() == rotasNegativas[i]->vertices.size()) //eh igual em tudo, mas precisa verificar os vertices da rota
				{
					for (j = 1; j < r->vertices.size(); ++j)
					{
						if (r->vertices[j] != rotasNegativas[i]->vertices[j]) break;
					}
					if (j == r->vertices.size()) iguais = true;
				}
			}
		}
		if (iguais == true) break;
	}

	if (i == tam) //significa que passou por todas as rotas de v e nenhuma delas eh igual a r, portanto, insere r
	{
		if (origem == 'C') //copia a rota a ser inserida, pois a original sera usada no path-relinking
		{
			RotaG* novaRota = new RotaG();
			novaRota->vertices.assign(r->vertices.begin(), r->vertices.end());
			novaRota->cargaForn = r->cargaForn;
			novaRota->cargaCons = r->cargaCons;
			novaRota->custo = r->custo;
			rotasNegativas.push_back(novaRota);

		}
		else //a rota passada como parametro sera apontada (sem fazer uma copia de r)
		{
			rotasNegativas.push_back(r);
		}
	}
	else
	{
		if (origem != 'C') delete r;
	}
}

