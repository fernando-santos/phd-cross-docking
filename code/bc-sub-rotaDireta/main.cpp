#include "ModeloBC-RD.h"

int main(int argc, char** argv){
	if ( ( argc != 4 ) && ( argc != 5 ) )
	{
		cout << "Parametros esperados:\n"
			<< "  (1) Arquivo de instancias\n"
			<< "  (2) Quantidade de requisicoes\n"
			<< "  (3) Valor (x) que define o intervalo de custo dual nos vertices (valor aleatorio entre [-x, 0])\n"
			<< "  (4) Semente de numeros aleatorios (opcional : default = time(0))\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	int baseIntervaloDual = atoi(argv[3]);
	int seed = (argc > 4) ? atoi(argv[4]) : time(0);
	srand(seed);

	Grafo* G = new Grafo(argv[1], numRequisicoes);
	for ( int i = 1; i <= 2*numRequisicoes; ++i ) G->setCustoVerticeDual( i , -( rand() % ( baseIntervaloDual + 1 ) ) );
	G->setCustoArestasDual();

	printf("build\n");

	ModeloBC bc( G, MAIS_INFINITO );
	
	printf("exec\n");
	
	bc.calculaCaminhoElementar( G );
	
	printf("finish\n");
	
	int aux = bc.rotasNegativas.size();
	for ( int i = 0; i < aux; ++i )
	{
		bc.rotasNegativas[i]->imprimir();
	}
	
	delete G;
}
