#include "ModeloCplexI.h"
#include "Grafo.h"
using namespace std;

int main(int argc, char** argv){

	if (argc != 5){
		 cout	<< "Parametros esperados:\n  (1) Arquivo de instancias Solomon \n"
				<< "  (2) Quantidade de Fornecedores (e consequentemente Consumidores) a considerar\n"
				<< "  (3) Quantidade de veículos a serem utilizados na solução (K)\n"
				<< "  (4) Arquivo de saida (*.lp)\n\n";
		exit(0);
	}

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], atoi(argv[2]));

	ModeloCplexI modCplexI(G, atoi(argv[2]), atoi(argv[3]), argv[4]);

	delete G;
	return 0;
}
