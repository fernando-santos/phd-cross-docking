#include "Grafo.h"
using namespace std;

void Grafo::setCustoArestaDual(int i, int j, float custo){
	vert[i]->custoArestasDual[j] = custo;
}

void Grafo::setCustoVerticeDual(int v, float custo){
	vert[v]->custoDual = custo;
}

void Grafo::setCapacVeiculo(int Q){
	capacVeiculo = Q;
}

void Grafo::setCargaVertice(int i, int q){
	vert[i]->carga = q;
}

float Grafo::getCustoVerticeDual(int v){
	return vert[v]->custoDual;
}

bool Grafo::existeAresta(int i, int j){
	return vert[i]->existeArestas[j];
}

float Grafo::getCustoAresta(int i, int j){
	return vert[i]->custoArestas[j];
}

float Grafo::getCustoArestaDual(int i, int j){
	return vert[i]->custoArestasDual[j];
}


int Grafo::getCargaVertice(int i){
	return vert[i]->carga;
}


int Grafo::getCapacVeiculo(){
	return capacVeiculo;
}

int Grafo::getNumReqs(){
	return numReqs;
}

int Grafo::getNumVertices(){
	return numVertices;
}

Grafo::~Grafo(){
	for (int i = 0; i < numVertices; ++i){
		delete vert[i];
	}
	delete [] vert;
}

Grafo::Grafo(char* instanciaSolomon, int nReqs) : numReqs(nReqs), numVertices(2*nReqs + 2){
	int tmp;
	//aloca o vetor de ponteiros
	vert = new Vertice*[numVertices];
	for (int i = 0; i < numVertices; ++i){
		vert[i] = new Vertice(numVertices);
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
	float matriz[numVertices][2];
	for (int i = 0; i < numVertices-1; ++i){
		inFile >> tmp >> matriz[i][0] >> matriz[i][1] >> vert[i]->carga >> vert[i]->tInicio >> vert[i]->tFim >> vert[i]->tServico;
	}
	matriz[numVertices-1][0] = matriz[0][0];
	matriz[numVertices-1][1] = matriz[0][1];
	inFile.close();

	//neste ponto, matriz[i][0] armazena a coordenada X e matriz[i][1] a coordenada Y do vertice i
	//estes valores serao utilizados para calcular o custo das arestas entre os vertices da rede
	//apenas aqueles vertices que existem no grafo serao armazenados (se um vertice nao for incluido, ele nao existe)

	//origem: DEPOSITO(0), destino: FORNECEDORES
	for (int i = 1; i <= numReqs; ++i){
		vert[0]->existeArestas[i] = true;
		vert[0]->custoArestas[i] = sqrt (pow ((matriz[0][0] - matriz[i][0]), 2) + pow ((matriz[0][1] - matriz[i][1]), 2));
	}

	//origem: FORNECEDORES, destino: FORNECEDORES
	for (int i = 1; i <= numReqs; ++i)
	{
		for (int j = (i+1); j <= numReqs; ++j)
		{
			if ( ( rand() % 2 ) == 0 )
			{
				vert[i]->existeArestas[j] = true;
				vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));

				vert[j]->existeArestas[i] = true;
				vert[j]->custoArestas[i] = vert[i]->custoArestas[j];
			}
		}
	}

	//origem: FORNECEDORES, destino: CONSUMIDORES
	for (int i = 1; i <= nReqs; ++i)
	{
		for (int j = (nReqs+1); j < (numVertices-1); ++j)
		{
			vert[i]->existeArestas[j] = true;
			vert[i]->custoArestas[j] = vert[0]->custoArestas[i] + sqrt (pow ((matriz[0][0] - matriz[j][0]), 2) + pow ((matriz[0][1] - matriz[j][1]), 2));
		}
	}

	//origem: CONSUMIDORES, destino: CONSUMIDORES
	for (int i = (nReqs+1); i < (numVertices-1); ++i)
	{
		for (int j = (i+1); j < (numVertices-1); ++j)
		{
			if ( ( rand() % 2 ) == 0 )
			{
				vert[i]->existeArestas[j] = true;
				vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));

				vert[j]->existeArestas[i] = true;
				vert[j]->custoArestas[i] = vert[i]->custoArestas[j];
			}
		}
	}

	//origem: CONSUMIDORES, destino: DEPOSITO ARTIFICIAL
	for (int i = (nReqs+1); i < (numVertices-1); ++i)
	{
		vert[i]->existeArestas[numVertices-1] = true;
		vert[i]->custoArestas[numVertices-1] = sqrt (pow ((matriz[i][0] - matriz[numVertices-1][0]), 2) + pow ((matriz[i][1] - matriz[numVertices-1][1]), 2));
	}
}

void Grafo::setCustoArestasDual(){
	for ( int i = 0; i < numVertices; ++i )
	{
		for ( int j = 0; j < numVertices; ++j )
		{
			if ( vert[i]->existeArestas[j] )
			{
				vert[i]->custoArestasDual[j] = vert[i]->custoArestas[j] + vert[j]->custoDual;
			}
		}
	}
}

void Grafo::imprimeGrafo(){
	for(int i = 0; i < numVertices; ++i)
	{
		printf("%02d} (%02d, %02.02f) [", i, vert[i]->carga, vert[i]->custoDual);
		for (int j = 0; j < numVertices; ++j)
		{
			if (vert[i]->existeArestas[j])
			{
				printf("%02.01f, ", vert[i]->custoArestas[j]);
			}
			else
			{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade = %d\n", capacVeiculo);
}

void Grafo::imprimeGrafoDual(){
	for( int i = 0; i < numVertices; ++i )
	{
		printf("%02d} (%02d) [", i, vert[i]->carga);

		for ( int j = 0; j < numVertices; ++j )
		{
			if ( vert[i]->existeArestas[j] )
			{
				printf("%02.02f, ", vert[i]->custoArestasDual[j]);
			}
			else
			{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade = %d\n", capacVeiculo);
}

