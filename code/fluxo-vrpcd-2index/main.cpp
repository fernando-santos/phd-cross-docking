#include "ModeloCplexI.h"
#include "ModeloCplexR.h"
#include "Grafo.h"
using namespace std;

int main(int argc, char** argv){

	if (argc < 6)
	{
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Custo para carregar/descarregar mercadorias no Cross-Docking\n"
				<< "  (4) Numero de veiculos\n"
				<< "  (...) Capacidade relativa (1 = capacidade original) de cada veiculo determinado em argv[4]\n"
				<< "  (argv[4] + 5) Arquivo de saida '*.lp' para exportar o modelo (OPCIONAL - DEFAULT SEM EXPORTAR MODELO)\n"
				<< "  (argv[4] + 6) Opcao de otimizacao {I - inteira, R - real} (OPCIONAL - DEFAULT 'I')\n"
				<< "  (argv[4] + 7) Tempo do veiculo no Cross-Docking para a consolidacao (OPCIONAL - DEFAULT SEM JANELAS DE TEMPO)\n\n";
		exit(0);
	}
	
	int numRequisicoes = atoi(argv[2]);
	int custoTroca = atoi(argv[3]);
	if ( custoTroca < 0 )
	{
		cout << "O valor para custo de troca das mercadorias deve ser um valor não-negativo\n" << endl;
		exit(0);
	}
	
	int numVeic = atoi(argv[4]);
	if ( argc < ( numVeic + 5 ) )
	{
		cout << "Numero de veículos e suas respectivas capacidades incompatíveis\n";
		exit(0);
	}
	float* classesVeic = new float[numVeic];
	for (int i = 0; i < numVeic; ++i)
	{
		classesVeic[i] = atof(argv[5+i]);
	}

	char *arquivo = ( argc > ( numVeic + 5 ) ) ? argv[numVeic+5] : NULL;
	char opOpt = ( argc > ( numVeic + 6 ) ) ? argv[numVeic+6][0] : 'I';
	int tempoCrossDocking = ( argc > ( numVeic + 7 ) ) ? atoi(argv[numVeic+7]) : 0;


	/*-------------------EXIBE AS INFORMACOES SOBRE O PROBLEMA----------------------*/
	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("numVeic = %d {%f", numVeic, classesVeic[0]);
	for (int i = 1; i < numVeic; ++i) printf(", %f", classesVeic[i]);
	printf("}\ncustoTroca = %d\n", custoTroca);
	printf("opcaoOtimizacao = %c\n", opOpt);
	printf("Janela tempo Cross-Docking = %d\n\n", tempoCrossDocking);
	/*------------------------------------------------------------------------------*/

	Grafo* G = new Grafo(argv[1], numRequisicoes);

	if (opOpt == 'I')
	{
		ModeloCplexI modCplexI(G, numRequisicoes, numVeic, custoTroca, tempoCrossDocking, arquivo, classesVeic);
		int tempo = time(0);
		cout << "Solucao = " << modCplexI.optimize(14400) << endl;
		cout << "Tempo = " << time(0) - tempo << endl << endl << endl;
	}
	else if (opOpt == 'R')
	{
		ModeloCplexR modCplexR(G, numRequisicoes, numVeic, custoTroca, tempoCrossDocking, arquivo, classesVeic);
		int tempo = time(0);
		cout << "Solucao = " << modCplexR.optimize(14400) << endl;
		cout << "Tempo = " << time(0) - tempo << endl << endl << endl;
	}

	delete G;
	return 0;
}
