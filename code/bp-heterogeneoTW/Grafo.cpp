#include "Grafo.h"
using namespace std;

int Grafo::getTempoServico(int i){
	return vert[i]->tServico;
}

int Grafo::getInicioJanela(int i){
	return vert[i]->tInicio;
}

int Grafo::getFimJanela(int i){
	return vert[i]->tFim;
}

int Grafo::getMaiorFimJanelaForn(){
	int maior = 0;
	for (int i = 1; i <= numCmdt; ++i){
		if (vert[i]->tFim > maior){
			maior = vert[i]->tFim;
		}
	}
	return maior;
}

int Grafo::getMenorInicioJanelaCons(){
	int menor = MAIS_INFINITO;
	for (int i = numCmdt+1; i < 2*numCmdt; ++i){
		if (vert[i]->tInicio < menor){
			menor = vert[i]->tInicio;
		}
	}
	return menor;
}

void Grafo::setCustoArestaDual(int i, int j, float custo){
	vert[i-vert[i]->deslocamentoGrafoDual]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] += custo;
}

int Grafo::getPesoCommoditySemCorrigirPosicao(int i){
	return vert[i]->pesoCommodity;
}

int* Grafo::getVetorPosicaoOriginal(){
	return vetorPosicaoOriginal;
}

int Grafo::getNumFornDuais(){
	return numFornDuais;
}

int Grafo::getNumConsDuais(){
	return numConsDuais;
}

void Grafo::setCustoVerticeDual(int v, float custoD){
	vert[v]->custoDual = custoD;
}


float Grafo::getCustoVerticeDual(int v){
	return vert[v]->custoDual;
}


void Grafo::setCapacVeiculo(int c){
	capacVeiculo = c;
}


void Grafo::setPesoCommodity(int i, int p){
	vert[i]->pesoCommodity = p;
}


float Grafo::getCustoAresta(int i, int j){
	return vert[i]->custoArestas[j];
}


float Grafo::getCustoArestaDual(int i, int j){
	return vert[i]->custoArestasDual[j];
}


int Grafo::getPesoCommodity(int i){
	return vert[vetorPosicaoOriginal[i]]->pesoCommodity;
}


int Grafo::getCapacVeiculo(){
  return capacVeiculo;
}


int Grafo::getNumCmdt(){
	return numCmdt;
}

int Grafo::getNumVertices(){
	return numVertices;
}

Grafo::~Grafo(){
	for (int i = 0; i < numVertices; ++i){
		delete vert[i];
	}
	delete [] vert;

	for (int i = 1; i <= numCmdt; ++i){
		for (int j = 0; j <= numCmdt+1; ++j){
			delete [] subEstimativaVertices[i][j];
		}
		delete [] subEstimativaVertices[i];
	}
	delete [] subEstimativaVertices;
	delete [] vetorPosicaoOriginal;
}


Grafo::Grafo(char* instanciaSolomon, int nCmdt) : numCmdt(nCmdt), numVertices(2*nCmdt + 2){
	int tmp;
	//aloca o vetor de ponteiros
	vert = new Vertice*[numVertices];
	for (int i = 0; i < numVertices; ++i){
		vert[i] = new Vertice(numVertices); //um vertice aponta para todos os outros (dele para ele mesmo eh infinito)
	}
	
	//aloca a matriz para armazenar as subestimativas de custos para os vertices
	subEstimativaVertices = new float**[numCmdt+1];
	for (int i = 1; i <= numCmdt; ++i){
		subEstimativaVertices[i] = new float*[numCmdt+2];
		for (int j = 0; j <= numCmdt+1; ++j){
			subEstimativaVertices[i][j] = new float[numCmdt+2];
		}
	}
	
	vetorPosicaoOriginal = new int[numVertices];
	
	//Abre o arquivo de instancia para ser processado e montar o grafo
	fstream inFile;
	inFile.open(instanciaSolomon, ios::in);

	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 4; ++i){
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}	
	inFile >> tmp >> capacVeiculo; //Le a capacidade do veiculo
	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 5; ++i){
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}

	//Lê todos os dados da instancia e armazena no vetor para as alteracoes necessarias
	float matriz[numVertices-1][2];
	for (int i = 0; i < numVertices-1; ++i){
		inFile >> tmp >> matriz[i][0] >> matriz[i][1] >> vert[i]->pesoCommodity
				 >> vert[i]->tInicio >> vert[i]->tFim >> vert[i]->tServico;
	}
	inFile.close();

	//neste ponto, matriz[i][0] armazena a coordenada X e matriz[i][1] a coordenada Y do vertice i
	//estes valores serao utilizados para calcular o custo das arestas entre os vertices da rede

	//CALCULA-SE INICIALMENTE O CUSTO DO DEPOSITO (0) PARA OS OUTROS VERTICES
	//E DO ARTIFICIAL (NUMVERTICES-1) PARA OS OUTROS E MAIS_INFINITO  
	vert[0]->custoArestas[0] = MAIS_INFINITO;
	vert[0]->custoArestas[numVertices-1] = MAIS_INFINITO;
	vert[numVertices-1]->custoArestas[numVertices-1] = MAIS_INFINITO;
	vert[numVertices-1]->custoArestas[0] = MAIS_INFINITO;
	for (int j = 1; j < numVertices-1; ++j){
		//custo do deposito aos demais vertices e vice-versa
		vert[0]->custoArestas[j] = sqrt (pow ((matriz[0][0] - matriz[j][0]), 2) + pow ((matriz[0][1] - matriz[j][1]), 2));
		vert[j]->custoArestas[numVertices-1] = vert[0]->custoArestas[j];
		
		//o custo para chegar ao deposito, ou sair do deposito artificial eh MAIS_INFINITO
		vert[j]->custoArestas[0] = MAIS_INFINITO;
		vert[numVertices-1]->custoArestas[j] = MAIS_INFINITO;
	}

	//CALCULA-SE OS CUSTOS ENTRE OS VERTICES FORNECEDORES
	for (int i = 1; i < numVertices-1; ++i){
		if (i <= nCmdt){
			for (int j = 1; j < numVertices-1; ++j){
				if ((j <= nCmdt) && (i != j)){
					vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
				}else{
					vert[i]->custoArestas[j] = MAIS_INFINITO;
				}
			}
		}else{
			for (int j = 1; j < numVertices-1; ++j){
				if ((j > nCmdt) && (i != j)){
					vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
				}else{
					vert[i]->custoArestas[j] = MAIS_INFINITO;
				}
			}
		}
	}

	//FINALMENTE, CALCULA-SE AS SUBESTIMATIVAS DE CUSTOS A SEREM USADAS NA PODA DE LABELS NO CAMINHO ELEMENTAR
	calculaSubEstimativaVertices();
}


