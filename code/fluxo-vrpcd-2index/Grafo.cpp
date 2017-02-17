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

float Grafo::getCustoAresta(int i, int j){
	return vert[i]->custoArestas[j];
}


int Grafo::getPesoCommodity(int i){
	return vert[i]->pesoCommodity;
}


int Grafo::getCapacVeiculo(){
  return capacVeiculo;
}


int Grafo::getNumCmdt(){
	return numCmdt;
}


Grafo::~Grafo(){
	for (int i = 0; i < numVertices; ++i){
		delete vert[i];
	}
	delete [] vert;
}


Grafo::Grafo(char* instanciaSolomon, int nCmdt) : numCmdt(nCmdt), numVertices(2*nCmdt + 2){
	int tmp;
	char buffer[200];

	//aloca o vetor de ponteiros
	vert = new Vertice*[numVertices];
	for (int i = 0; i < numVertices; ++i){
		vert[i] = new Vertice(numVertices); //um vertice aponta para todos os outros (dele para ele mesmo eh infinito)
	}
	
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
	//O DEPOSITO ARTIFICIAL AQUI EH O CROSS-DOCKING
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
}


void Grafo::imprimeGrafo(){
	for(int i = 0; i < numVertices; ++i){
		cout << i << "} (" << vert[i]->pesoCommodity << ") [";
		for (int j = 0; j < numVertices; ++j){
				printf("%02.01f, ", vert[i]->custoArestas[j]);
		}
		printf("]\n");	
	}
	cout << "capacidade veiculos = " << capacVeiculo << endl;
}
