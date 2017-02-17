#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
using namespace std;

int main(int argc, char** argv){

	if ((argc < 7) || (argc > 11)){
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Numero de veiculos a serem usados na classe '=' (veículos tem a mesma capacidade da instancia original)\n"
				<< "  (4) Numero de veiculos a serem usados na classe '+' (veículos tem a capacidade da instancia original + 50%)\n"
				<< "  (5) Numero de veiculos a serem usados na classe '-' (veículos tem a capacidade da instancia original - 50%)\n"
				<< "  (6) Custo para carregar OU descarregar mercadorias no Cross-Docking\n"
				<< "  (7) Opcao para solucao do subproblema {E (Elementar), S (StateSpaceRelax)} (OPCIONAL - default 'E')\n"
				<< "  (8) Opcao para estrategia de branching {T (Tau), A (Arcos)} (OPCIONAL - default 'T')\n"
				<< "  (9) Quantidade de pontos interiores para a estabilizacao (OPCIONAL - default '0')\n"
				<< "  (10) Semente de números aleatórios (OPCIONAL - default 'time(0)')\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	int numVeicClasse1 = atoi(argv[3]);
	int numVeicClasse2 = atoi(argv[4]);
	int numVeicClasse3 = atoi(argv[5]);
	int numVeic = numVeicClasse1 + numVeicClasse2 + numVeicClasse3;
	if ( ( numVeicClasse1 < 0 ) || ( numVeicClasse2 < 0 ) || ( numVeicClasse3 < 0) || 
		( numVeic > ( numRequisicoes / 2 ) ) || ( numVeic == 0 ) )
	{
		cout << "Numero de veículos para uma (ou mais) classe(s) é inválido!\n";
		exit(0);
	}
	Grafo::classesVeic = new int[numVeic];
	for (int i = 0; i < numVeicClasse1; ++i)
	{
		Grafo::classesVeic[i] = 0;
	}
	for (int i = 0; i < numVeicClasse2; ++i)
	{
		Grafo::classesVeic[numVeicClasse1+i] = 1;
	}
	for (int i = 0; i < numVeicClasse3; ++i)
	{
		Grafo::classesVeic[numVeicClasse1+numVeicClasse2+i] = 2;
	}

	int custoTroca = atoi(argv[6]);
	if ( custoTroca < 0 )
	{
		cout << "O valor para custo de troca das mercadorias deve ser um valor não-negativo\n" << endl;
		exit(0);
	}

	char opcaoSub = (argc > 7) ? argv[7][0] : 'E';
	if ( ( opcaoSub != 'E' ) && ( opcaoSub != 'S') )
	{
		cout << "A opcao para solucao do subproblema deve ser {E, S}\n" << endl;
		exit(0);
	}
	char opcaoBranching = (argc > 8) ? argv[8][0] : 'T';
	if ( ( opcaoBranching != 'T' ) && ( opcaoBranching != 'A') )
	{
		cout << "A opcao para estratégia de branching deve ser {T, A}\n" << endl;
		exit(0);
	}
	
	int numPontosExtremosDual = (argc > 9) ? atoi(argv[9]) : 0;
	if ( ( numPontosExtremosDual < 0 ) || ( numPontosExtremosDual > 1000 ) )
	{
		cout << "O número de pontos extremos deve estar no intervalo [0, 1000]\n" << endl;
		exit(0);
	}

	int seed = ( argc > 10 ) ? atoi(argv[10]) : time(0);
	bool pricingFull, alcancouRaiz, terminouAntes = false;
	int numRotasCons, numRotasForn;
	srand(seed);
	
	/*-------------------EXIBE AS INFORMACOES SOBRE O PROBLEMA----------------------*/
	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("numVeic = %d {%d, %d, %d}\n", numVeic, numVeicClasse1, numVeicClasse2, numVeicClasse3);
	printf("custoTroca = %d\n", custoTroca);
	printf("Sub, Branching, NumPontosInt = {%c, %c, %d}\n", opcaoSub, opcaoBranching, numPontosExtremosDual);
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
	
	modCplex.exportModel("m1.lp");

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
		modCplex.atualizaRotasBasicas();

		//atualiza o grafo com os custos duais dos vertices, calcula as rotas e as insere no master restrito
		limVeic = iVeicForn + numVeic;
		for ( int k = iVeicForn; k < limVeic; ++k, ++iVeicForn )
		{
			if ( pricingFullForn[(k % numVeic)] != -1 )
			{
				modCplex.updateDualCostsForn(G, (k % numVeic));
				float vS = modCplex.getValorSubtrairForn(k % numVeic);
				colunas = retornaColuna(G, (k % numVeic), true, pricingFull, vS, opcaoSub, pricingFullForn[(k % numVeic)]);
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
				colunas = retornaColuna(G, (k % numVeic), false, pricingFull, modCplex.getValorSubtrairCons(k % numVeic), opcaoSub, pricingFullCons[(k % numVeic)]);
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
	
	modCplex.exportModel("m2.lp");
	
	modCplex.imprimeRotasBasicas();

	float lDual = modCplex.solveMaster();
	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t -\t\t\t -\t\t%ld\n", lDual, time(0)-tempo);
/*	ModeloHeuristica* modHeuristica = new ModeloHeuristica(modCplex);
	float lPrimal = modHeuristica->optimize();
	ModeloCplex::setLimitePrimal(lPrimal);
	delete modHeuristica;

	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f%\t\t%ld\n", lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);*/

	delete G;
	return 0;
}
