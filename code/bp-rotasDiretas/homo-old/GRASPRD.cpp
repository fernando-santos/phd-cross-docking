#include "GRASPRD.h"

GRASPRD::GRASPRD(Grafo* G, int nReqs, int depArtif, float parametroAlfa)
{
	g = G;
	nRequisicoes = nReqs;
	depArtificial = depArtif;
	alfa = parametroAlfa;
	fornForaRota = new vector<int>();
}

GRASPRD::~GRASPRD()
{
	for (int i = 0; i < rotasNegativas.size(); ++i) delete rotasNegativas[i];
	delete fornForaRota;
}

RotaRD* GRASPRD::solucaoInicial()
{
	vector<int> lrc;
	RotaRD* r = new RotaRD();
	int* verticeVisitado = new int[depArtificial+1];
	float* estimativaCusto = new float[depArtificial+1];
	float menorCusto, maiorCusto, limite, custoVisita;
	int tmp, tam, antec, ultimoVertice, tentativas = 0, capacVeiculo = g->getCapacVeiculo();

	do
	{
		r->vertices.push_back(0);
		memset(verticeVisitado, 0, (depArtificial+1)*sizeof(int));

		//PRIMEIRA ETAPA: VERTICE QUE SAI DO DEPOSITO 0 (a LRC deve ser composta apenas por fornecedores)
		menorCusto = MAIS_INFINITO;
		maiorCusto = -MAIS_INFINITO;
		for (int i = 1; i <= nRequisicoes; ++i)
		{
			estimativaCusto[i] = MAIS_INFINITO;
			if ( g->existeAresta( 0 , i )  && ( g->getCustoArestaDual( 0 , i ) < estimativaCusto[i] ) ) estimativaCusto[i] = g->getCustoArestaDual( 0 , i );
			if ( estimativaCusto[i] < menorCusto ) menorCusto = estimativaCusto[i];
			if ( estimativaCusto[i] > maiorCusto ) maiorCusto = estimativaCusto[i];
		}

		//lista restrita de candidatos: elementos com custo entre [menorCusto, menorCusto + alfa*(maiorCusto-menorCusto)]
		limite = menorCusto + alfa*(maiorCusto-menorCusto);
		for (int i = 1; i <= nRequisicoes; ++i)
		{
			if ( estimativaCusto[i] <= limite ) //solucao deve entrar na lista restrita de candidatos
			{
				lrc.push_back(i);
			}
		}

		//insere um dos vertices da lista restrita de candidatos na rota e parte para a proxima etapa da construcao
		ultimoVertice = lrc[rand() % lrc.size()];
		verticeVisitado[ultimoVertice] = 1;
		r->vertices.push_back(ultimoVertice);
		r->custo = g->getCustoArestaDual(0, ultimoVertice);
		r->carga = g->getCargaVertice(ultimoVertice);
		lrc.clear();

		//SEGUNDA ETAPA: VERTICES QUE SAEM DE UM FORNECEDOR PARA OUTRO FORNECEDOR OU PARA UM CONSUMIDOR
		do{
			menorCusto = MAIS_INFINITO;
			maiorCusto = -MAIS_INFINITO;
			for ( int i = 1; i <= nRequisicoes; ++i )
			{
				//passa pelos FORNECEDORES e marca com (-1 : unreachable) aqueles que poderiam ser visitados, mas excedem a capacidade do veiculo
				estimativaCusto[i] = estimativaCusto[i+nRequisicoes] = MAIS_INFINITO;
				if ( ( verticeVisitado[i] == 0 ) && ( ( r->carga + g->getCargaVertice( i ) ) > capacVeiculo ) ) verticeVisitado[i] = -1;

				if ( ( verticeVisitado[i] != 0 ) || ( g->existeAresta( ultimoVertice ,  i ) == false ) ) continue;

				custoVisita = g->getCustoArestaDual( ultimoVertice , i );
				if ( g->existeAresta( ultimoVertice, i ) && ( custoVisita < estimativaCusto[i] ) ) estimativaCusto[i] = custoVisita;

				if ( estimativaCusto[i] < menorCusto ) menorCusto = estimativaCusto[i];
				if ( estimativaCusto[i] > maiorCusto ) maiorCusto = estimativaCusto[i];
			}

			tam = r->vertices.size();
			for ( int i = 1; i < tam; ++i )
			{
				tmp = r->vertices[i]+nRequisicoes;
				custoVisita = g->getCustoArestaDual( ultimoVertice , tmp );
				if ( g->existeAresta( ultimoVertice, tmp ) && ( custoVisita < estimativaCusto[tmp] ) ) estimativaCusto[tmp] = custoVisita;

				if ( estimativaCusto[tmp] < menorCusto ) menorCusto = estimativaCusto[tmp];
				if ( estimativaCusto[tmp] > maiorCusto ) maiorCusto = estimativaCusto[tmp];
			}


			//a LRC sera composta por fornecedores e eventualmente consumidores, seguindo os limites de aceitacao de alpha
			if ( menorCusto < MAIS_INFINITO )
			{
				limite = menorCusto + alfa*(maiorCusto-menorCusto);
				for ( int i = 1; i < depArtificial; ++i )
				{
					if ( estimativaCusto[i] <= limite ) lrc.push_back(i);
				}

				//insere aleatoriamente um vertice da LRC (que pode ser consumidor ou fornecedor)
				antec = ultimoVertice;
				ultimoVertice = lrc[rand() % lrc.size()];
				verticeVisitado[ultimoVertice] = 1;
				r->vertices.push_back( ultimoVertice );
				r->custo += g->getCustoArestaDual( antec , ultimoVertice );
				if ( ultimoVertice <= nRequisicoes ) r->carga += g->getCargaVertice( ultimoVertice );
				lrc.clear();
			}
			else //significa que essa tentativa de solucao inicial deve ser abortada e uma nova tentativa iniciada
			{
				r->vertices.clear();
				break;
			}

		}while(ultimoVertice <= nRequisicoes);

		if ( tentativas++ > 10 )
		{
			delete r;
			delete [] verticeVisitado;
			delete [] estimativaCusto;
			return NULL;
		}

	} while( ultimoVertice <= nRequisicoes );

	//NESTE PONTO TEM-SE A ROTA COM OS FORNECEDORES E UM CONSUMIDOR: ULTIMA ETAPA DA CONSTRUCAO (INSERE OS CONSUMIDORES CUJOS FORNECEDORES ESTEJAM NA ROTA)

	antec = ultimoVertice;
	for ( int i = 1; r->vertices[i] <= nRequisicoes; ++i )
	{
		if ( ( r->vertices[i]+nRequisicoes ) != ultimoVertice )
		{
			r->vertices.push_back( r->vertices[i]+nRequisicoes );
			r->custo += g->getCustoArestaDual( antec , r->vertices[i]+nRequisicoes );
			antec = r->vertices[i]+nRequisicoes;
			verticeVisitado[antec] = 1;
		}
	}
	r->vertices.push_back( depArtificial );
	r->custo += g->getCustoArestaDual( antec , depArtificial );

	//preeche o vector de vertices fora da rota que sera usado na BL insercao (por eficiencia)
	preencheFornecedoresForaRota( r );

	delete [] verticeVisitado;
	delete [] estimativaCusto;
	return r;	
}

