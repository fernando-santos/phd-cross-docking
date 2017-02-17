#include "Grafo.h"
using namespace std;

bool Grafo::existeArestaRD(int i, int j){
	if ( i == 0)
	{
		if ( ( j != 0 ) && ( j <= numCmdt ) ) return true;
	}
	else if ( i <= numCmdt )
	{
		if ( ( j != 0 ) && ( j != i ) && ( j != ( numVertices - 1 ) ) ) return true;
	}
	else if ( i <= 2*numCmdt )
	{
		if ( ( j != i ) && ( j > numCmdt ) ) return true;
	}

	return false;
}

void Grafo::setCustoArestaDual(int i, int j, float custoD){
	vert[i]->custoArestasDual[j] = custoD;
}


void Grafo::setCustoVerticeDual(int v, float custoD){
	vert[v]->custoDual = custoD;
}


float Grafo::getCustoVerticeDual(int v){
	return vert[v]->custoDual;
}


void Grafo::setCapacVeiculo(int c, int cl){
	capacVeiculo[cl] = c;
}


void Grafo::setPesoCommodity(int i, int p){
	vert[i]->pesoCommodity = p;
}


float Grafo::getCustoAresta(int i, int j, int cl){
	return vert[i]->custoArestas[cl][j];
}


float Grafo::getCustoArestaDual(int i, int j){
	return vert[i]->custoArestasDual[j];
}


int Grafo::getPesoCommodity(int i){
	return vert[i]->pesoCommodity;
}


int Grafo::getCapacVeiculo(int cl){
  return capacVeiculo[cl];
}


int Grafo::getNumCmdt(){
	return numCmdt;
}

int Grafo::getNumVertices(){
	return numVertices;
}

Grafo::~Grafo(){
	for (int i = 0; i < numVertices; ++i) delete vert[i];
	delete [] capacVeiculo;
	delete [] vert;
}


Grafo::Grafo(char* instanciaSolomon, float* classesVeic, int maxVeic, int nCmdt) : numCmdt(nCmdt), numVertices(2*nCmdt + 2){
	int tmp;
	//aloca o vetor para representar capacidades para as diferentes classes
	capacVeiculo = new int[maxVeic];
	
	//aloca o vetor de ponteiros
	vert = new Vertice*[numVertices];
	for (int i = 0; i < numVertices; ++i)
	{
		vert[i] = new Vertice(numVertices, maxVeic); //um vertice aponta para todos os outros (dele para ele mesmo eh infinito)
	}
	
	//Abre o arquivo de instancia para ser processado e montar o grafo
	fstream inFile;
	inFile.open(instanciaSolomon, ios::in);

	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 4; ++i)
	{
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}	
	inFile >> tmp >> tmp; //tmp armazena a capacidade do veiculo
	for ( int k = 0; k < maxVeic; ++k ) capacVeiculo[k] = ceil(classesVeic[k] * tmp);

	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 5; ++i)
	{
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}

	//Lê todos os dados da instancia e armazena no vetor para as alteracoes necessarias
	float matriz[numVertices-1][2];
	for (int i = 0; i < numVertices-1; ++i)
	{
		inFile >> tmp >> matriz[i][0] >> matriz[i][1] >> vert[i]->pesoCommodity >> vert[i]->tInicio >> vert[i]->tFim >> vert[i]->tServico;
	}
	inFile.close();

	//neste ponto, matriz[i][0] armazena a coordenada X e matriz[i][1] a coordenada Y do vertice i
	//estes valores serao utilizados para calcular o custo das arestas entre os vertices da rede

	//CALCULA-SE INICIALMENTE O CUSTO DO DEPOSITO (0) PARA OS OUTROS VERTICES
	//E DO ARTIFICIAL (NUMVERTICES-1) PARA OS OUTROS EH MAIS_INFINITO
	for (int k = 0; k < maxVeic; ++k)
	{
		vert[0]->custoArestas[k][0] = MAIS_INFINITO;
		vert[0]->custoArestas[k][numVertices-1] = MAIS_INFINITO;
		vert[numVertices-1]->custoArestas[k][numVertices-1] = MAIS_INFINITO;
		vert[numVertices-1]->custoArestas[k][0] = MAIS_INFINITO;
		for (int j = 1; j < numVertices-1; ++j)
		{
			//custo do deposito aos demais vertices e vice-versa
			vert[0]->custoArestas[k][j] = classesVeic[k] * sqrt (pow ((matriz[0][0] - matriz[j][0]), 2) + pow ((matriz[0][1] - matriz[j][1]), 2));
			vert[j]->custoArestas[k][numVertices-1] = vert[0]->custoArestas[k][j];

			//o custo para chegar ao deposito, ou sair do deposito artificial eh MAIS_INFINITO
			vert[j]->custoArestas[k][0] = MAIS_INFINITO;
			vert[numVertices-1]->custoArestas[k][j] = MAIS_INFINITO;
		}

		//CALCULA-SE OS CUSTOS DAS ARESTAS ENTRE FORNECEDORES ( CONSUMIDORES )
		//CALCULA-SE TAMBEM O CUSTO DAS ARESTAS QUE PARTEM DOS FORNECEDORES PARA OS CONSUMIDORES
		for (int i = 1; i < numVertices-1; ++i)
		{
			if (i <= nCmdt)
			{
				for (int j = 1; j < numVertices-1; ++j)
				{
					if (i != j)
					{
						vert[i]->custoArestas[k][j] = classesVeic[k] * sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
					}
					else
					{
						vert[i]->custoArestas[k][j] = MAIS_INFINITO;
					}
				}
			}
			else
			{
				for (int j = 1; j < numVertices-1; ++j)
				{
					if ((j > nCmdt) && (i != j))
					{
						vert[i]->custoArestas[k][j] = classesVeic[k] * sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
					}
					else
					{
						vert[i]->custoArestas[k][j] = MAIS_INFINITO;
					}
				}
			}
		}
	}
}


