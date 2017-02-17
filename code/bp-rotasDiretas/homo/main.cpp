#include <sys/time.h>
#include <iostream>
#include "Grafo.h"
#include "geraRotas.h"
#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
#include "ListaNosAtivos.h"
#include "NoArvore.h"
using namespace std;

int main(int argc, char** argv){

	if ( argc < 5 )
	{
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de requisicoes\n"
				<< "  (3) Numero de veiculos a serem obrigatoriamente usados\n"
				<< "  (4) Custo para trocar mercadorias no Cross-Docking\n"
				<< "  (...) +i para fixar a requisicao i a passar no CD; -i para que i seja atendida por rota direta\n"
				<< "  (.+5) Opcao para solucao do subproblema {P, B} (opcional - default P)\n"
				<< "  (.+6) Opcao para solucao do subproblema Rota Direta {P, B} (opcional - default P)\n"
				<< "  (.+7) Semente para numeros aleatorios (opcional - default time(0))\n\n";
		exit(0);
	}

	//PARAMETROS OBRIGATORIOS
	//-----------------------------------------------------------------------------------
	int numRequisicoes = atoi(argv[2]);
	int numVeic = atoi(argv[3]);
	int custoTroca = atoi(argv[4]);
	//-----------------------------------------------------------------------------------

	//PARAMETROS OPCIONAIS (ou respectivos valores default)
	//-----------------------------------------------------------------------------------
	int numReqsCD = 0, numReqsPD = 0, aux = 5;
	int* vetorReqsCD = new int[numRequisicoes];
	int* vetorReqsPD = new int[numRequisicoes];
	while ( ( aux < argc ) && ( argv[aux][0] != 'P' ) && ( argv[aux][0] != 'B' ) )
	{
		if ( atoi( argv[aux] ) > 0 )
		{
			vetorReqsCD[numReqsCD++] = atoi( argv[aux++] ); 
		}
		else
		{
			vetorReqsPD[numReqsPD++] = -atoi( argv[aux++] );
		}
	}
	for ( int i = 0; i < numReqsCD; ++i )
	{
		if ( vetorReqsCD[i] > numRequisicoes )
		{
			printf("Requisicao (%d) nao existe para esta instancia!\n", vetorReqsCD[i]);
			exit(0);
		}

		for ( int j = 0; j < numReqsPD; ++j )
		{
			if ( vetorReqsPD[i] > numRequisicoes )
			{
				printf("Requisicao (%d) nao existe para esta instancia!\n", vetorReqsPD[i]);
				exit(0);
			}
			if ( vetorReqsCD[i] == vetorReqsPD[j] )
			{
				printf("Requisicao (%d) fixa como CD e PD\n", vetorReqsCD[i]);
				exit(0);
			}
		}
	}

	char opcaoSub = 'P';
	if ( aux < argc ) opcaoSub = argv[aux++][0];
	if ( (opcaoSub != 'P') && (opcaoSub != 'B') )
	{
		printf("A opcao para solucao do subproblema deve ser {P, B}\n");
		exit(0);
	}

	char opcaoSubRD = 'P';
	if ( aux < argc )  opcaoSubRD = argv[aux++][0];
	if ( (opcaoSubRD != 'P') && (opcaoSubRD != 'B') )
	{
		printf("A opcao para solucao do subproblema de Rotas Diretas deve ser {P, B}\n");
		exit(0);
	}

	int seed = time(0);
	if ( aux < argc ) seed = atoi(argv[aux]);
	srand(seed);
	//-----------------------------------------------------------------------------------

	printf("%s\n", argv[1]);
	printf("numReqs = %d\n", numRequisicoes);
	printf("numVeic = %d\n", numVeic);
	printf("custoTroca = %d\n", custoTroca);
	printf("ReqsCD(%d) = ", numReqsCD);
	for ( int i = 0; i < numReqsCD; ++i ) printf("%d ", vetorReqsCD[i]); 
	printf("\nReqsPD(%d) = ", numReqsPD);
	for ( int i = 0; i < numReqsPD; ++i ) printf("%d ", vetorReqsPD[i]);
	printf("\n");
	printf("opcaoSub = %c\n", opcaoSub);
	printf("opcaoSubRD = %c\n", opcaoSubRD);
	printf("seed = %d\n\n", seed);
	fflush(stdout);

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	//if ( ( argv[1][20] != 'm') && ( argv[1][20] != 'r' ) )
	//{
	//	printf("Problemas ao ler instancia (esperada pasta 'mw' ou 'ropke'\n");
	//	exit(0);
	//}
	Grafo* G = new Grafo(argv[1], numRequisicoes, 'm'/*argv[1][20]*/);

	Rota** rotasDiretas;
	Rota** rotasIniciais;
	rotasIniciais = geraRotasBasicas(G, vetorReqsCD, vetorReqsPD, numReqsCD, numReqsPD, numVeic);
	verificaRotasBasicas(G, rotasIniciais, vetorReqsCD, vetorReqsPD, numReqsCD, numReqsPD, numVeic);

	//Insere as rotas no modelo para realizar a geracao de colunas
	ModeloCplex* modCplex = new ModeloCplex(G, rotasIniciais, numVeic, custoTroca, 0);

	//Libera memoria alocada para armazenar as rotas
	delete [] rotasIniciais;

	//variaveis necessarias para calcular a raiz e calcular o tempo de execucao do algoritmo
	bool pricingHeuristica, boundLasdon = false;
	int tmp, pricingRD, pricingCD, itRaiz = 0, tempoTotal = time(0);
	float opt, solPricingCD, solPricingRD, bdLasdon, menorBdLasdon = -9999999;

	//variaveis necessarias para coletar o tempo com precisao
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	int hora, min, seg, micro, tempo;
	//captura o tempo para controlar o processamento do pricing
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
	hora = tm->tm_hour;
	min = tm->tm_min;
	seg = tm->tm_sec;
	micro = tv.tv_usec;


	pricingRD = pricingCD = 1;


	do{
		pricingHeuristica = false;

		if ( numReqsCD < numRequisicoes )
		{
			modCplex->solveMaster();
			modCplex->updateDualCostsRD(G, vetorReqsCD, numReqsCD); //parametros para impedir que requisicoes CD sejam precificadas como PD
			GRASPRD graspRD(G, numRequisicoes, 2*numRequisicoes+1, 0.5);
			aux = 0;//graspRD.run(200*numRequisicoes, modCplex->getAlfa());
			if ( aux > 0 )
			{
				pricingHeuristica = true;
				for (int h = 0; h < aux; ++h) modCplex->insert_ColumnPrimal_RowDual_Direct(graspRD.getRotaConvertida(h));
			}
		}

		if ( numReqsPD < numRequisicoes )
		{
			modCplex->solveMaster();
			modCplex->updateDualCosts(G, vetorReqsPD, numReqsPD); //parametros para impedir que requisicoes PD sejam precificadas como CD
			GRASP grasp(G, numRequisicoes, 2*numRequisicoes+1, modCplex->getXi(), 0.5);
			aux = 0;//grasp.run(200*numRequisicoes, modCplex->getAlfa());
			if (aux > 0)
			{
				pricingHeuristica = true;
				for (int h = 0; h < aux; ++h) modCplex->insert_ColumnPrimal_RowDual(grasp.getRotaConvertida(h));
			}
		}
		if ( pricingHeuristica ) pricingRD = pricingCD = 1;


		if ( ( !pricingHeuristica ) && ( pricingCD == 1 ) && ( numReqsPD < numRequisicoes ) )
		{
			if ( opcaoSub == 'B' )
			{
				ModeloBC BCE(G, modCplex->getXi(), modCplex->getAlfa());
				BCE.calculaCaminhoElementar(G, tempoTotal);
				aux = BCE.rotasNegativas.size();
				if (aux > 0) //significa que encontrou rota(s) de custo reduzido negativo
				{
					solPricingCD = ( BCE.cplex.getBestObjValue() - modCplex->getAlfa() );
					for (int y = 0; y < aux; ++y) modCplex->insert_ColumnPrimal_RowDual(BCE.rotasNegativas[y]);
				}
				else pricingCD = 0;
			}
			else if ( opcaoSub == 'P' )
			{
				ESPPRCbi *bi = new ESPPRCbi(G, modCplex->getXi(), modCplex->getAlfa());
				bi->calculaCaminhoElementarBi(G);
				Rota** rr = bi->getRotaCustoMinimo(G, 0.9);
				if ( rr != NULL )
				{
					solPricingCD = 9999999;
					for ( int y = 0; rr[y] != NULL; ++y )
					{
						modCplex->insert_ColumnPrimal_RowDual(rr[y]);
						if ( rr[y]->getCustoReduzido() < solPricingCD ) solPricingCD = rr[y]->getCustoReduzido();
					}
					solPricingCD -= modCplex->getAlfa();
					delete [] rr;
				}
				else pricingCD = 0;
				delete bi;
			}
		}

		if ( ( !pricingHeuristica ) && ( pricingRD == 1 ) && ( numReqsCD < numRequisicoes ) )
		{
			modCplex->solveMaster();
			modCplex->updateDualCostsRD(G, vetorReqsCD, numReqsCD);

			if ( opcaoSubRD == 'B' )
			{
				ModeloBC_RD bcRD(G, modCplex->getAlfa());
				bcRD.calculaCaminhoElementar(G, tempoTotal);
				int aux = bcRD.rotasNegativas.size();
				if ( aux > 0 )
				{
					solPricingRD = ( bcRD.cplex.getBestObjValue() - modCplex->getAlfa() );
					for ( int i = 0; i < aux; ++i ) modCplex->insert_ColumnPrimal_RowDual_Direct(bcRD.rotasNegativas[i]);
				}
				else pricingRD = 0;
			}
			else if ( opcaoSubRD == 'P' )
			{
				ElementarRD camElemRD(G, modCplex->getAlfa());
				if ( camElemRD.calculaCaminhoElementar(G, tempoTotal) > 0 )
				{
					rotasDiretas = camElemRD.getRotaCustoMinimo(G, 0.9);
					if ( rotasDiretas != NULL )
					{
						solPricingRD = 9999999;
						for ( int i = 0; rotasDiretas[i] != NULL; ++i )
						{
							modCplex->insert_ColumnPrimal_RowDual_Direct(rotasDiretas[i]);
							if ( rotasDiretas[i]->getCustoReduzido() < solPricingRD ) solPricingRD = rotasDiretas[i]->getCustoReduzido();
						}
						solPricingRD -= modCplex->getAlfa();
						delete [] rotasDiretas;
					}
					else pricingRD = 0;
				}
				else pricingRD = 0;
			}
		}

		if ( !pricingHeuristica )
		{
			if ( ( numReqsPD < numRequisicoes ) && ( numReqsCD < numRequisicoes ) ) bdLasdon = ( solPricingCD < solPricingRD ) ? solPricingCD : solPricingRD;
			else if ( numReqsPD < numRequisicoes ) bdLasdon = solPricingCD;
			else bdLasdon = solPricingRD;
			if ( bdLasdon > menorBdLasdon ) menorBdLasdon = bdLasdon;
		}

		if ( ( pricingRD == 0 ) && ( pricingCD == 1 ) ) pricingRD = -1;
		if ( ( pricingCD == 0 ) && ( pricingRD == 1 ) ) pricingCD = -1;

		if ( ( pricingCD == 0 ) && ( pricingRD == -1 ) ) pricingRD = 1;
		if ( ( pricingRD == 0 ) && ( pricingCD == -1 ) ) pricingCD = 1;

		if ( (time(0) - tempoTotal ) > 14400 ) { printf("Bound de Lasdon!\n"); boundLasdon = true; break; }

	}while( !( !pricingHeuristica && ( pricingCD == 0 ) && ( pricingRD == 0 ) ) );

	float lD = modCplex->solveMaster();
	if ( boundLasdon ) lD += numVeic*menorBdLasdon;

	ModeloHeuristica* modHeuristica = new ModeloHeuristica(*modCplex);
	float lP = modHeuristica->optimize(*modCplex);
	ModeloCplex::setLimitePrimal(lP);
	delete modHeuristica;

	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", lD, lP, ((lP - lD) / lP) * 100, time(0)-tempoTotal);
	fflush(stdout);

	//A partir deste ponto, a raiz do master foi alcançada e deve ser realizado o branch-and-price
	//A raiz do master representara o primeiro no da arvore, que sera ramificada com a insercao das
	//restricoes de branching. Sera feito um branching em arcos, estendendo a formulacao por rotas
	//Os custos de troca serao uma consequencia das rotas obtidas pelo branching em arcos
	if ( !boundLasdon )
	{
		ptrNo noAtual;
		vector < ptrNo > novosNos;
		ListaNosAtivos arvBranching;
		NoArvore *raiz = new NoArvore(modCplex, G, novosNos, lD);
		for (int i = 0; i < novosNos.size(); ++i) arvBranching.insereNo(novosNos[i]);
		delete raiz; //o no raiz serve apenas para inicializar os atributos estaticos e criar o(s) primeiro(s) no(s) -- caso nao seja inteiro --

		int it = 1;
		while ( !arvBranching.vazia() )
		{
			novosNos.clear();
			noAtual = arvBranching.retornaProximo();

			lD = noAtual->getLimiteDual();
			lP = ModeloCplex::getLimitePrimal();
			printf(" %d\t\t  %d\t\t %0.3f\t\t %0.3f\t\t%0.2f\t\t%ld\n", ++it, NoArvore::getTotalNosAtivos(), lD, lP, ((lP - lD) / lP) * 100, time(0)-tempoTotal);
			fflush(stdout);

			lP = noAtual->executaBranching(novosNos, vetorReqsCD, vetorReqsPD, numReqsCD, numReqsPD, opcaoSub, opcaoSubRD, tempoTotal);
			if (lP < ModeloCplex::getLimitePrimal())
			{
				arvBranching.podaPorLimiteDual(lP);
				ModeloCplex::setLimitePrimal(lP);
				noAtual->setMelhoresRotas(modCplex);
			}

			for (int i = 0; i < novosNos.size(); ++i) arvBranching.insereNo(novosNos[i]);
			delete noAtual;

			if ((time(0) - tempoTotal) > 14400) break;
		}
	}

	modCplex->imprimeMelhoresRotas(G);
	printf("sol = %0.2f\n", ModeloCplex::getLimitePrimal());

	float auxTempo;
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
	tempo = (tm->tm_hour - hora) * 3600;
	tempo += (tm->tm_min - min) * 60;
	tempo += tm->tm_sec - seg;
	tempo += tempo * 1000000;
	tempo += tv.tv_usec - micro;
	auxTempo = tempo;
	auxTempo /= 1000000;
	printf ("tempo = %0.2f\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", auxTempo);

	delete [] vetorReqsCD;
	delete [] vetorReqsPD;
	delete modCplex;
	delete G;
	return 0;
}
