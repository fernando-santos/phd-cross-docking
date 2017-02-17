#include "ModeloCplexI.h"
#include "ModeloCplexR.h"
#include "Grafo.h"
using namespace std;

int main(int argc, char** argv){

	if (argc != 7){
		 cout	<< "Parametros esperados:\n  (1) Arquivo de instancias Solomon \n"
				<< "  (2) Quantidade de Fornecedores (e consequentemente Consumidores) a considerar\n"
				<< "  (3) Quantidade de veículos a serem utilizados na solução (K)\n"
				<< "  (4) Custo para carregar E descarregar mercadorias no Cross-Docking\n"
				<< "  (5) Opcao de otimizacao {I - inteira, R - real}\n"
				<< "  (6) Arquivo de saida (*.lp)\n\n";
		exit(0);
	}

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], atoi(argv[2]));

	if (argv[5][0] == 'I'){
		ModeloCplexI modCplexI(G, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), argv[6]);
	}else if (argv[5][0] == 'R'){
		ModeloCplexR modCplexR(G, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), argv[6]);
	}else{
		cout << "Parametro (5) incorreto! Use apeans 'I' ou 'R' ao inves de '" << argv[5] << "'" << endl;
		delete G;
		exit(0);
	}

	delete G;
	return 0;
}

