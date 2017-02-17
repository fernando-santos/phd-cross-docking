#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
using namespace std;

int main(int argc, char** argv){

	if (argc < 5)
	{
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Numero de veiculos a serem usados\n"
				<< "  (4) Custo para carregar OU descarregar mercadorias no Cross-Docking\n"
				<< "  (5) Semente de números aleatórios (OPCIONAL - default 'time(0)')\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	int numVeic = atoi(argv[3]);
	int custoTroca = atoi(argv[4]);
	int seed = ( argc > 5 ) ? atoi(argv[5]) : time(0);
	bool pricingFull, alcancouRaiz;
	srand(seed);
	
	/*-------------------EXIBE AS INFORMACOES SOBRE O PROBLEMA----------------------*/
	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("numVeic = %d \n", numVeic);
	printf("custoTroca = %d\n", custoTroca);
	printf("seed = %d\n\n", seed);
	/*------------------------------------------------------------------------------*/

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], numRequisicoes);

	//Calcula-se as rotas  (solucao viavel e pool de rotas do programa java) para os FORNECEDORES
	Rota** rotasFornViavel = geraPoolRotas(G, true, numVeic);

	//Calcula-se as rotas  (solucao viavel e pool de rotas do programa java) para os CONSUMIDORES
	Rota** rotasConsViavel = geraPoolRotas(G, false, numVeic);
	
	for (int i = 0; i < numVeic; ++i)
	{
		printf("lambda[%d]", i); rotasFornViavel[i]->imprimir();
		printf("gamma[%d]", i);rotasConsViavel[i]->imprimir();
	}
	

	ModeloCplex modCplex(G, rotasFornViavel, rotasConsViavel, numVeic, numVeic, numVeic, numRequisicoes, custoTroca);

	//Libera memoria das rotas alocadas para fornecedores e consumidores
	delete [] rotasFornViavel;
	delete [] rotasConsViavel;

	Rota** colunas;
	int numPricingFull, tempo = time(0);
	int* pricingFullForn = new int[numVeic];
	int* pricingFullCons = new int[numVeic];

	//Declara um ponteiro para as colunas que serao retornadas do pricing 
	alcancouRaiz = false;
	int k, limVeic, iVeicForn = 0, iVeicCons = 0, numIt = 0;
	do{
		pricingFull = alcancouRaiz;
		alcancouRaiz = true;

		//roda o cplex, soluciona a relaxacao do master restrito e cria o modelo dual para a estabilizacao		
		printf("%f\n", modCplex.solveMaster());

		//atualiza o grafo com os custos duais dos vertices, calcula as rotas e as insere no master restrito
		limVeic = iVeicForn + numVeic;
		for ( int k = iVeicForn; k < limVeic; ++k, ++iVeicForn )
		{
			if ( pricingFullForn[(k % numVeic)] != -1 )
			{
				modCplex.updateDualCostsForn(G, (k % numVeic));
				float vS = modCplex.getValorSubtrairForn(k % numVeic);
				colunas = retornaColuna(G, true, pricingFull, vS, 'E', pricingFullForn[(k % numVeic)]);
				if ( colunas != NULL )
				{
					for ( int i = 0; colunas[i] != NULL; ++i )
					{
						//printf("k[%d]", (k % numVeic)); colunas[i]->imprimir();
						modCplex.insertColumnPrimalForn(colunas[i], (k % numVeic));
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

		modCplex.solveMaster();
		limVeic = iVeicCons + numVeic;
		for ( int k = iVeicCons; k < limVeic; ++k, ++iVeicCons )
		{
			if ( pricingFullCons[(k % numVeic)] != -1 )
			{
				modCplex.updateDualCostsCons(G, (k % numVeic));
				colunas = retornaColuna(G, false, pricingFull, modCplex.getValorSubtrairCons(k % numVeic), 'E', pricingFullCons[(k % numVeic)]);
				if ( colunas != NULL )
				{
					for ( int i = 0; colunas[i] != NULL; ++i )
					{
						modCplex.insertColumnPrimalCons(colunas[i], (k % numVeic));
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
		
//		if (++numIt > 3) break;

	}while( ( !alcancouRaiz ) || ( numPricingFull != ( -2*numVeic ) ) );

	modCplex.exportModel("export.lp");

	float lDual = modCplex.solveMaster();
	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t -\t\t\t -\t\t%ld\n", lDual, time(0)-tempo);
	ModeloHeuristica* modHeuristica = new ModeloHeuristica(modCplex);
	float lPrimal = modHeuristica->optimize();
	ModeloCplex::setLimitePrimal(lPrimal);
	delete modHeuristica;

	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);

	delete G;
	return 0;
}
