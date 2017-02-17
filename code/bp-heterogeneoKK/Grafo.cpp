#include "Grafo.h"
using namespace std;

void Grafo::setCustoArestaDual(int i, int j, float custo){
	vert[i]->custoArestasDual[j] = custo;
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
	return vert[i]->pesoCommodity;
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
	for (int i = 0; i < numVertices; ++i)
	{
		delete vert[i];
	}
	delete [] vert;
}


Grafo::Grafo(char* instanciaSolomon, int nReqs) : numReqs(nReqs), numVertices(2*nReqs + 2){
	int tmp;

	//aloca o vetor de ponteiros
	vert = new Vertice*[numVertices];
	for (int i = 0; i < numVertices; ++i)
	{
		vert[i] = new Vertice(numVertices); //um vertice aponta para todos os outros (dele para ele mesmo eh infinito)
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
	//E DO ARTIFICIAL (NUMVERTICES-1) PARA OS OUTROS E MAIS_INFINITO  (APENAS PARA A CLASSE '=' {0})
	vert[0]->custoArestas[0] = MAIS_INFINITO;
	vert[0]->custoArestas[numVertices-1] = MAIS_INFINITO;
	vert[numVertices-1]->custoArestas[numVertices-1] = MAIS_INFINITO;
	vert[numVertices-1]->custoArestas[0] = MAIS_INFINITO;
	for (int j = 1; j < numVertices-1; ++j)
	{
		//custo do deposito aos demais vertices e vice-versa
		vert[0]->custoArestas[j] = sqrt (pow ((matriz[0][0] - matriz[j][0]), 2) + pow ((matriz[0][1] - matriz[j][1]), 2));
		vert[j]->custoArestas[numVertices-1] = vert[0]->custoArestas[j];

		//o custo para chegar ao deposito, ou sair do deposito artificial eh MAIS_INFINITO
		vert[j]->custoArestas[0] = MAIS_INFINITO;
		vert[numVertices-1]->custoArestas[j] = MAIS_INFINITO;
	}

	//CALCULA-SE OS CUSTOS DAS ARESTAS ENTRE FORNECEDORES ( CONSUMIDORES )
	for (int i = 1; i < numVertices-1; ++i)
	{
		if (i <= numReqs)
		{
			for (int j = 1; j < numVertices-1; ++j)
			{
				if ((j <= numReqs) && (i != j))
				{
					vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
				}else{
					vert[i]->custoArestas[j] = MAIS_INFINITO;
				}
			}
		}
		else
		{
			for (int j = 1; j < numVertices-1; ++j)
			{
				if ((j > numReqs) && (i != j))
				{
					vert[i]->custoArestas[j] = sqrt (pow ((matriz[i][0] - matriz[j][0]), 2) + pow ((matriz[i][1] - matriz[j][1]), 2));
				}
				else
				{
					vert[i]->custoArestas[j] = MAIS_INFINITO;
				}
			}
		}
	}
}


void Grafo::setCustoArestasDual(char op){
	int tmp, aux = 0;
	if (op == 'F')//ATUALIZA AS ARESTAS ENTRE OS VERTICES FORNECEDORES
	{
		//Atualiza os custos das arestas duais do deposito para os fornecedores ativos
		for (int j = 1; j <= numReqs; ++j)
		{
			vert[0]->custoArestasDual[j] = vert[0]->custoArestas[j] + vert[j]->custoDual;
		}
		//Atualiza os custos duais das arestas entre os fornecedores ativos
		for (int i = 1; i <= numReqs; ++i)
		{
			for (int j = 1; j <= numReqs; ++j)
			{
				if (i != j)
				{
					vert[i]->custoArestasDual[j] = vert[i]->custoArestas[j] + vert[j]->custoDual;
				}
			}
		}
		//Atualiza os custos das arestas duais dos consumidores ao deposito artificial
		for (int j = 1; j <= numReqs; ++j)
		{
			vert[j]->custoArestasDual[numVertices-1] = vert[j]->custoArestas[numVertices-1];// + vert[numVertices-1]->custoDual;
		}

		//finalmente insere MAIS_INFINITO na diagonal principal da matriz de distancias que acabou de ser criada
		for (int i = 0; i <= numReqs; ++i)
		{
			vert[i]->custoArestasDual[i] = MAIS_INFINITO;
		}
	}
	else//ATUALIZA AS ARESTAS ENTRE OS VERTICES CONSUMIDORES
	{
		//Atualiza os custos das arestas duais do deposito para os consumidores ativos
		for (int j = numReqs+1; j < numVertices-1; ++j)
		{
			vert[0]->custoArestasDual[j] = vert[0]->custoArestas[j] + vert[j]->custoDual;
		}

		//Atualiza os custos duais das arestas entre os consumidores ativos
		for (int i = numReqs+1; i < numVertices-1; ++i)
		{
			for (int j = numReqs+1; j < numVertices-1; ++j)
			{
				if (i != j)
				{
					vert[i]->custoArestasDual[j] = vert[i]->custoArestas[j] + vert[j]->custoDual;
				}
			}
		}
		//Atualiza os custos das arestas duais dos consumidores ao deposito artificial
		for (int j = numReqs+1; j < numVertices-1; ++j)
		{
			vert[j]->custoArestasDual[numVertices-1] = vert[j]->custoArestas[numVertices-1];// + vert[numVertices-1]->custoDual;
		}

		//finalmente insere MAIS_INFINITO na diagonal principal da matriz de distancias que acabou de ser criada
		for (int i = numReqs+1; i <= 2*numReqs; ++i)
		{
			vert[i]->custoArestasDual[i] = MAIS_INFINITO;
		}
	}
}


void Grafo::imprimeGrafo(){
	for(int i = 0; i < numVertices; ++i)
	{
		printf("%02d} (%02d, %02.02f) [", i, vert[i]->pesoCommodity, vert[i]->custoDual);
		for (int j = 0; j < numVertices; ++j)
		{
			if (vert[i]->custoArestas[j] != MAIS_INFINITO)
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
	printf("capacidade veiculos = %d\n", capacVeiculo);
}


void Grafo::imprimeGrafoDual(){
	for(int i = 0; i < numVertices; ++i)
	{
		printf("%02d} (%02d) [", i, vert[i]->pesoCommodity);
		for (int j = 0; j < numVertices; ++j)
		{
			if (vert[i]->custoArestasDual[j] != MAIS_INFINITO)
			{
				printf("%02.01f, ", vert[i]->custoArestasDual[j]);
			}
			else
			{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade veiculos = %d\n", capacVeiculo);
}
