#include "Grafo.h"
using namespace std;

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

bool Grafo::existeArestaDual(int i, int j){
	if ( i == 0)
	{
		if ( ( j != 0 ) && ( j <= numReqsDual ) ) return true;
	}
	else if ( i <= numReqsDual )
	{
		if ( ( j != 0 ) && ( j != i ) && ( j != ( numVerticesDual - 1 ) ) ) return true;
	}
	else if ( i <= 2*numReqsDual )
	{
		if ( ( j != i ) && ( j > numReqsDual ) ) return true;
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

float Grafo::getCustoAresta(int i, int j, bool RD){
	return ( RD ) ? vert[i]->custoArestasRD[j] : vert[i]->custoArestas[j];
}

float Grafo::getCustoArestaDual(int i, int j){
	return vert[i]->custoArestasDual[j];
}

int* Grafo::getIndiceVerticesDual(){
	return indiceVerticesDual;
}

int Grafo::getNumReqsDual(){
	return numReqsDual;	
}

int Grafo::getNumVerticesDual(){
	return numVerticesDual;
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
	delete [] indiceVerticesDual; 
}

Grafo::Grafo(char* instancia, int nReqs, char type) : numReqs(nReqs), numVertices(2*nReqs + 2){
	int tmp;
	indiceVerticesDual = new int[numVertices];
	
	//aloca o vetor de ponteiros
	vert = new Vertice*[numVertices];
	for (int i = 0; i < numVertices; ++i)
	{
		vert[i] = new Vertice(numVertices); //um vertice aponta para todos os outros (dele para ele mesmo eh infinito)
	}
	
	//Abre o arquivo de instancia para ser processado e montar o grafo
	fstream inFile;
	inFile.open(instancia, ios::in);
	float matriz[numVertices-1][2];	
	if (type == 'm')
	{
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
	}
	else if ( type == 'r' )
	{
		inFile >> tmp >> tmp >> tmp >> capacVeiculo >> tmp;
		for (int i = 0; i < numVertices-1; ++i)
		{
			inFile >> tmp >> matriz[i][0] >> matriz[i][1] >> vert[i]->tServico >> vert[i]->carga >> vert[i]->tInicio >> vert[i]->tFim;
			if ( vert[i]->carga < 0 ) vert[i]->carga *= -1;
		}
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
		vert[0]->custoArestasRD[j] = vert[0]->custoArestas[j];

		vert[numReqs+j]->custoArestas[numVertices-1] = sqrt (pow ((matriz[0][0] - matriz[numReqs+j][0]), 2) + pow ((matriz[0][1] - matriz[numReqs+j][1]), 2));
		vert[numReqs+j]->custoArestasRD[numVertices-1] = vert[numReqs+j]->custoArestas[numVertices-1];
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
					vert[i]->custoArestasRD[j] = vert[i]->custoArestas[j];
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
					vert[i]->custoArestasRD[j] = vert[i]->custoArestas[j];
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
			vert[i]->custoArestasRD[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
		}
	}
}

void Grafo::setCustoArestasDual(int* vetorReqsFixas, int numReqsFixas, bool RD){
	int indiceReqs = 0, j;
	numReqsDual = numReqs-numReqsFixas;
	numVerticesDual = 2*numReqsDual+2;
	for ( int i = 0; i < numVertices; ++i )
	{
		for ( j = 0; j < numReqsFixas; ++j )
		{
			if ( ( i <= numReqs ) && ( vetorReqsFixas[j] == i ) ) break;
			else if ( ( i > numReqs ) && ( ( vetorReqsFixas[j]+numReqs ) == i ) ) break;
		}

		if ( j == numReqsFixas ) indiceVerticesDual[indiceReqs++] = i;
	}

	for ( int i = 0; i < numVerticesDual; ++i )
	{
		for ( int j = 0; j < numVerticesDual; ++j )
		{
			if ( existeArestaDual(i, j) )
			{
				if ( RD ) vert[i]->custoArestasDual[j] = vert[indiceVerticesDual[i]]->custoArestasRD[indiceVerticesDual[j]] + vert[indiceVerticesDual[j]]->custoDual;
				else vert[i]->custoArestasDual[j] = vert[indiceVerticesDual[i]]->custoArestas[indiceVerticesDual[j]] + vert[indiceVerticesDual[j]]->custoDual;
			}
			else
			{
				vert[i]->custoArestasDual[j] = MAIS_INFINITO;
			}
		}
	}
}

void Grafo::imprimeGrafo(bool RD){
	for(int i = 0; i < numVertices; ++i)
	{
		printf("%02d} (%02d, %02.02f) [", i, vert[i]->carga, vert[i]->custoDual);
		for (int j = 0; j < numVertices; ++j)
		{
			if ( existeAresta(i, j) )
			{
				if ( RD ) printf("%0.02f, ", vert[i]->custoArestasRD[j]);
				else printf("%0.02f, ", vert[i]->custoArestas[j]);
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
	for( int i = 0; i < numVerticesDual; ++i )
	{
		printf("%02d} (%02d) [", i, vert[indiceVerticesDual[i]]->carga);

		for ( int j = 0; j < numVerticesDual; ++j )
		{
			if ( existeArestaDual(i, j) )
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