void GRASPRD::preencheFornecedoresForaRota(RotaRD* r){
	int tmp;
	fornForaRota->clear();
	for ( int i = 1; i <= nRequisicoes; ++i )
	{
		for ( tmp = 1; r->vertices[tmp] <= nRequisicoes; ++tmp ) 
		{
			if ( r->vertices[tmp] == i ) break;
		}
		if ( r->vertices[tmp] > nRequisicoes ) fornForaRota->push_back(i);
	}
}

int GRASPRD::remocao(RotaRD* r, int i) //o valor de i passado como parametro indica onde comeca a tentar remover
{
	int tmp;
	float sai, entra;
	vector<int>::iterator itC, itF = r->vertices.begin()+1;
	vector<int>::iterator inicioItC = r->vertices.begin()+1;
	while ( *inicioItC <= nRequisicoes ) ++inicioItC;

	//verifica apenas se os fornecedores podem ser removidos (os consumidores tambem serao automaticamente)
	while( *itF <= nRequisicoes )
	{
		itC = inicioItC;
		while ( *itC != ((*itF)+nRequisicoes) ) ++itC;

		if ( *(itF+1) != *itC )
		{
			if ( g->existeAresta( *(itF-1) , *(itF+1) ) && g->existeAresta( *(itC-1) , *(itC+1) ) )
			{
				sai = g->getCustoArestaDual( *(itF-1) , *itF ) + g->getCustoArestaDual( *itF , *(itF+1) ) +
						g->getCustoArestaDual( *(itC-1) , *itC ) + g->getCustoArestaDual( *itC , *(itC+1) );
				entra = g->getCustoArestaDual( *(itF-1) , *(itF+1) ) + g->getCustoArestaDual( *(itC-1) , *(itC+1) );

				if ( ( entra - sai ) < -0.01 ) //significa que o custo da solucao que remove i eh menor
				{
					r->carga -= g->getCargaVertice( *itF );
					fornForaRota->insert( fornForaRota->begin(), *itF );
					r->vertices.erase( itC ); r->vertices.erase( itF );
					r->custo += ( entra - sai );
					return 1;
				}
			}
		}
		else //o ultimo fornecedor e o primeiro consumidor na rota sao do mesmo produto (a remocao deve retirar ambos)
		{
			if ( g->existeAresta( *(itF-1) , *(itF+2) ) )
			{
				sai = g->getCustoArestaDual( *(itF-1) , *itF ) + g->getCustoArestaDual( *itF , *(itF+1) ) + g->getCustoArestaDual( *(itF+1) , *(itF+2) );
				entra = g->getCustoArestaDual( *(itF-1) , *(itF+2) );
				if ( ( entra - sai ) < -0.01 ) //significa que o custo da solucao que remove i eh menor
				{
					r->carga -= g->getCargaVertice( *itF );
					fornForaRota->insert( fornForaRota->begin(), *itF );
					r->vertices.erase( itC ); r->vertices.erase( itF );
					r->custo += ( entra - sai );
					return 1;
				}
			}
		}
		++itF;
	}
	return 0;
}

