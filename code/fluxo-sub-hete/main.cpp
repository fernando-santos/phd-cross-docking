#include "ElementarCplex.h"
#include "Grafo.h"
#include <time.h>
#include <sys/time.h>

using namespace std;

int main(int argc, char** argv){
	if ((argc != 3) && (argc != 4)){
		 cout	<< "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias Solomon \n"
				<< "  (2) Quantidade de Fornecedores (ou Consumidores)\n"
				<< "  (3) Janela de Tempo? (OPCIONAL)\n\n";
		exit(0);
	}

	bool jt = (argc == 4) ? true : false;
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	int hora, min, seg, micro, tempo;
	float tempoTotal;

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], atoi(argv[2]));
	for (int i = 1; i < G->getNumVertices()-1; ++i){
		G->setCustoVerticeDual(i, -30);
	}
	G->setCustoArestasDual();

	ElementarCplex::initModel(G);
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
	hora = tm->tm_hour;
	min = tm->tm_min;
	seg = tm->tm_sec;
	micro = tv.tv_usec;
	ElementarCplex::calculaCaminhoElementar(G, true, jt);
	Rota** rF = ElementarCplex::getRotaCustoMinimo(G, 1000);
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
	tempo = (tm->tm_hour - hora) * 3600;
	tempo += (tm->tm_min - min) * 60;
	tempo += tm->tm_sec - seg;
	tempo += tempo * 1000000;
	tempo += tv.tv_usec - micro;
	tempoTotal = tempo;
	tempoTotal /= 1000000;
	printf("%f\n", tempoTotal);
	rF[0]->imprimir();
	ElementarCplex::freeModel();


	ElementarCplex::initModel(G);
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
	hora = tm->tm_hour;
	min = tm->tm_min;
	seg = tm->tm_sec;
	micro = tv.tv_usec;
	ElementarCplex::calculaCaminhoElementar(G, false, jt);
	Rota** rC = ElementarCplex::getRotaCustoMinimo(G, 1000);
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
	tempo = (tm->tm_hour - hora) * 3600;
	tempo += (tm->tm_min - min) * 60;
	tempo += tm->tm_sec - seg;
	tempo += tempo * 1000000;
	tempo += tv.tv_usec - micro;
	tempoTotal = tempo;
	tempoTotal /= 1000000;
	printf("%f\n", tempoTotal);
	rC[0]->imprimir();
	ElementarCplex::freeModel();


	delete G;
	return 0;
}

