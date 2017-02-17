#include "Grafo.h"
#include "GRASP.h"
#include "ESPPRCbi.h"
#include <iostream>
#include <time.h>
using namespace std;

int main(int argc, char **argv)
{
	if ( argc < 5 )
	{
		cout << "Parametros esperados:\n"
			<< "  (1) Arquivo de instancias\n"
			<< "  (2) Quantidade de requisicoes\n"
			<< "  (3) Custo para trocar mercadorias no Cross-Docking\n"
			<< "  (4) Semente para numeros aleatorios\n\n";
		exit(0);
	}

	int numRequisicoes = atoi(argv[2]);
	int custoTroca = atoi(argv[3]);
	int seed = atoi(argv[4]);
	srand(seed);

	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("custoTroca = %d\n", custoTroca);
	printf("seed = %d\n\n", seed);
	fflush(stdout);

	Grafo* G = new Grafo(argv[1], numRequisicoes, 'm');
	for ( int i = 1; i < 2*numRequisicoes; ++i ) G->setCustoVerticeDual(i, -rand()%50);
	G->setCustoArestasDual(NULL, 0, false);

	float* vetorXi = new float[numRequisicoes+1];
	for ( int i = 1; i <= numRequisicoes; ++i ) vetorXi[i] = custoTroca;
	ESPPRCbi *bi = new ESPPRCbi(G, vetorXi, 9999);
	bi->calculaCaminhoElementarBi(G);
	Rota** rr = bi->getRotaCustoMinimo(G, 0.99);

	float custoMenorRota = 99999;
	for ( int i = 0; rr[i] != NULL; ++i )
	{
		if ( rr[i]->getCustoReduzido() < custoMenorRota ) custoMenorRota = rr[i]->getCustoReduzido();
		rr[i]->imprimir();		
	}
	printf("custoMenorRota = %0.2f\n", custoMenorRota);

	for ( int i = 0; i < 200; ++i )
	{
		struct timeval tv;
		struct timezone tz;
		struct tm *tm;
		int hora, min, seg, micro, tempo;
		float tempoTotal;
		gettimeofday(&tv, &tz);
		tm=localtime(&tv.tv_sec);
		hora = tm->tm_hour;
		min = tm->tm_min;
		seg = tm->tm_sec;
		micro = tv.tv_usec;

		GRASP* grasp = new GRASP(G, numRequisicoes, 2*numRequisicoes+1, vetorXi, 9999);
		grasp->run(custoMenorRota);

		gettimeofday(&tv, &tz);
		tm=localtime(&tv.tv_sec);
		tempo = (tm->tm_hour - hora) * 3600;
		tempo += (tm->tm_min - min) * 60;
		tempo += tm->tm_sec - seg;
		tempo += tempo * 1000000;
		tempo += tv.tv_usec - micro;
		tempoTotal = tempo;
		tempoTotal /= 1000000;
		printf("%0.2f\n", tempoTotal);
	}

	delete [] vetorXi;
	delete G;
}