int GRASPRD::insercao(RotaRD* r, int i)
{
	float saiF, entraF, saiC, entraC, menorCustoC;
	int tmp, bestC,capacidadeCarga = g->getCapacVeiculo();
	vector<int>::iterator itfr, itC, itF = r->vertices.begin();
	vector<int>::iterator inicioItC = r->vertices.begin()+1;
	while ( *inicioItC <= nRequisicoes ) ++inicioItC;

	//verifica apenas se os fornecedores podem ser inseridos (os consumidores tambem serao automaticamente)
	while( *itF <= nRequisicoes )
	{
		itfr = fornForaRota->begin();
		while ( itfr != fornForaRota->end() )
		{
			if ( ( ( r->carga + g->getCargaVertice( *itfr ) ) <= capacidadeCarga ) && 
					( g->existeAresta( *itF , *itfr ) ) &&  ( g->existeAresta( *itfr , *(itF+1) ) ) )
			{
				saiF = g->getCustoArestaDual( *itF , *(itF+1) );
				entraF = g->getCustoArestaDual( *itF , *itfr ) + g->getCustoArestaDual( *itfr , *(itF+1) );

				itC = inicioItC;
				menorCustoC = MAIS_INFINITO;
				while ( itC != r->vertices.end() )
				{
					if ( (itF+1) != itC )
					{
						if ( g->existeAresta( *(itC-1) , (*itF)+nRequisicoes ) &&  g->existeAresta( (*itF)+nRequisicoes , *itC ) )
						{
							saiC = g->getCustoArestaDual( *(itC-1) , *itC );
							entraC = g->getCustoArestaDual( *(itC-1) , (*itfr)+nRequisicoes ) + g->getCustoArestaDual( (*itfr)+nRequisicoes , *itC );

							if ( ( entraC - saiC ) < ( menorCustoC - 0.01 ) )
							{
								bestC = *itC;
								menorCustoC = entraC - saiC;
							}
						}
					}
					++itC;
				}

				if ( ( entraF-saiF+menorCustoC ) < -0.01 )
				{
					r->carga += g->getCargaVertice( *itfr );
					r->vertices.insert( itF+1 , *itfr );

					itC = r->vertices.begin();
					while( *itC != bestC ) ++itC;
					r->vertices.insert( itC , (*itfr)+nRequisicoes );

					fornForaRota->erase( itfr );
					r->custo += (entraF-saiF+menorCustoC);
					return 1;
				}
			}
			++itfr;
		}
		++itF;
	}
	return 0;
}

