#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
#include "ListaNosAtivos.h"
#include "NoArvoreArcos.h"
#include "NoArvore.h"
using namespace std;

int main(int argc, char** argv){

	if (argc < 6)
	{
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Custo para carregar/descarregar mercadorias no Cross-Docking\n"
				<< "  (4) Numero de veiculos\n"
				<< "  (...) Capacidade relativa (1 = capacidade origina) de cada veiculo determinado em argv[4]\n"
				<< "  (argv[4] + 5) Opcao para solucao do subproblema {E (Elementar), S (StateSpaceRelax)} (OPCIONAL - default 'E')\n"
				<< "  (argv[4] + 6) Opcao para estrategia de branching {T (Tau), A (Arcos)} (OPCIONAL - default 'T')\n"
				<< "  (argv[4] + 7) Quantidade de pontos interiores para a estabilizacao (OPCIONAL - default '0')\n"
				<< "  (argv[4] + 8) Semente de números aleatórios (OPCIONAL - default 'time(0)')\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	int custoTroca = atoi(argv[3]);
	if ( custoTroca < 0 )
	{
		cout << "O valor para custo de troca das mercadorias deve ser um valor não-negativo\n" << endl;
		exit(0);
	}
	
	int numVeic = atoi(argv[4]);
	if ( argc < ( numVeic + 5 ) )
	{
		cout << "Numero de veículos e suas respectivas capacidades incompatíveis\n";
		exit(0);
	}

	if ( argc > ( numVeic + 9 ) )
	{
		cout << "Numero de parametros excede o maximo para esta configuracao de veiculos\n";
		exit(0);
	}

	float* classesVeic = new float[numVeic];
	for (int i = 0; i < numVeic; ++i)
	{
		classesVeic[i] = atof(argv[5+i]);
	}

	char opcaoSub = ( argc > ( numVeic + 5 ) ) ? argv[numVeic+5][0] : 'E';
	if ( ( opcaoSub != 'E' ) && ( opcaoSub != 'S') )
	{
		cout << "A opcao para solucao do subproblema deve ser {E, S}\n" << endl;
		exit(0);
	}
	char opcaoBranching = ( argc > ( numVeic + 6 ) ) ? argv[numVeic+6][0] : 'T';
	if ( ( opcaoBranching != 'T' ) && ( opcaoBranching != 'A') )
	{
		cout << "A opcao para estratégia de branching deve ser {T, A}\n" << endl;
		exit(0);
	}
	
	int numPontosExtremosDual = ( argc > ( numVeic + 7 ) ) ? atoi(argv[numVeic+7]) : 0;
	if ( ( numPontosExtremosDual < 0 ) || ( numPontosExtremosDual > 1000 ) )
	{
		cout << "O número de pontos extremos deve estar no intervalo [0, 1000]\n" << endl;
		exit(0);
	}

	int seed = ( argc > ( numVeic + 8 ) ) ? atoi(argv[numVeic+8]) : time(0);
	bool pricingFull, alcancouRaiz, terminouAntes = false;
	int numRotasCons, numRotasForn;
	srand(seed);
	
	/*-------------------EXIBE AS INFORMACOES SOBRE O PROBLEMA----------------------*/
	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("numVeic = %d {%f", numVeic, classesVeic[0]);
	for (int i = 1; i < numVeic; ++i) printf(", %f", classesVeic[i]);
	printf("}\ncustoTroca = %d\n", custoTroca);
	printf("Sub, Branching, NumPontosInt = {%c, %c, %d}\n", opcaoSub, opcaoBranching, numPontosExtremosDual);
	printf("seed = %d\n\n", seed);
	fflush(stdout);
	/*------------------------------------------------------------------------------*/

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], classesVeic, numVeic, numRequisicoes);
	Grafo::timeInicio = time(0);
	Grafo::timeLimit = 14400;

	//Calcula-se as rotas  (solucao viavel e pool de rotas do programa java) para os FORNECEDORES
	Rota** rotasFornViavel = geraPoolRotas(G, true, numVeic);

	//Calcula-se as rotas  (solucao viavel e pool de rotas do programa java) para os CONSUMIDORES
	Rota** rotasConsViavel = geraPoolRotas(G, false, numVeic);

	ModeloCplex modCplex(G, rotasFornViavel, rotasConsViavel, numVeic, numVeic, numVeic, numRequisicoes, custoTroca);

	//Libera memoria das rotas alocadas para fornecedores e consumidores
	delete [] rotasFornViavel;
	delete [] rotasConsViavel;
	delete [] classesVeic;

	Rota** colunas;
	int numPricingFull, tempo = time(0);
	int* pricingFullForn = new int[numVeic];
	int* pricingFullCons = new int[numVeic];

	//Declara um ponteiro para as colunas que serao retornadas do pricing 
	alcancouRaiz = false;
	int k, limVeic, iVeicForn = 0, iVeicCons = 0; int xxx = 0;
	do{

		++xxx;

		pricingFull = alcancouRaiz;
		alcancouRaiz = true;

		//roda o cplex, soluciona a relaxacao do master restrito e cria o modelo dual para a estabilizacao
		printf("%f\n", modCplex.solveMaster());
		fflush(stdout);
		modCplex.atualizaRotasBasicas();
		if ( numPontosExtremosDual > 0 ) modCplex.geraModeloDual(NULL);
		modCplex.geraPontoInterior(numPontosExtremosDual);

		//atualiza o grafo com os custos duais dos vertices, calcula as rotas e as insere no master restrito
		limVeic = iVeicForn + numVeic;
		for ( int k = iVeicForn; k < limVeic; ++k, ++iVeicForn )
		{
			if ( pricingFullForn[(k % numVeic)] != -1 )
			{
				modCplex.updateDualCostsForn(G, (k % numVeic));
				colunas = retornaColuna(G, (k % numVeic), true, pricingFull, modCplex.getAlfaDual(k % numVeic), opcaoSub, pricingFullForn[(k % numVeic)]);
				if ( colunas != NULL )
				{
					for ( int i = 0; colunas[i] != NULL; ++i )
					{
						modCplex.insert_ColumnPrimal_RowDual_Forn(colunas[i], (k % numVeic));
					}

					//Retorna os valores dos vetores para que todos computem os Subproblemas
					for ( int v = 0; v < numVeic; ++v )
					{
						pricingFullForn[v] = pricingFullCons[v] = 1;
					}
					
					alcancouRaiz = false;
					pricingFull = false;
					delete [] colunas;
					++iVeicForn;
					break;
				}
			}
		}

		limVeic = iVeicCons + numVeic;
		for ( int k = iVeicCons; k < limVeic; ++k, ++iVeicCons )
		{
			if ( pricingFullCons[(k % numVeic)] != -1 )
			{
				modCplex.updateDualCostsCons(G, (k % numVeic));
				colunas = retornaColuna(G, (k % numVeic), false, pricingFull, modCplex.getBetaDual(k % numVeic), opcaoSub, pricingFullCons[(k % numVeic)]);
				if ( colunas != NULL )
				{
					for ( int i = 0; colunas[i] != NULL; ++i )
					{
						modCplex.insert_ColumnPrimal_RowDual_Cons(colunas[i], (k % numVeic));
					}

					//Retorna os valores dos vetores para que todos computem os Subproblemas
					for ( int v = 0; v < numVeic; ++v )
					{
						pricingFullForn[v] = pricingFullCons[v] = 1;
					}

					alcancouRaiz = false;
					delete [] colunas;
					++iVeicCons;
					break;
				}
			}
		}
		
		numPricingFull = 0;
		for ( int v = 0; v < numVeic; ++v )
		{
			numPricingFull += pricingFullForn[v];
			numPricingFull += pricingFullCons[v];
		}

	}while( ( !alcancouRaiz ) || ( numPricingFull != ( -2*numVeic ) ) );

	printf("xxx = %d\n", xxx);

	float lDual = modCplex.solveMaster();
	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t -\t\t\t -\t\t%ld\n", lDual, time(0)-tempo);
	fflush(stdout);
	ModeloHeuristica* modHeuristica = new ModeloHeuristica(modCplex);
	float lPrimal = modHeuristica->optimize();
	ModeloCplex::setLimitePrimal(lPrimal);
	delete modHeuristica;

	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);

	//A partir deste ponto, a raiz do master foi alcançada e deve ser realizado o branch-and-price
	//A raiz do master representara o primeiro no da arvore, que sera ramificada com a insercao das
	//restricoes de branching. Cada no da arvore dara origem a 7 outros nos, cujas restricoes serao
	//escritas em funcao do da manipulacao das mercadorias entre fornecedores/consumidores,
	//alem das trocas no CD
	ListaNosAtivos arvoreBranching;
	ptrNo aux, noAtual;
	int noCount = 1;

	if ( opcaoBranching == 'T' )
	{
		bool segundoBranching, bestPrimal;
		noAtual = new NoArvore(modCplex, numPontosExtremosDual);

		while ( noAtual != NULL )
		{
			if ( ( ( noCount % 1 ) == 0 ) && ( noCount > 1 ) )
			{
				lDual = noAtual->getLimiteDual();
				lPrimal = ModeloCplex::getLimitePrimal();
				printf(" %d\t\t  %d\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", noCount, NoArvore::getTotalNosArmazenados(), lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);
				fflush(stdout);
			}
			
			//gera-se 4 filhos (2 considerando (tau^k_q = 0) e 2 para (tau^k_q = 1) sendo que 
			//a mercadoria 'q' devera estar em alguma rota com lambda ou gamma fracionario e 'q' nao
			//tenha sido restringido neste no da arvore.
			for ( int i = 1; i <= 4; ++i )
			{
				//o metodo geraFilho() iniciara um novo no (que tera as restricoes de branching pertinentes ao filho)
				//e solucionara sua relaxacao ate a raiz, sendo que para isto novas colunas podem ser geradas, usando
				//os custos duais da relaxacao do modelo. Caso o no possa ser podado, ele nao eh inserido na arvore

				//segundoBranching e bestPrimal sao passados por referencia, pois a a lista de nos da arvore
				//nao pode ser acessada dentro da classe NoArvore, devido a referencia cruzada
				aux = noAtual->geraFilho(modCplex, G, i, opcaoSub, segundoBranching, bestPrimal);
				if ( aux != NULL )
				{
					if ( !segundoBranching )
					{
						//Caso o no nao possa ser podado, mas ainda seja possivel realizar o branching de restricoes,
						//o insere na arvore para que seja fixada um outro par [k,q] nas proximas iteracoes
						arvoreBranching.insereNo(aux);
						
					}
					else
					{
						//Caso nao seja mais possivel realizar o branching de restricoes no No, mas ele ainda nao possa ser podado
						//por limite dual ou otimalidade, executa-se o branching de arcos neste no, ate se obter solucao inteira
						ModeloCplex* modelo = new ModeloCplex(modCplex, aux->getMatrizA_no(), aux->getMatrizB_no(), 
													aux->getPtrRForn_no(), aux->getPtrRCons_no(),  aux->getVariaveisComRestricao());
						bestPrimal = NoArvoreArcos::executaBranchingArcos(modelo, G, aux->getVariaveisComRestricao(), numVeic, numRequisicoes, opcaoSub, tempo);
						delete modelo;
						delete aux; //Poda por otimalidade
					}
					if ( bestPrimal )
					{
						arvoreBranching.podaPorLimiteDual(ModeloCplex::getLimitePrimal());
					}
				}
				modCplex.limpaVariaveisArvoreB1();
			}
			delete noAtual;
			noAtual = arvoreBranching.retornaProximo();
			++noCount;

			//Se o tempo de execucao exceder 5 horas, mostra a ultima solucao e termina
			if ( ( time(0) - tempo ) > 14400 )
			{
				if ( noAtual != NULL )
				{
					lDual = noAtual->getLimiteDual();
					lPrimal = ModeloCplex::getLimitePrimal();
					printf(" %d\t\t  %d\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld*\n", noCount, NoArvore::getTotalNosArmazenados(), lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);
					fflush(stdout);
				}
				terminouAntes = true;
				break;
			}
		}

	}
	else
	{
		vector<Rota*>* rotasVazia = new vector<Rota*>[numVeic];
		vector<short int*>* matrizVazia = new vector<short int*>[numVeic];
		char** varBranchingZero = new char*[numVeic];
		for (int k = 0; k < numVeic; ++k){
			varBranchingZero[k] = new char[numRequisicoes+1];
			memset(varBranchingZero[k], 0, (numRequisicoes+1)*sizeof(char));
		}
	
		ModeloCplex* modelo = new ModeloCplex(modCplex, matrizVazia, matrizVazia, rotasVazia, rotasVazia, varBranchingZero);
		NoArvoreArcos::executaBranchingArcos(modelo, G, varBranchingZero, numVeic, numRequisicoes, opcaoSub, tempo);
		delete modelo;

	}

	if ( !terminouAntes )
	{
		cout << "solucao otima = " << ModeloCplex::getLimitePrimal() << endl; 
		cout << "tempo = " << time(0) - tempo << endl;
		fflush(stdout);
	}

	delete G;
	return 0;
}