void Grafo::setCustoArestasDual(int cl){

	for (int j = 1; j < numVertices-1; ++j) //CUSTO DE 0 PARA TODOS OS VERTICES
	{
		vert[0]->custoArestasDual[j] = vert[0]->custoArestas[cl][j] + vert[j]->custoDual;
	}

	for (int j = 1; j < numVertices-1; ++j) //CUSTO DE TODOS OS VERTICES PARA 0'
	{
		vert[j]->custoArestasDual[numVertices-1] = vert[j]->custoArestas[cl][numVertices-1] + vert[numVertices-1]->custoDual;
	}

	for (int i = 1; i <= numCmdt; ++i) //ARESTAS PARTINDO DO FORNECEDORES
	{
		for (int j = 1; j <= numVertices-1; ++j)
		{
			if (i != j)
			{
				vert[i]->custoArestasDual[j] = vert[i]->custoArestas[cl][j] + vert[j]->custoDual;
			}
			else
			{
				vert[i]->custoArestasDual[j] = MAIS_INFINITO;
			}
		}
	}

	for (int i = numCmdt+1; i < numVertices-1; ++i) //ARESTAS PARTINDO DOS CONSUMIDORES
	{
		for (int j = 1; j < numVertices-1; ++j)
		{
			if ((j > numCmdt) && (i != j))
			{
				vert[i]->custoArestasDual[j] = vert[i]->custoArestas[cl][j] + vert[j]->custoDual;
			}
			else
			{
				vert[i]->custoArestasDual[j] = MAIS_INFINITO;
			}
		}
	}
}


void Grafo::imprimeGrafo(int cl){
	for(int i = 0; i < numVertices; ++i){
		printf("%02d} (%02d, %02.02f) [", i, vert[i]->pesoCommodity, vert[i]->custoDual);
		for (int j = 0; j < numVertices; ++j){
			if (vert[i]->custoArestas[cl][j] < MAIS_INFINITO){
				printf("%02.01f, ", vert[i]->custoArestas[cl][j]);
			}else{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade veiculos = %d\n", capacVeiculo[cl]);
}


void Grafo::imprimeGrafoDual(int cl){
	for(int i = 0; i < numVertices; ++i){
		printf("%02d} (%02d) [", i, vert[i]->pesoCommodity);
		for (int j = 0; j < numVertices; ++j){
			if (vert[i]->custoArestasDual[j] < MAIS_INFINITO){
				printf("%02.01f, ", vert[i]->custoArestasDual[j]);
			}else{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade veiculos = %d\n", capacVeiculo[cl]);
}
