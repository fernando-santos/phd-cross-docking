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

	//neste ponto, matriz[i][0] armazena a coordenada X e matriz[i][1] a coordenada Y do vertice i
	//estes valores serao utilizados para calcular o custo das arestas entre os vertices da rede
	//apenas aqueles vertices que existem no grafo serao armazenados (se um vertice nao for incluido, ele nao existe)

	char ch;
	int vertice, adj, countCh, countNum;
	vector<int> depositoForn, depositoCons;

	//primeiro insiro no grafo os adjacentes ao deposito (os consumidores nao podem ser inseridos diretamente)
	countCh = 0, countNum = 0;
	inFile >> vertice;
	do
	{
		++countCh;
		ch = inFile.get();
		if ( ch == ' ' ) ++countNum;
	}
	while ( ch != '\n' );
	inFile.seekg ( -countCh, ios::cur );
	while (--countNum >= 0)
	{
		inFile >> adj;
		if ( adj <= numReqs ) 
		{
			vert[0]->existeArestas[adj] = true;
			vert[0]->custoArestas[adj] = sqrt (pow ((matriz[0][0] - matriz[adj][0]), 2) + pow ((matriz[0][1] - matriz[adj][1]), 2));
			depositoForn.push_back(adj);
		}
		else
		{
			vert[adj]->existeArestas[numVertices-1] = true;
			vert[adj]->custoArestas[numVertices-1] = sqrt (pow ((matriz[adj][0] - matriz[numVertices-1][0]), 2) + 
															pow ((matriz[adj][1] - matriz[numVertices-1][1]), 2));
			depositoCons.push_back(adj);
		}
	}

	//depois insere as arestas dos fornecedores e consumidores (mas nao as arestas dos fornecedores aos consumidores)
	for ( int i = 1; i <= 2*numReqs; ++i )
	{
		countCh = 0, countNum = 0;
		inFile >> vertice;
		do
		{
			++countCh;
			ch = inFile.get();
			if ( ch == ' ' ) ++countNum;
		}
		while ( ch != '\n' );
		inFile.seekg ( -countCh, ios::cur );

		while (--countNum >= 0)
		{
			inFile >> adj;
			if ( adj != 0 )
			{
				vert[vertice]->existeArestas[adj] = true;
				vert[vertice]->custoArestas[adj] = sqrt (pow ((matriz[vertice][0] - matriz[adj][0]), 2) + pow ((matriz[vertice][1] - matriz[adj][1]), 2));
			}
		}
	}

	//insere finalmente apenas as arestas dos fornecedores aos consumidores
	for ( int i = 0; i < depositoForn.size(); ++i )
	{
		for ( int j = 0; j < depositoCons.size(); ++j )
		{
			vert[depositoForn[i]]->existeArestas[depositoCons[j]] = true;
			vert[depositoForn[i]]->custoArestas[depositoCons[j]] = sqrt (pow ((matriz[depositoForn[i]][0] - matriz[0][0]), 2) + 
																		pow ((matriz[depositoForn[i]][1] - matriz[0][1]), 2)) + 
																	sqrt (pow ((matriz[0][0] - matriz[depositoCons[j]][0]), 2) + 
																		pow ((matriz[0][1] - matriz[depositoCons[j]][1]), 2));
		}
	}

	inFile.close();
}

