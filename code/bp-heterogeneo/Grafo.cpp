#include "Grafo.h"
using namespace std;
int Grafo::timeLimit;
long int Grafo::timeInicio;

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
	return vert[vetorPosicaoOriginal[i]]->pesoCommodity;
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
	delete [] capacVeiculo;
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
	
	//aloca a matriz para armazenar as subestimativas de custos para os vertices
	subEstimativaVertices = new float**[numCmdt+1];
	for (int i = 1; i <= numCmdt; ++i)
	{
		subEstimativaVertices[i] = new float*[numCmdt+2];
		for (int j = 0; j <= numCmdt+1; ++j)
		{
			subEstimativaVertices[i][j] = new float[numCmdt+2];
		}
	}
	
	vetorPosicaoOriginal = new int[numVertices];
	
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
	//E DO ARTIFICIAL (NUMVERTICES-1) PARA OS OUTROS E MAIS_INFINITO  (APENAS PARA A CLASSE '=' {0})
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
		for (int i = 1; i < numVertices-1; ++i)
		{
			if (i <= nCmdt)
			{
				for (int j = 1; j < numVertices-1; ++j)
				{
					if ((j <= nCmdt) && (i != j))
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


void Grafo::setCustoArestasDual(int cl, char op){
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
				vert[0]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[0]->custoArestas[cl][j] + vert[j]->custoDual;
				
				//preenche o vetor com a posicao original de cada vertice no grafo
				vetorPosicaoOriginal[j-vert[j]->deslocamentoGrafoDual] = j;
			}
		}
		//Atualiza os custos duais das arestas entre os fornecedores ativos
		for (int i = 1; i <= numCmdt; ++i){
			if (vert[i]->deslocamentoGrafoDual >= 0){
				for (int j = 1; j <= numCmdt; ++j){
					if ((vert[j]->deslocamentoGrafoDual >= 0) && (i != j)){
						vert[i-vert[i]->deslocamentoGrafoDual]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[i]->custoArestas[cl][j] + vert[j]->custoDual;
					}
				}
			}
		}
		//Atualiza os custos das arestas duais dos consumidores ao deposito artificial
		for (int j = 1; j <= numCmdt; ++j){
			if (vert[j]->deslocamentoGrafoDual >= 0){
				vert[j-vert[j]->deslocamentoGrafoDual]->custoArestasDual[numVertices-1] = vert[j]->custoArestas[cl][numVertices-1] + vert[numVertices-1]->custoDual;
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
				vert[0]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[0]->custoArestas[cl][j] + vert[j]->custoDual;
				
				//preenche o vetor com a posicao original de cada vertice no grafo
				vetorPosicaoOriginal[j-vert[j]->deslocamentoGrafoDual] = j;
			}
		}
		//Atualiza os custos duais das arestas entre os consumidores ativos
		for (int i = numCmdt+1; i < numVertices-1; ++i){
			if (vert[i]->deslocamentoGrafoDual >= 0){
				for (int j = numCmdt+1; j < numVertices-1; ++j){
					if ((vert[j]->deslocamentoGrafoDual >= 0)  && (i != j)){
						vert[i-vert[i]->deslocamentoGrafoDual]->custoArestasDual[j-vert[j]->deslocamentoGrafoDual] = vert[i]->custoArestas[cl][j] + vert[j]->custoDual;
					}
				}
			}
		}
		//Atualiza os custos das arestas duais dos consumidores ao deposito artificial
		for (int j = numCmdt+1; j < numVertices-1; ++j){
			if (vert[j]->deslocamentoGrafoDual >= 0){
				vert[j-vert[j]->deslocamentoGrafoDual]->custoArestasDual[numVertices-1] = vert[j]->custoArestas[cl][numVertices-1] + vert[numVertices-1]->custoDual;
			}
		}

		//finalmente insere MAIS_INFINITO na diagonal principal da matriz de distancias que acabou de ser criada
		for (int i = numCmdt+1; i <= numCmdt+numConsDuais; ++i){
			vert[i]->custoArestasDual[i] = MAIS_INFINITO;
		}
	}
}


void Grafo::imprimeGrafo(int cl){
	for(int i = 0; i < numVertices; ++i){
		printf("%02d} (%02d, %02.02f) [", i, vert[i]->pesoCommodity, vert[i]->custoDual);
		for (int j = 0; j < numVertices; ++j){
			if (vert[i]->custoArestas[cl][j] != MAIS_INFINITO){
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
			if (vert[i]->custoArestasDual[j] != MAIS_INFINITO){
				printf("%02.01f, ", vert[i]->custoArestasDual[j]);
			}else{
				printf("- , ");
			}
		}
		printf("]\n");	
	}
	printf("capacidade veiculos = %d\n", capacVeiculo[cl]);
}
