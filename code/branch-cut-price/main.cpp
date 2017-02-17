#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
using namespace std;

int main(int argc, char** argv){

	if ( ( argc < 5 ) || ( argc > 7 ) )
	{
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Numero de veiculos a serem obrigatoriamente usados\n"
				<< "  (4) Custo para trocar mercadorias no Cross-Docking\n"
				<< "  (5) Opcao para solucao do subproblema { E, S } (default - 'E')\n"
				<< "  (6) Semente de numeros aleatorios (default - time(0))\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	int numVeic = atoi(argv[3]);
	int custoTroca = atoi(argv[4]);

	char opcaoSub = 'E';
	if ( argc > 5 )
	{
		opcaoSub = argv[5][0];
		if ( ( opcaoSub != 'E' ) && ( opcaoSub != 'S' ) )
		{
			cout << "A opcao para solucao do subproblema deve ser { E, S }" << endl;
			exit(0);
		}
	}

	int seed = time(0);
	if ( argc > 6 )
	{
		seed = atoi(argv[6]);
	}
	srand(seed);
	printf("seed = %d\n", seed);

	Grafo* G = new Grafo(argv[1], numRequisicoes);
	Rota** rotasFornViavel = geraPoolRotas(G, true, numVeic);
	Rota** rotasConsViavel = geraPoolRotas(G, false, numVeic);
	
	for (int i = 0; i < numVeic; ++i) rotasFornViavel[i]->imprimir(); printf("\n");
	for (int i = 0; i < numVeic; ++i) rotasConsViavel[i]->imprimir();
	
	ModeloCplex modCplex(G, rotasFornViavel, rotasConsViavel, numVeic, numVeic, numVeic, numRequisicoes, custoTroca);
	delete [] rotasFornViavel;
	delete [] rotasConsViavel;

	Rota** colunas;
	bool alcancouRaiz;
	int it = 0, tempo = time(0);
	do
	{
		alcancouRaiz = true;

		printf("%f\n", modCplex.solveMaster());
		modCplex.updateDualCostsForn( G );
		colunas = retornaColuna(G, true, modCplex.getAlfaDual(), opcaoSub);
		if (colunas != NULL)
		{	
			for (int i = 0; colunas[i] != NULL; ++i)
			{
				modCplex.insertColumnForn( colunas[i] );
			}
			alcancouRaiz = false;
			delete [] colunas;
		}

		modCplex.solveMaster();
		modCplex.updateDualCostsCons( G );
		colunas = retornaColuna(G, false, modCplex.getBetaDual(), opcaoSub);
		if (colunas != NULL)
		{
			for (int i = 0; colunas[i] != NULL; ++i)
			{
				modCplex.insertColumnCons( colunas[i] );
			}
			alcancouRaiz = false;
			delete [] colunas;
		}
	}
	while( !alcancouRaiz );

	float lDual = modCplex.solveMaster();
	ModeloHeuristica* modHeuristica = new ModeloHeuristica(modCplex);
	float lPrimal = modHeuristica->optimize();
	ModeloCplex::setLimitePrimal(lPrimal);

	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);

	delete modHeuristica;
	delete G;
	return 0;
}