Grafo::Grafo(char* instanciaSolomon, char* solucaoInicial, int nReqs, int grauVertices) : numReqs(nReqs), numVertices(2*nReqs + 2){
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

	//insere os arcos presentes na solucao inicial
	vector<int>* listaAdj = new vector<int>[numVertices-1];
	int numVeic, i, j;
	fstream fileSolucaoInicial;
	fileSolucaoInicial.open(solucaoInicial, ios::in);
	fileSolucaoInicial >> numVeic;
	for ( int k = 0; k < numVeic; ++k )
	{
		fileSolucaoInicial >> i;
		while ( i != ( numVertices-1 ) )
		{
			fileSolucaoInicial >> j;
			
			if ( ( ( i <= numReqs ) && ( j > nReqs ) ) )
			{
				listaAdj[0].push_back(i);
				listaAdj[i].push_back(0);
				listaAdj[0].push_back(j);
				listaAdj[j].push_back(0);
			}
			else
			{				
				if ( j != ( numVertices - 1 ) ) 
				{
					listaAdj[i].push_back(j);
					listaAdj[j].push_back(i);
				}
				else
				{
					listaAdj[i].push_back(0);
					listaAdj[0].push_back(i);
				}								
			}

			i = j;
		}
	}
	
	//insere arcos no grafo para que o grau de cada vertice seja exatamente aquele passado como argumento
	bool adjacente;
	int ajuste, desconto, count;
	for ( int i = 1; i <= 2*numReqs; ++i )
	{
		ajuste = ( i <= numReqs ) ? 0 : numReqs;
		desconto = ( ( i == numReqs ) || ( i == 2*numReqs ) ) ?  ( numReqs % 2 ) : 0;
		while ( listaAdj[i].size() < ( grauVertices - desconto ) )
		{
			//seleciona um adjacente aleatoriamente e verifica se este adjacente ainda nao alcancou o grau desejado
			count = 0;
			do
			{
				j = (rand() % numReqs) + 1;

				//verifica se (j+ajuste) ja eh adjacente de i
				adjacente = false;
				for ( int y = 0; y < listaAdj[i].size(); ++y )
				{
					if ( listaAdj[i][y] == (j+ajuste) )
					{
						adjacente = true;
						break;
					}
				}
				if ( ++count > 100000 ) exit(0);
			}
			while ( ( adjacente ) || ( listaAdj[j+ajuste].size() >= grauVertices ) || ( ( j + ajuste) ==  i ) );

			listaAdj[i].push_back(j+ajuste);
			listaAdj[j+ajuste].push_back(i);
		}
	}

	//preenche o grafo a ser usado durante a execucao do programa (ADJACENTES AO DEPOSITO)
	for ( int i = 0; i < listaAdj[0].size(); ++i )
	{
		if ( listaAdj[0][i] <= numReqs )
		{
			vert[0]->existeArestas[listaAdj[0][i]] = true;
			vert[0]->custoArestas[listaAdj[0][i]] = sqrt (pow ( ( matriz[0][0] - matriz[listaAdj[0][i]][0] ), 2 ) + 
														pow ( ( matriz[0][1] - matriz[listaAdj[0][i]][1] ), 2 ) );
		}
		else
		{
			vert[listaAdj[0][i]]->existeArestas[numVertices-1] = true;
			vert[listaAdj[0][i]]->custoArestas[numVertices-1] = sqrt (pow ( ( matriz[listaAdj[0][i]][0] - matriz[numVertices-1][0] ), 2 ) + 
														pow ( ( matriz[listaAdj[0][i]][1] - matriz[numVertices-1][1] ), 2 ) );
		}
	}

	//preenche o grafo a ser usado durante a execucao do programa (ADJACENTES FORN->FORN ou CONS->CONS)
	for ( int i = 1; i <= 2*numReqs; ++i )
	{
		for ( int j = 0; j < listaAdj[i].size(); ++j )
		{
			if (listaAdj[i][j] != 0)
			{
				vert[i]->existeArestas[listaAdj[i][j]] = true;
				vert[i]->custoArestas[listaAdj[i][j]] = sqrt (pow ( ( matriz[i][0] - matriz[listaAdj[i][j]][0] ), 2 ) + 
														pow ( ( matriz[i][1] - matriz[listaAdj[i][j]][1] ), 2 ) );
			}
		}
	}

	//preenche o grafo a ser usado durante a execucao do programa (ADJACENTES FORN->CONS)
	for ( int i = 1; i <= numReqs; ++i )
	{
		for ( int j = 0; j < listaAdj[i].size(); ++j )
		{
			if ( listaAdj[i][j] == 0 ) //conecta o fornecedor aos consumidores adjacentes a 0
			{
				for ( int k = ( numReqs + 1 ); k <= 2*numReqs; ++k )
				{
					adjacente = false;
					for ( int l = 0; l < listaAdj[k].size(); ++l )
					{
						if ( listaAdj[k][l] == 0 )
						{
							adjacente = true;
							break;
						}
					}
					if ( adjacente )
					{
						vert[i]->existeArestas[k] = true;
						vert[i]->custoArestas[k] = sqrt (pow ((matriz[i][0] - matriz[0][0]), 2) + pow ((matriz[i][1] - matriz[0][1]), 2)) + 
													sqrt (pow ((matriz[0][0] - matriz[k][0]), 2) + pow ((matriz[0][1] - matriz[k][1]), 2));
					}
				}
			}
		}
	}

	inFile.close();
	fileSolucaoInicial.close();
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
				printf("%0.02f, ", vert[i]->custoArestas[j]);
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

void Grafo::imprimeConexoes(){
	//primeiro imprime todas as conexoes do deposito para fornecedores e consumidores
	//(lembrando que tanto fornecedores quanto consumidores podem retornar ao deposito)
	printf("0\t");
	for ( int i = 1; i <= numReqs; ++i )
	{
		if ( vert[0]->existeArestas[i] ) printf("%d ", i);
	}
	for ( int i = numReqs+1; i <= 2*numReqs; ++i )
	{
		if ( vert[i]->existeArestas[numVertices-1] ) printf("%d ", i);
	}

	//agora exibe os fornecedores
	for( int i = 1; i <= numReqs; ++i )
	{
		printf("\n%d\t", i);
		for ( int j = 1; j <= numReqs; ++j )
		{
			if ( vert[i]->existeArestas[j] ) printf("%d ", j);
		}
		if ( vert[0]->existeArestas[i] ) printf("0 ");
	}

	//finalmente exibe os consumidores
	for( int i = numReqs+1; i <= 2*numReqs; ++i )
	{
		printf("\n%d\t", i);
		for ( int j = numReqs+1; j <= 2*numReqs; ++j )
		{
			if ( vert[i]->existeArestas[j] ) printf("%d ", j);
		}
		if ( vert[i]->existeArestas[numVertices-1] ) printf("0 ");
	}
	printf("\n");

	int numArcs = 0;
	for ( int i = 0; i < numVertices; ++i )
	{
		for ( int j = 0; j < numVertices; ++j )
		{
			if ( vert[i]->existeArestas[j] ) ++numArcs;
		}
	}
	printf("numArcs = %d\n", numArcs);
}
