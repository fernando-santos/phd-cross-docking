#include <sys/time.h>
#include <iostream>
#include "Grafo.h"
#include "NoArvore.h"
#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ListaNosAtivos.h"
#include "ModeloHeuristica.h"
using namespace std;

int main(int argc, char** argv){

	if ((argc < 5) || (argc > 8)){
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Numero de veiculos a serem obrigatoriamente usados\n"
				<< "  (4) Custo para trocar mercadorias no Cross-Docking\n"
				<< "  (5) Opcao para solucao do subproblema {P, B} (opcional - default P)\n"
				<< "  (6) Frequencia para usar heuristica de GC na arvore {1 - 999999 (default)} (opcional)\n"
				<< "  (7) Semente para numeros aleatorios (opcional - default time(0))\n\n";
		exit(0);
	}

	//PARAMETROS OPCIONAIS (ou respectivos valores default) - 1328209686
	//-----------------------------------------------------------------------------------
	char opcaoSub = 'P';
	if (argc > 5) opcaoSub = argv[5][0];
	if ((opcaoSub != 'P') && (opcaoSub != 'B')){
		printf("A opcao para solucao do subproblema deve ser {P, B}\n");
		exit(0);
	}

	int frequenciaHeuristica = 1;
	if (argc > 6) frequenciaHeuristica = atoi(argv[6]);
	if (frequenciaHeuristica <= 0)
	{
		printf("A frequencia para execucao da heuristica de GC deve ser maior que zero\n");
		exit(0);
	}

	int seed = (argc > 7) ? atoi(argv[7]) : time(0);
	//-----------------------------------------------------------------------------------


	//PARAMETROS OBRIGATORIOS
	//-----------------------------------------------------------------------------------
	int numRotas;
	int numRequisicoes = atoi(argv[2]);
	int numVeic = atoi(argv[3]);
	int custoTroca = atoi(argv[4]);
	srand(seed);
	//-----------------------------------------------------------------------------------

	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("numVeic = %d\n", numVeic);
	printf("custoTroca = %d\n", custoTroca);
	printf("opcaoSub = %c\n", opcaoSub);
	printf("freqHeuristica = %d\n", frequenciaHeuristica);
	printf("seed = %d\n\n", seed);
	fflush(stdout);
	

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	float solPricing, opt;
	long int tempoTotal = time(0);
	Grafo::timeInicio = tempoTotal;
	Grafo::timeLimit = 14400;
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
		opt = modCplex->solveMaster();
		printf("%f [%f]\n", opt, opt + numVeic*solPricing);
		fflush(stdout);
		modCplex->updateDualCosts(G);

		if ( opcaoSub == 'B' )
		{
			GRASP grasp(G, numRequisicoes, 2*numRequisicoes+1, modCplex->getXi(), 0.5);
			aux = grasp.run(2000, modCplex->getAlfa());
			if (aux > 0)
			{
				alcancouRaiz = false;
				for (int h = 0; h < aux; ++h) modCplex->insert_ColumnPrimal_RowDual(grasp.getRotaConvertida(h));
			}
			else
			{
				ModeloBC BCE(G, modCplex->getXi(), modCplex->getAlfa());
				BCE.calculaCaminhoElementar(G);
				aux = BCE.rotasNegativas.size();

				if (aux > 0) //significa que encontrou rota(s) de custo reduzido negativo
				{
					alcancouRaiz = false;
					solPricing = ( BCE.cplex.getBestObjValue() - modCplex->getAlfa() );
					for (int y = 0; y < aux; ++y) modCplex->insert_ColumnPrimal_RowDual(BCE.rotasNegativas[y]);
				}
			}
		}
		else
		{
			ESPPRC *eee = new ESPPRC(G, modCplex->getXi(), modCplex->getAlfa());
			eee->calculaCaminhoElementar(G);
			Rota** rr = eee->getRotaCustoMinimo(G, 0.7);
			if ( rr != NULL )
			{
				alcancouRaiz = false;
				solPricing = 9999999;
				for ( int y = 0; rr[y] != NULL; ++y )
				{
					modCplex->insert_ColumnPrimal_RowDual(rr[y]);
					if ( rr[y]->getCustoReduzido() < solPricing ) solPricing = rr[y]->getCustoReduzido();
				}
				solPricing -= modCplex->getAlfa();
			}
			delete eee;
		}
	}while(!alcancouRaiz);

	float lD = modCplex->solveMaster();
	ModeloHeuristica* modHeuristica = new ModeloHeuristica(*modCplex);
	float lP = modHeuristica->optimize();
	ModeloCplex::setLimitePrimal(lP);
	delete modHeuristica;

	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", lD, lP, ((lP - lD) / lP) * 100, time(0)-tempoTotal);
	fflush(stdout);

	//A partir deste ponto, a raiz do master foi alcançada e deve ser realizado o branch-and-price
	//A raiz do master representara o primeiro no da arvore, que sera ramificada com a insercao das
	//restricoes de branching. Sera feito um branching em arcos, estendendo a formulacao por rotas
	//Os custos de troca serao uma consequencia das rotas obtidas pelo branching em arcos
	ptrNo noAtual;
	vector < ptrNo > novosNos;
	ListaNosAtivos arvBranching;
	NoArvore *raiz = new NoArvore(modCplex, G, novosNos, lD);
	for (int i = 0; i < novosNos.size(); ++i) arvBranching.insereNo(novosNos[i]);
	delete raiz; //o no raiz serve apenas para inicializar os atributos estaticos e criar o(s) primeiro(s) no(s) -- caso nao seja inteiro --

	int it = 1;
	while(!arvBranching.vazia()){
		novosNos.clear();
		noAtual = arvBranching.retornaProximo();

		lD = noAtual->getLimiteDual();
		lP = ModeloCplex::getLimitePrimal();
		printf(" %d\t\t  %d\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", ++it, NoArvore::getTotalNosAtivos(), lD, lP, ((lP - lD) / lP) * 100, time(0)-tempoTotal);
		fflush(stdout);

		lP = noAtual->executaBranching(novosNos, opcaoSub);
		if (lP < ModeloCplex::getLimitePrimal())
		{
			arvBranching.podaPorLimiteDual(lP);
			ModeloCplex::setLimitePrimal(lP);

		} else if ( (it % frequenciaHeuristica) == 0 ){ //executa a heuristica caso a solucao nao seja inteira (respeitando a frequencia)
			modHeuristica = new ModeloHeuristica(*modCplex, noAtual);
			float result = modHeuristica->optimize();
			if (result < ModeloCplex::getLimitePrimal())
			{
				arvBranching.podaPorLimiteDual(result);
				ModeloCplex::setLimitePrimal(result);
			}
			delete modHeuristica;
		}

		for (int i = 0; i < novosNos.size(); ++i) arvBranching.insereNo(novosNos[i]);
		delete noAtual;

		if ((time(0) - tempoTotal) > 14400) break;
	}

	delete modCplex;
	delete G;
	return 0;
}
