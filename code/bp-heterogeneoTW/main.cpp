#include "ModeloCplex.h"
#include "ModeloHeuristica.h"
#include "ListaNosAtivos.h"
#include "NoArvoreArcos.h"
#include "NoArvore.h"
using namespace std;

int main(int argc, char** argv){
	if ((argc != 6) && (argc != 7)){
		cout << "Parametros esperados:\n"
				<< "  (1) Arquivo de instancias\n"
				<< "  (2) Quantidade de Fornecedores (e consequentemente Consumidores) a considerar\n"
				<< "  (3) Numero de veiculos a serem utilizados no roteamento\n"
				<< "  (4) Custo para carregar OU descarregar cada mercadoria no Cross-Docking\n"
				<< "  (5) Tempo em que o veiculo ficara no Cross-Docking para a consolidacao\n" 
				<< "  (6) Quantidade de pontos interiores para a estabilizacao {OPCIONAL}\n\n";
		exit(0);
	}

	srand(time(0));
	int numPontosExtremosDual;
	int numRotasForn, numRotasCons;
	bool alcancouRaiz, branchingRestricoes = true;
	int numCons, numForn;
	int numVeic = atoi(argv[3]);
	numForn = numCons = atoi(argv[2]);
	int custoTroca = atoi(argv[4]);
	int tempoCrossDocking = atoi(argv[5]);
	if (argc == 6){
		numPontosExtremosDual = 0;
	}else{
		numPontosExtremosDual = atoi(argv[6]);
	}

	//Cria um grafo que contem os custos de todas as arestas entre fornecedores ou consumidores
	//(como as instancias de solomon tem grafo completo, nao eh necessario discriminar arestas)
	//alem dos custos, armazena-se tambem as demandas de fornecedores ou consumidores
	Grafo* G = new Grafo(argv[1], numForn);
	int inicioJanelaDeposito = G->getInicioJanela(0);
	int fimJanelaDeposito = G->getFimJanela(0);
	int fimJanelaForn = G->getMaiorFimJanelaForn();
	int inicioJanelaCons = G->getMenorInicioJanelaCons();
	int diferenca = inicioJanelaCons - fimJanelaForn;
	if (diferenca >= 0){
		fimJanelaForn += ((diferenca - tempoCrossDocking) / 2);
		inicioJanelaCons -= ((diferenca - tempoCrossDocking) / 2);
	}else{
		fimJanelaForn -= ((tempoCrossDocking/2) - (diferenca/2));
		inicioJanelaCons += ((tempoCrossDocking/2) - (diferenca/2));
	}
	cout << "Janelas [" << inicioJanelaDeposito << "," << fimJanelaForn << "] : [" << inicioJanelaCons << "," << fimJanelaDeposito << "]\n";
	
	//Verifica se o problema continua viavel mesmo com a insercao de janelas de tempo 
	//dos fornecedores ao cross-docking e do cross-docking aos consumidores
	for (int i = 1; i <= numForn; ++i){
		//FORNECEDORES
		if ((inicioJanelaDeposito + G->getCustoAresta(0, i)) > G->getFimJanela(i)){
			cout << "Não é possível do Cross-Docking alcançar o vertice (" << i << ") dentro do limite de sua janela de tempo!" << endl;
			exit(0);
		}
		if ((G->getInicioJanela(i) + G->getCustoAresta(i, 2*numForn+1)) > fimJanelaForn){
			cout << "Partindo a qualquer instante dentro da janela do vertice (" << i << ") não é possível alcançar o Cross-Docking dentro do tempo limite!" << endl;
			exit(0);
		}
		//CONSUMIDORES
		if ((inicioJanelaCons + G->getCustoAresta(0, i+numForn)) > G->getFimJanela(i+numForn)){
			cout << "Não é possível do Cross-Docking alcançar o vertice (" << i+numForn << ") dentro do limite de sua janela de tempo!" << endl;
			exit(0);
		}
		if ((G->getInicioJanela(i+numForn) + G->getCustoAresta(i+numForn, 2*numForn+1)) > fimJanelaDeposito){
			cout << "Partindo a qualquer instante dentro da janela do vertice (" << i+numForn << ") não é possível alcançar o Cross-Docking dentro do tempo limite!" << endl;
			exit(0);
		}
	}

	//invoca-se o gerador de rotas, para dar uma solucao viavel para o problema e um pool de rotas
	Rota** rotasForn = geraPoolRotas(G, true, inicioJanelaDeposito, fimJanelaForn, numVeic, numRotasForn);
	Rota** rotasCons = geraPoolRotas(G, false, inicioJanelaCons, fimJanelaDeposito, numVeic, numRotasCons);

	//OBS.: *O numero de rotas encontradas para fornecedores e consumidores certamente sera diferente
	//       mas deve-se inserir uma quantidade de rotas para consumidores e outra para fornecedores
	//		*Aquelas rotas da solucao "otima" nao podem ser descartadas, pois asseguram a viabilidade
	//		*O parametro numForn e numCons tem o mesmo valor (a quantidade de commodities do problema)
	ModeloCplex modCplex(G, rotasForn, rotasCons, numRotasForn, numRotasCons, numVeic, numForn, custoTroca);
	//Libera memoria das rotas alocadas para fornecedores e consumidores
	delete [] rotasForn;
	delete [] rotasCons;

	//Declara um ponteiro para as colunas que serao retornadas do pricing 
	Rota** colunas;
	int k, limVeic, iteracoes = 0, iVeicForn = 0, iVeicCons = 0, tempo = time(0);
	do{
		alcancouRaiz = true;

		//roda o cplex, soluciona a relaxacao do master restrito e cria o modelo dual para a estabilizacao
		modCplex.solveMaster();
		if (numPontosExtremosDual > 0){
			modCplex.geraModeloDual(NULL);
		}
		modCplex.geraPontoInterior(numPontosExtremosDual);
		++iteracoes;

		//atualiza o grafo com os custos duais dos vertices, calcula as rotas e as insere no master restrito
		limVeic = iVeicForn + numVeic;
		for (int k = iVeicForn; k < limVeic; ++k, ++iVeicForn){
			modCplex.updateDualCostsForn(G, (k % numVeic));
			ElementarTW camElemForn(G, true, modCplex.getAlfaDual(k % numVeic));

			if (camElemForn.calculaCaminhoElementar(G, inicioJanelaDeposito, fimJanelaForn) > 0){
				colunas = camElemForn.getRotaCustoMinimo(G, 0.7);
				for (int i = 0; colunas[i] != NULL; ++i){
					modCplex.insert_ColumnPrimal_RowDual_Forn(colunas[i], (k % numVeic));
				}
				alcancouRaiz = false;
				delete [] colunas;
				++iVeicForn;
				break;
			}
		}

		limVeic = iVeicCons + numVeic;
		for (int k = iVeicCons; k < limVeic; ++k, ++iVeicCons){
			modCplex.updateDualCostsCons(G, (k % numVeic));
			ElementarTW camElemCons(G, false, modCplex.getBetaDual(k % numVeic));

			if (camElemCons.calculaCaminhoElementar(G, inicioJanelaCons, fimJanelaDeposito) > 0){
				colunas = camElemCons.getRotaCustoMinimo(G, 0.7);
				for (int i = 0; colunas[i] != NULL; ++i){
					modCplex.insert_ColumnPrimal_RowDual_Cons(colunas[i], (k % numVeic));
				}
				alcancouRaiz = false;
				delete [] colunas;
				++iVeicCons;
				break;
			}
		}
	}while(!alcancouRaiz);

	float lDual = modCplex.solveMaster();	
	ModeloHeuristica* modHeuristica = new ModeloHeuristica(modCplex);
	float lPrimal = modHeuristica->optimize();
	ModeloCplex::setLimitePrimal(lPrimal);
	delete modHeuristica;

	printf("NoCount\t\tNosRestantes\tLowerBound\t\tUpperBound\t\tGAP\t\tTempo\n");
	printf(" 1\t\t  0\t\t %0.3f\t\t %0.3f\t\t%0.2f%\t\t%ld\n", lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);

	//A partir deste ponto, a raiz do master foi alcançada e deve ser realizado o branch-and-price
	//A raiz do master representara o primeiro no da arvore, que sera ramificada com a insercao das
	//restricoes de branching. Cada no da arvore dara origem a 7 outros nos, cujas restricoes serao
	//escritas em funcao do da manipulacao das mercadorias entre fornecedores/consumidores,
	//alem das trocas no CD
	ListaNosAtivos arvoreBranching;
	ptrNo aux, noAtual;
	int noCount = 1;

	if (branchingRestricoes){
		bool segundoBranching, bestPrimal;
		noAtual = new NoArvore(modCplex, tempoCrossDocking, numPontosExtremosDual);
		while (noAtual != NULL){
			if (((noCount % 1) == 0) && (noCount > 1)){
				lDual = noAtual->getLimiteDual();
				lPrimal = ModeloCplex::getLimitePrimal();
				printf(" %d\t\t  %d\t\t %0.3f\t\t %0.3f\t\t%0.2f%\t\t%ld\n", noCount, NoArvore::getTotalNosArmazenados(), lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);
			}
			
			//gera-se 4 filhos (2 considerando (tau^k_q = 0) e 2 para (tau^k_q = 1) sendo que 
			//a mercadoria 'q' devera estar em alguma rota com lambda ou gamma fracionario e 'q' nao
			//tenha sido restringido neste no da arvore.
			for (int i = 1; i <= 4; ++i){
				//o metodo geraFilho() iniciara um novo no (que tera as restricoes de branching pertinentes ao filho)
				//e solucionara sua relaxacao ate a raiz, sendo que para isto novas colunas podem ser geradas, usando
				//os custos duais da relaxacao do modelo. Caso o no possa ser podado, ele nao eh inserido na arvore

				//segundoBranching e bestPrimal sao passados por referencia, pois a a lista de nos da arvore
				//nao pode ser acessada dentro da classe NoArvore, devido a referencia cruzada
				aux = noAtual->geraFilho(modCplex, G, i, 'E', segundoBranching, bestPrimal);
				if (aux != NULL){
					if (!segundoBranching){
						//Caso o no nao possa ser podado, mas ainda seja possivel realizar o branching de restricoes,
						//o insere na arvore para que seja fixada um outro par [k,q] nas proximas iteracoes
						arvoreBranching.insereNo(aux);
						
					}else{
						//Caso nao seja mais possivel realizar o branching de restricoes no No, mas ele ainda nao possa ser podado
						//por limite dual ou otimalidade, executa-se o branching de arcos neste no, ate se obter solucao inteira
						ModeloCplex* modelo = new ModeloCplex(modCplex, aux->getMatrizA_no(), aux->getMatrizB_no(), 
													aux->getPtrRForn_no(), aux->getPtrRCons_no(),  aux->getVariaveisComRestricao());
						bestPrimal = NoArvoreArcos::executaBranchingArcos(modelo, G, aux->getVariaveisComRestricao(), numVeic, numForn, 'E');
						delete modelo;
						delete aux; //Poda por otimalidade
					}
					if (bestPrimal){
						arvoreBranching.podaPorLimiteDual(ModeloCplex::getLimitePrimal());
					}
				}
				modCplex.limpaVariaveisArvoreB1();
			}
			delete noAtual;
			noAtual = arvoreBranching.retornaProximo();
			++noCount;

			//Se o tempo de execucao exceder 5 horas, mostra a ultima solucao e termina
			if (((time(0) - tempo) > 18000) && (noAtual != NULL)){
				lDual = noAtual->getLimiteDual();
				lPrimal = ModeloCplex::getLimitePrimal();
				printf(" %d\t\t  %d\t\t %0.3f\t\t %0.3f\t\t%0.2f%\t\t%ld*\n", noCount, NoArvore::getTotalNosArmazenados(), lDual, lPrimal, ((lPrimal - lDual) / lPrimal) * 100, time(0)-tempo);
				break;
			}
		}

	}else{

		vector<Rota*>* rotasVazia = new vector<Rota*>[numVeic];
		vector<short int*>* matrizVazia = new vector<short int*>[numVeic];
		char** varBranchingZero = new char*[numVeic];
		for (int k = 0; k < numVeic; ++k){
			varBranchingZero[k] = new char[numForn+1];
			memset(varBranchingZero[k], 0, (numForn+1)*sizeof(char));
		}
	
		ModeloCplex* modelo = new ModeloCplex(modCplex, matrizVazia, matrizVazia, rotasVazia, rotasVazia, varBranchingZero);
		NoArvoreArcos::executaBranchingArcos(modelo, G, varBranchingZero, numVeic, numForn, 'E');
		delete modelo;

	}

	cout << "solucao otima = " << ModeloCplex::getLimitePrimal() << endl; 
	cout << "tempo = " << time(0) - tempo << endl;

	//deleta o grafo (o destrutor se encarrega de deletar os vertices)
	delete G;

	return 0;
}

