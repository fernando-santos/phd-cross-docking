#include <sys/time.h>
#include <iostream>
#include "Grafo.h"
#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
using namespace std;

int main(int argc, char** argv){

	if ((argc < 5) || (argc > 8)){
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Numero de veiculos a serem obrigatoriamente usados\n"
				<< "  (4) Custo para trocar mercadorias no Cross-Docking\n"
				<< "  (5) Semente para numeros aleatorios (opcional - default time(0))\n\n";
		exit(0);
	}

	int numRotas;
	int numRequisicoes = atoi(argv[2]);
	int numVeic = atoi(argv[3]);
	int custoTroca = atoi(argv[4]);
	int seed = (argc > 5) ? atoi(argv[5]) : time(0);
	srand(seed);

//	printf("%s\n", argv[1]);
//	printf("numReqs = %d\n", numRequisicoes);
//	printf("numVeic = %d\n", numVeic);
//	printf("custoTroca = %d\n", custoTroca);
//	printf("seed = %d\n\n", seed);
//	fflush(stdout);
	

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], numRequisicoes);
	Rota** rotasIniciais = geraRotasBasicas(G, numVeic);

	//Insere as rotas no modelo para realizar a geracao de colunas
	ModeloCplex* modCplex = new ModeloCplex(G, rotasIniciais, numVeic, custoTroca, 0);

	//Libera memoria alocada para armazenar as rotas
	delete [] rotasIniciais;

	//variaveis necessarias para calcular a raiz e calcular o tempo de execucao do algoritmo
	int tmp, aux;
	bool alcancouRaiz;
	do{
		alcancouRaiz = true;
		modCplex->solveMaster();
		modCplex->updateDualCosts(G);

		GRASP grasp(G, numRequisicoes, 2*numRequisicoes+1, modCplex->getXi(), ( (float) ( rand()%4 ) ) / 10.0 + 0.3 );
		aux = grasp.run(8000, modCplex->getAlfa());
		if (aux > 0)
		{
			alcancouRaiz = false;
			for (int h = 0; h < aux; ++h) modCplex->insert_ColumnPrimal_RowDual(grasp.getRotaConvertida(h));
		}
	}while(!alcancouRaiz);

	ModeloHeuristica* modHeuristica = new ModeloHeuristica(*modCplex);
	printf("%0.2f\n", modHeuristica->optimize());
	modHeuristica->imprimeRotasOtimas(*modCplex);
	printf("\n\n");

	delete modHeuristica;
	delete modCplex;
	delete G;
	return 0;
}
