#include "Grafo.h"
#include "ElementarCplex.h"

int main(int argc, char** argv){
	if (argc != 5){
		cout << "Parametros esperados:\n"
			<< "  (1) Arquivo de instancias\n"
			<< "  (2) Quantidade de requisicoes\n"
			<< "  (3) Limite de custo dual em cada vertice (valor positivo)\n"
			<< "  (4) Semente para numeros aleatorios \n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	float* classesVeic = new float[1];
	classesVeic[0] = 1;
	int numVeic = 1;
	

	srand(atoi(argv[4]));
	Grafo* G = new Grafo(argv[1], classesVeic, numVeic, numRequisicoes);
	for (int i = 1; i <= 2*numRequisicoes; ++i) G->setCustoVerticeDual( i , - ( rand() % atoi( argv[3] ) ) );
	G->setCustoArestasDual(0);
	ElementarCplex camElem(G);
	camElem.calculaCaminhoElementar();
	camElem.getRotaCustoMinimo(G)->imprimir();
}
