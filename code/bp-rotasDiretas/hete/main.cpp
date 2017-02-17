#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
#include "NoArvore.h"
#include "ListaNosAtivos.h"
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
				<< "  (argv[4] + 6) Opcao para solucao do subproblema de rotas diretas {P (PD), B (BC)} (OPCIONAL - default 'P')\n"
				<< "  (argv[4] + 7) Semente de números aleatórios (OPCIONAL - default 'time(0)')\n\n";
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
		cout << "Numero de veículos e suas respectivas capacidades relativas incompatíveis\n";
		exit(0);
	}

	if ( argc > ( numVeic + 8 ) )
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
	char opcaoSubRD = ( argc > ( numVeic + 6 ) ) ? argv[numVeic+6][0] : 'P';
	if ( ( opcaoSubRD != 'P' ) && ( opcaoSubRD != 'B' ) )
	{
		cout << "A opcao para solucao do subproblema de rotas diretas deve ser {P, B}\n" << endl;
		exit(0);
	}

	int seed = ( argc > ( numVeic + 7 ) ) ? atoi(argv[numVeic+7]) : time(0);
	bool pricingFull, alcancouRaiz;
	int numRotasCons, numRotasForn;
	srand(seed);
	
	/*-------------------EXIBE AS INFORMACOES SOBRE O PROBLEMA----------------------*/
	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("numVeic = %d {%f", numVeic, classesVeic[0]);
	for (int i = 1; i < numVeic; ++i) printf(", %f", classesVeic[i]);
	printf("}\ncustoTroca = %d\n", custoTroca);
	printf("Sub, SubRD = {%c, %c}\n", opcaoSub, opcaoSubRD);
	printf("seed = %d\n\n", seed);
	fflush(stdout);
	/*------------------------------------------------------------------------------*/

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], classesVeic, numVeic, numRequisicoes);

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
	int limVeic, iVeicForn, iVeicCons, iVeicDirect = 0 ;
	int* pricingFullForn = new int[numVeic];
	int* pricingFullCons = new int[numVeic];

	//Declara um ponteiro para as colunas que serao retornadas do pricing
	do{
		iVeicForn = 0, iVeicCons = 0;
		memset(pricingFullForn, 0, numVeic*sizeof(int));
		memset(pricingFullCons, 0, numVeic*sizeof(int));

		do{
			pricingFull = false;
			alcancouRaiz = true;
			modCplex.solveMaster();
			limVeic = iVeicForn + numVeic;
			for ( int k = iVeicForn; k < limVeic; ++k, ++iVeicForn )
			{
				if ( pricingFullForn[(k % numVeic)] != -1 )
				{
					modCplex.updateDualCostsForn(G, (k % numVeic));
					colunas = retornaColuna(G, (k % numVeic), modCplex.getAlfaDual(k % numVeic), opcaoSub, true, pricingFull, pricingFullForn[(k % numVeic)]);
					if ( colunas != NULL )
					{
						for ( int i = 0; colunas[i] != NULL; ++i )
						{
							modCplex.insert_ColumnPrimal_Forn(colunas[i], (k % numVeic));
						}

						//Retorna os valores dos vetores para que todos computem os Subproblemas
						for ( int v = 0; v < numVeic; ++v )
						{
							pricingFullForn[v] = pricingFullCons[v] = 1;
						}
						
						modCplex.solveMaster();
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
					colunas = retornaColuna(G, (k % numVeic), modCplex.getBetaDual(k % numVeic), opcaoSub, false, pricingFull, pricingFullCons[(k % numVeic)]);
					if ( colunas != NULL )
					{
						for ( int i = 0; colunas[i] != NULL; ++i )
						{
							modCplex.insert_ColumnPrimal_Cons(colunas[i], (k % numVeic));
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

		modCplex.solveMaster();
		limVeic = iVeicDirect + numVeic;
		for ( int k = iVeicDirect; k < limVeic; ++k, ++iVeicDirect )
		{
			modCplex.updateDualCostsDirect(G, (k % numVeic));
			colunas = retornaColuna(G, (k % numVeic), modCplex.getBetaDual(k % numVeic) + modCplex.getAlfaDual(k % numVeic), opcaoSubRD, tempo);
			if ( colunas != NULL )
			{
				for ( int i = 0; colunas[i] != NULL; ++i )
				{
					modCplex.insert_ColumnPrimal_Direct(colunas[i], (k % numVeic));
				}
				alcancouRaiz = false;
				++iVeicDirect;
				break;
			}
		}
		if ( ( time(0) - tempo ) > 10800 ) { printf("Time Limit!\n"); exit(0); }
	}while ( !alcancouRaiz );

	modCplex.atualizaRotasBasicas();
	float lDual = modCplex.solveMaster();
	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t -\t\t\t -\t\t%ld\n", lDual, time(0)-tempo);	
	fflush(stdout);

	ModeloHeuristica* modHeuristica = new ModeloHeuristica(modCplex);
	float lPrimal = modHeuristica->optimize();
	ModeloCplex::setLimitePrimal(lPrimal);
	delete modHeuristica;

	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);
	fflush(stdout);	

	//A partir deste ponto, a raiz do master foi alcançada e deve ser realizado o branch-and-price
	//A raiz do master representara o primeiro no da arvore, que sera ramificada com a insercao das
	//restricoes de branching. Cada no da arvore dara origem a 7 outros nos, cujas restricoes serao
	//escritas em funcao do da manipulacao das mercadorias entre fornecedores/consumidores,
	//alem das trocas no CD
	ListaNosAtivos arvoreBranching;
	ptrNo aux, noAtual;
	int noCount = 1;

	bool bestPrimal;
	noAtual = new NoArvore(modCplex);
	while ( noAtual != NULL )
	{
		if ( noCount > 1 )
		{
			lDual = noAtual->getLimiteDual();
			lPrimal = ModeloCplex::getLimitePrimal();
			printf(" %d\t\t  %d\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", noCount, NoArvore::getTotalNosArmazenados(), 
																		lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);
			fflush(stdout);
		}
		
		//gera-se 5 filhos: 2 considerando (tau^k_i = 1) e 3 para (tau^k_i = 0)
		for ( int i = 1; i <= 5; ++i )
		{
			//o metodo geraFilho() iniciara um novo no (que tera as restricoes de branching pertinentes ao filho)
			//e solucionara sua relaxacao ate a raiz, sendo que para isto novas colunas podem ser geradas, usando
			//os custos duais da relaxacao do modelo. Caso o no possa ser podado, ele nao eh inserido na arvore

			//bestPrimal eh passado por referencia, pois a a lista de nos nao pode ser acessada dentro da classe NoArvore (referencia cruzada)
			aux = noAtual->geraFilho(modCplex, G, i, opcaoSub, opcaoSubRD, tempo, bestPrimal);
			if ( aux != NULL )
			{
				arvoreBranching.insereNo(aux);
				if ( bestPrimal ) arvoreBranching.podaPorLimiteDual(ModeloCplex::getLimitePrimal());
			}

			modCplex.limpaVariaveisArvoreB1();
		}

		delete noAtual;
		noAtual = arvoreBranching.retornaProximo();
		++noCount;

		//Se o tempo de execucao exceder 3 horas, mostra a ultima solucao e termina
		if ( ( time(0) - tempo ) > 10800 ) break;
	}

	delete G;
	return 0;
}

