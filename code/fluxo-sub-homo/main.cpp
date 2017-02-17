#include "Grafo.h"
#include "ElementarCplex.h"

int main(int argc, char** argv){
	if (argc != 6){
		cout << "Parametros esperados:\n"
			<< "  (1) Arquivo de instancias\n"
			<< "  (2) Quantidade de requisicoes\n"
			<< "  (3) Custo para trocar mercadorias no Cross-Docking\n"
			<< "  (4) Limite de custo dual em cada vertice (valor positivo)\n"
			<< "  (5) Semente para numeros aleatorios\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	int custoTroca = atoi(argv[3]);

	srand(atoi(argv[5]));
	Grafo* G = new Grafo(argv[1], numRequisicoes);
	for (int i = 1; i <= 2*numRequisicoes; ++i) G->setCustoVerticeDual( i , - ( rand() % atoi( argv[4] ) ) );
	G->setCustoArestasDual();

	ElementarCplex camElem(G, custoTroca);
	camElem.calculaCaminhoElementar();
	camElem.getRotaCustoMinimo(G)->imprimir();
}