int GRASPRD::swap(RotaRD* r){
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

			if ( ( entra - sai ) < -0.01 ) //troca i e j na rota, atualiza seu custo e executa nova iteracao
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

			if ( ( entra - sai ) < -0.01 ) //troca i e j na rota, atualiza seu custo e executa nova iteracao
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

int GRASPRD::run(int it_max, float valorSub){
	RotaRD *r;
	int aux, it = 0;
	float menorCusto = MAIS_INFINITO;

	while( it < it_max )
	{

		//printf("--------------it = %d--------------\n", it);

		do
		{
			++it;
			r = solucaoInicial();
		}
		while(r == NULL);

		while(true)
		{
			//printf("startR: "); r->imprimir();
			if ( !remocao( r ) )
			{
				//printf("startI: "); r->imprimir();
				if ( !insercao( r ) )
				{
					//printf("startS: "); r->imprimir();
					if ( !swap(r) )
					{
						break;
					}
				}
			}
		}

		//inclue a rota obtida pela Busca Local, caso ela tenha o custo reduzido negativo
		if ( r->custo < ( valorSub - 0.01 ) ) insereRotaNegativa(rotasNegativas, r);
	}

	return rotasNegativas.size();
}

void GRASPRD::insereRotaNegativa(vector < RotaRD* >& v, RotaRD* r)
{
	//se a rota r for diferente das outras rotas armazenadas em v, ela sera armazenada
	bool iguais;
	int j, i = -1, tam = v.size();
	while(++i < tam)
	{
		iguais = false;
		if ( r->carga == v[i]->carga ) //eh igual na carga, mas pode nao ser igual no custo
		{
			if ( ( r->custo < ( v[i]->custo + 0.01 ) ) && ( r->custo > ( v[i]->custo - 0.01 ) ) ) // eh igual no custo tambem, mas pode nao ser na rota
			{
				if ( r->vertices.size() == v[i]->vertices.size() ) //eh igual em tudo, mas precisa verificar os vertices da rota
				{
					for ( j = 1; j < r->vertices.size(); ++j )
					{
						if ( r->vertices[j] != v[i]->vertices[j] ) break;
					}
					if (j == r->vertices.size()) iguais = true;
				}
			}
		}
		if (iguais == true) break;
	}

	//caso passe por todas as rotas de v e nenhuma delas seja igual a r, insere r
	if ( i == tam ) v.push_back(r);
	else delete r;
}

Rota* GRASPRD::getRotaConvertida(int index){
	RotaRD* rG = rotasNegativas[index];

	//calcula o custo real da rota (sem custo de troca, devido ao modelo), pois o GRASP lida com os custos duais
	int ultimo = rG->vertices.size() - 2;
	float custo = g->getCustoAresta(0, rG->vertices[1], true);
	for (int i = 1; i <= ultimo; ++i) custo += g->getCustoAresta(rG->vertices[i], rG->vertices[i+1], true);

	Rota* r = new Rota();
	r->setVertices(rG->vertices);
	r->setCustoReduzido(rG->custo);
	r->setCusto(custo);
	return r;
}
