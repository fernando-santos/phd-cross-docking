#include "ElementarM2.h"

int main(int argc, char** argv){
	if ( ( argc != 5 ) && ( argc != 6 ) )
	{
		cout << "Parametros esperados:\n"
			<< "  (1) Arquivo de instancias\n"
			<< "  (2) Quantidade de requisicoes\n"
			<< "  (3) Custo para trocar mercadorias no Cross-Docking\n"
			<< "  (4) Valor que define o intervalo de custo dual nos vertices (valor aleatorio entre [-x, x])\n"
			<< "  (5) Semente de numeros aleatorios (opcional : default = time(0))\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	float* custoTroca = new float[numRequisicoes+1]; custoTroca[0] = 0;
	for (int i = 1; i <= numRequisicoes; ++i) custoTroca[i] = atof(argv[3]);

	int baseIntervaloDual = atoi(argv[4]);
	int seed = (argc > 5) ? atoi(argv[5]) : time(0);
	srand(seed);

	Grafo* G = new Grafo(argv[1], numRequisicoes); 
	float* custosDuais = new float[2*numRequisicoes +1];
	for ( int i = 1; i <= 2*numRequisicoes; ++i )
	{
		custosDuais[i] = -( rand() % ( baseIntervaloDual + 1 ) );
		if ( i <= numRequisicoes ) G->setCustoVerticeDual( i , custosDuais[i] );
		else G->setCustoVerticeDual( i , 2*custosDuais[i] );
	}
	G->setCustoArestasDual();

	Elementar pd(G, custoTroca, MAIS_INFINITO);
	pd.calculaCaminhoElementar(G);
	Rota* r = pd.getRotaCustoMinimo(G);
	r->imprimir();

	delete G;
	delete [] custoTroca;
	delete [] custosDuais;
}
