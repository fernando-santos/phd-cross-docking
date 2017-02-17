#include "Grafo.h"
using namespace std;
int Grafo::timeLimit;
long int Grafo::timeInicio;

bool Grafo::existeAresta(int i, int j){
	if ( i == 0)
	{
		if ( ( j != 0 ) && ( j <= numReqs ) ) return true;
	}
	else if ( i <= numReqs )
	{
		if ( ( j != 0 ) && ( j != i ) && ( j != ( numVertices - 1 ) ) ) return true;
	}
	else if ( i <= 2*numReqs )
	{
		if ( ( j != i ) && ( j > numReqs ) ) return true;
	}
	return false;
}

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
	for (int i = 0; i < numVertices; ++i) delete vert[i];
	delete [] vert;
}

Grafo::Grafo(char* instancia, int nReqs) : numReqs(nReqs), numVertices(2*nReqs + 2){
	int tmp;
	
	//aloca o vetor de ponteiros
	vert = new Vertice*[numVertices];
	for (int i = 0; i < numVertices; ++i) vert[i] = new Vertice(numVertices); 
	
	//Abre o arquivo de instancia para ser processado e montar o grafo
	fstream inFile;
	inFile.open(instancia, ios::in);
	float matriz[numVertices-1][2];	
	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 4; ++i)
	{
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}	
	inFile >> tmp >> capacVeiculo;

	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 5; ++i)
	{
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}

	//Lê todos os dados da instancia e armazena no vetor para as alteracoes necessarias
	for (int i = 0; i < numVertices-1; ++i)
	{
		inFile >> tmp >> matriz[i][0] >> matriz[i][1] >> vert[i]->carga >> vert[i]->tInicio >> vert[i]->tFim >> vert[i]->tServico;
	}
	inFile.close();

	//neste ponto, matriz[i][0] armazena a coordenada X e matriz[i][1] a coordenada Y do vertice i
	//estes valores serao utilizados para calcular o custo das arestas entre os vertices da rede
	//CALCULA-SE INICIALMENTE O CUSTO DO DEPOSITO (0) PARA OS OUTROS VERTICES
	//E DO ARTIFICIAL (NUMVERTICES-1) PARA OS OUTROS EH MAIS_INFINITO
	vert[0]->custoArestas[0] = MAIS_INFINITO;
	vert[0]->custoArestas[numVertices-1] = MAIS_INFINITO;
	vert[numVertices-1]->custoArestas[numVertices-1] = MAIS_INFINITO;
	vert[numVertices-1]->custoArestas[0] = MAIS_INFINITO;
	for (int j = 1; j <= numReqs; ++j)
	{
		//custo do deposito aos demais vertices e vice-versa
		vert[0]->custoArestas[j] = sqrt (pow ((matriz[0][0] - matriz[j][0]), 2) + pow ((matriz[0][1] - matriz[j][1]), 2));
		vert[numReqs+j]->custoArestas[numVertices-1] = sqrt (pow ((matriz[0][0] - matriz[numReqs+j][0]), 2) + pow ((matriz[0][1] - matriz[numReqs+j][1]), 2));
	}

	//CALCULA-SE OS CUSTOS DAS ARESTAS ENTRE FORNECEDORES ( CONSUMIDORES )
	for (int i = 1; i < numVertices-1; ++i)
	{
		if (i <= numReqs)
		{
			for (int j = 1; j <= numReqs; ++j)
			{
				if (i != j)
				{
					vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
				}
			}
		}
		else
		{
			for (int j = numReqs+1; j <= 2*numReqs; ++j)
			{
				if (i != j)
				{
					vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
				}
			}
		}
	}

	//ARESTAS QUE VAO DOS FORNECEDORES AOS CONSUMIDORES
	for (int i = 1; i <= numReqs; ++i)
	{
		for (int j = numReqs+1; j <= 2*numReqs; ++j)
		{
			vert[i]->custoArestas[j] = vert[0]->custoArestas[i] + vert[j]->custoArestas[numVertices-1];
		}
	}
}


void Grafo::setCustoArestasDual(){
	for ( int i = 0; i < numVertices; ++i )
	{
		for ( int j = 0; j < numVertices; ++j )
		{
			if ( existeAresta(i, j) )
			{
				vert[i]->custoArestasDual[j] = vert[i]->custoArestas[j] + vert[j]->custoDual;
			}
			else
			{
				vert[i]->custoArestasDual[j] = MAIS_INFINITO;
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
			if ( existeAresta(i, j) ) printf("%0.02f, ", vert[i]->custoArestas[j]);
			else printf("- , ");
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
			if ( existeAresta(i, j) )
			{
				printf("%0.02f, ", vert[i]->custoArestasDual[j]);
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