void Grafo::setCustoArestasDual(char op){
	int tmp, aux = 0;
	vert[0]->deslocamentoGrafoDual = 0;
	if (op == 'F'){//ATUALIZA AS ARESTAS ENTRE OS VERTICES FORNECEDORES

		for (int i = 1; i <= numCmdt; ++i){
//			if (vert[i]->custoDual < 0){
//				tmp = 0;
//				for (int j = 0; ((j <= numCmdt) && (tmp == 0)); ++j){
//					for (int k = j+1; k <= numCmdt+1; ++k){
//						if ((subEstimativaVertices[i][j][k] + vert[i]->custoDual) < 0){
//							tmp = 1;
//							break;
//						}
//					}
//				}
//				//Caso o vertice nao contribua para melhorar o custo em nenhuma hipotese, este pode ser eliminado
//				if (tmp == 0){
//					vert[i]->deslocamentoGrafoDual = -1;
//					++aux;
//				}else{
					vert[i]->deslocamentoGrafoDual = aux;
//				}
//			}else{
//				vert[i]->deslocamentoGrafoDual = -1;
//				++aux;
//			}
		}
		numFornDuais = numCmdt - aux;

		//Atualiza os custos das arestas duais do deposito para os fornecedores ativos
		for (int j = 1; j <= numCmdt; ++j){
			if (vert[j]->deslocamentoGrafoDual >= 0){
				vert[0]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[0]->custoArestas[j] + vert[j]->custoDual;
				
				//preenche o vetor com a posicao original de cada vertice no grafo
				vetorPosicaoOriginal[j-vert[j]->deslocamentoGrafoDual] = j;
			}
		}
		//Atualiza os custos duais das arestas entre os fornecedores ativos
		for (int i = 1; i <= numCmdt; ++i){
			if (vert[i]->deslocamentoGrafoDual >= 0){
				for (int j = 1; j <= numCmdt; ++j){
					if ((vert[j]->deslocamentoGrafoDual >= 0) && (i != j)){
						vert[i-vert[i]->deslocamentoGrafoDual]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[i]->custoArestas[j] + vert[j]->custoDual;
					}
				}
			}
		}
		//Atualiza os custos das arestas duais dos consumidores ao deposito artificial
		for (int j = 1; j <= numCmdt; ++j){
			if (vert[j]->deslocamentoGrafoDual >= 0){
				vert[j-vert[j]->deslocamentoGrafoDual]->custoArestasDual[numVertices-1] = vert[j]->custoArestas[numVertices-1] + vert[numVertices-1]->custoDual;
			}
		}

		//finalmente insere MAIS_INFINITO na diagonal principal da matriz de distancias que acabou de ser criada
		for (int i = 1; i <= numFornDuais; ++i){
			vert[i]->custoArestasDual[i] = MAIS_INFINITO;
		}

	}else{//ATUALIZA AS ARESTAS ENTRE OS VERTICES CONSUMIDORES

		for (int i = 1; i <= numCmdt; ++i){
//			if (vert[i+numCmdt]->custoDual < 0){
//				tmp = 0;
//				for (int j = 0; ((j <= numCmdt) && (tmp == 0)); ++j){
//					for (int k = j+1; k <= numCmdt+1; ++k){
//						if ((subEstimativaVertices[i][k][j] + vert[i+numCmdt]->custoDual) < 0){
//							tmp = 1;
//							break;
//						}
//					}
//				}
//				//Caso o vertice nao contribua para melhorar o custo em nenhuma hipotese, este pode ser eliminado
//				if (tmp == 0){
//					vert[i+numCmdt]->deslocamentoGrafoDual = -1;
//					++aux;
//				}else{
					vert[i+numCmdt]->deslocamentoGrafoDual = aux;
//				}
//			}else{
//				vert[i+numCmdt]->deslocamentoGrafoDual = -1;
//				++aux;
//			}
		}
		numConsDuais = numCmdt - aux;

		//Atualiza os custos das arestas duais do deposito para os consumidores ativos
		for (int j = numCmdt+1; j < numVertices-1; ++j){
			if (vert[j]->deslocamentoGrafoDual >= 0){
				vert[0]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[0]->custoArestas[j] + vert[j]->custoDual;
				
				//preenche o vetor com a posicao original de cada vertice no grafo
				vetorPosicaoOriginal[j-vert[j]->deslocamentoGrafoDual] = j;
			}
		}
		//Atualiza os custos duais das arestas entre os consumidores ativos
		for (int i = numCmdt+1; i < numVertices-1; ++i){
			if (vert[i]->deslocamentoGrafoDual >= 0){
				for (int j = numCmdt+1; j < numVertices-1; ++j){
					if ((vert[j]->deslocamentoGrafoDual >= 0)  && (i != j)){
						vert[i-vert[i]->deslocamentoGrafoDual]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[i]->custoArestas[j] + vert[j]->custoDual;
					}
				}
			}
		}
		//Atualiza os custos das arestas duais dos consumidores ao deposito artificial
		for (int j = numCmdt+1; j < numVertices-1; ++j){
			if (vert[j]->deslocamentoGrafoDual >= 0){
				vert[j-vert[j]->deslocamentoGrafoDual]->custoArestasDual[numVertices-1] = vert[j]->custoArestas[numVertices-1] + vert[numVertices-1]->custoDual;
			}
		}

		//finalmente insere MAIS_INFINITO na diagonal principal da matriz de distancias que acabou de ser criada
		for (int i = numCmdt+1; i <= numCmdt+numConsDuais; ++i){
			vert[i]->custoArestasDual[i] = MAIS_INFINITO;
		}
	}
}


void Grafo::imprimeGrafo(){
	for(int i = 0; i < numVertices; ++i){
		printf("%02d} (%02d, %02.02f) [", i, vert[i]->pesoCommodity, vert[i]->custoDual);
		for (int j = 0; j < numVertices; ++j){
			if (vert[i]->custoArestas[j] != MAIS_INFINITO){
				printf("%02.01f, ", vert[i]->custoArestas[j]);
			}else{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade veiculos = %d\n", capacVeiculo);
}


void Grafo::imprimeGrafoDual(){
	for(int i = 0; i < numVertices; ++i){
		printf("%02d} (%02d) [", i, vert[i]->pesoCommodity);
		for (int j = 0; j < numVertices; ++j){
			if (vert[i]->custoArestasDual[j] != MAIS_INFINITO){
				printf("%02.01f, ", vert[i]->custoArestasDual[j]);
			}else{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade veiculos = %d\n", capacVeiculo);
}


void Grafo::calculaSubEstimativaVertices(){
	//subEstimativas para os FORNECEDORES
	//A subestimativa para os forncedores ficam na diagonal SUPERIOR da matriz
	for (int i = 1; i <= numCmdt; ++i){	//verifico os adjacentes do vertice i (inclusive o deposito e o deposito artificial)
		for (int a = 0; a <= numCmdt; ++a){ //antecessores
			for (int s = a+1; s <= numCmdt; ++s){ //sucessores
				subEstimativaVertices[i][a][s] = vert[a]->custoArestas[i] + vert[i]->custoArestas[s] - vert[a]->custoArestas[s];
			}
			if (a == 0){
				subEstimativaVertices[i][a][numCmdt+1] = MAIS_INFINITO;
			}else{
				subEstimativaVertices[i][a][numCmdt+1] = vert[a]->custoArestas[i] + vert[i]->custoArestas[numVertices-1] - vert[a]->custoArestas[numVertices-1];
			}
		}
	}

	//subEstimativas para os CONSUMIDORES
	//A subestimativa para os consumidores ficam na diagonal INFERIOR da matriz
	for (int i = 1; i <= numCmdt; ++i){	//verifico os adjacentes do vertice i (inclusive o deposito e o deposito artificial)
		for (int a = 0; a <= numCmdt; ++a){ //antecessores
			if (a == 0){
				for (int s = a+1; s <= numCmdt; ++s){
					subEstimativaVertices[i][s][0] = vert[0]->custoArestas[i+numCmdt] + vert[i+numCmdt]->custoArestas[s+numCmdt] - vert[0]->custoArestas[s+numCmdt];
				}
			}else{
				for (int s = a+1; s <= numCmdt+1; ++s){ //sucessores
					subEstimativaVertices[i][s][a] = vert[a+numCmdt]->custoArestas[i+numCmdt] + vert[i+numCmdt]->custoArestas[s+numCmdt] - vert[a+numCmdt]->custoArestas[s+numCmdt];
				}
			}
		}
	}
}
