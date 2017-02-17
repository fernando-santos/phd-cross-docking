#include "geraRotas.h"
using namespace std;

Rota** geraRotasBasicas(Grafo* g, int numVeic){
	bool inseriuVertice;
	int it, k, kAtual, v, cargaTotal;
	int nReqs = g->getNumReqs();
	int depositoArtificial = 2*nReqs+1;
	int capacidade = g->getCapacVeiculo();
	int numVerticesDescobertos;
	int* vertices = new int[nReqs+1];
	vector<int>* rotasForn = new vector<int>[numVeic];
	vector<int>* rotasCons = new vector<int>[numVeic];

	for ( it = 0; it < 100000; ++it )
	{
		//Inicializa os valores do vetor de vertices a serem cobertos para a iteracao (PRIMEIRO A ROTA PARA OS CONSUMIDORES)
		numVerticesDescobertos = nReqs;
		for ( int i = 1; i <= nReqs; ++i ) vertices[i] = i+nReqs;

		//A principio, cria uma rota que vai de 0 a algum CONSUMIDOR (tal que exista aresta) para cada veiculo
		for (k = 0; k < numVeic; ++k){
			int tentativas = 0;
			do {
				++tentativas;
				v = (rand() % numVerticesDescobertos) + 1;
				if ( tentativas > 100000 )
				{
					printf("Nao ha conexao entre deposito e consumidores suficiente para obter rotas viaveis!\n");
					exit(0);
				}
			} while ( !g->existeAresta( vertices[v], depositoArtificial ) ); //vertices[v] -> depositoArtificial <==> 0 -> vertices[v]
			rotasCons[k].push_back(0);
			rotasCons[k].push_back(vertices[v]);

			//Substitue o vertice coberto pelo ultimo do vetor
			vertices[v] = vertices[numVerticesDescobertos];
			--numVerticesDescobertos;
		}

		//Partindo-se das rotas parciais obtidas (0 -> CONSUMIDOR), insere os vertices nas rotas ate cobrir os demais CONSUMIDORES
		k = 0;
		while(numVerticesDescobertos > 0){
			inseriuVertice = false;
			v = (rand() % numVerticesDescobertos) + 1; //escolhe-se o INDICE de um vertice (vertices[v] representa o vertice)

			//Tenta inserir o vertice escolhido em um veiculo, passando para o proximo, caso nao seja possivel inserir no atual
			for (int veic = k; veic < (k+numVeic); ++veic){
				kAtual = ( veic % numVeic );

				//verifica se a cargaTotal excede a capacidade do veiculo
				cargaTotal = g->getCargaVertice( vertices[v] );
				for ( int z = 1; z < rotasCons[kAtual].size(); ++z ) cargaTotal += g->getCargaVertice( rotasCons[kAtual][z] );

				if ( (cargaTotal <= capacidade) && ( g->existeAresta( rotasCons[kAtual][rotasCons[kAtual].size()-1] , vertices[v] ) ) )
				{
					//insere o vertice e atualiza o vetor de verticesDescobertos
					rotasCons[kAtual].push_back(vertices[v]);
					vertices[v] = vertices[numVerticesDescobertos];
					k = (veic % numVeic) + 1;
					--numVerticesDescobertos;
					inseriuVertice = true;
					break;
				}
			}

			if (!inseriuVertice)
			{
				//Essa tentativa nao deu certo, reinicia os vetores e ponteiros e passa para a proxima tentativa
				break;
			}
		}

		if (numVerticesDescobertos == 0){ //Verifica se existem arestas para as rotas retornarem ao deposito para que elas sejam viaveis
			bool rotasInviaveis = false;
			for ( int k = 0; k < numVeic; ++k )
			{
				if ( !g->existeAresta( rotasCons[k][rotasCons[k].size()-1], depositoArtificial ) )
				{
					rotasInviaveis = true;
					break;
				}
			}
		
			if ( rotasInviaveis )
			{
				for ( int k = 0; k < numVeic; ++k ) rotasCons[k].clear();
			}
			else
			{
				break; printf("doneC\n");
			}
		
		}else{ //Limpa as rotas sem sucesso obtidas, para iniciar uma nova iteracao procurando por outras rotas
			for ( int k = 0; k < numVeic; ++k ) rotasCons[k].clear();
		}
	}

	if ( rotasCons[0].size() > 0 )  //SIGNIFICA QUE OBTEVE ROTAS PARA OS CONSUMIDORES COM OS K VEICULOS
	{
		for (it = 0; it < 100000; ++it){
			//Inicializa os valores do vetor de vertices a serem cobertos para a iteracao (PRIMEIRO A ROTA PARA OS FORNECEDORES)
			numVerticesDescobertos = nReqs;
			for (int i = 1; i <= nReqs; ++i) vertices[i] = i;

			//A principio, cria uma rota que vai de 0 a algum FORNECEDOR (tal que exista aresta) para cada veiculo
			for (k = 0; k < numVeic; ++k){
				int tentativas = 0;
				do {
					++tentativas;
					v = (rand() % numVerticesDescobertos) + 1;
					if ( tentativas > 100000 )
					{
						printf("Nao ha conexao entre deposito e fornecedores suficiente para obter rotas viaveis!\n");
						exit(0);
					}
				} while ( !g->existeAresta( 0, vertices[v] ) );
				rotasForn[k].push_back(0);
				rotasForn[k].push_back(vertices[v]);

				//Substitue o vertice coberto pelo ultimo do vetor
				vertices[v] = vertices[numVerticesDescobertos];
				--numVerticesDescobertos;
			}

			//Partindo-se das rotas singulares acima, tenta-se inserir aleatoriamente os vertices nas rotas ate cobrir todos os FORNECEDORES
			k = 0;
			while(numVerticesDescobertos > 0){
				inseriuVertice = false;
				v = (rand() % numVerticesDescobertos) + 1; //escolhe-se o INDICE de um vertice (vertices[v] representa o vertice)

				//Tenta inserir o vertice escolhido em um veiculo, passando para o proximo, caso nao seja possivel inserir no atual
				for (int veic = k; veic < (k+numVeic); ++veic){
					kAtual = ( veic % numVeic );

					//verifica se a cargaTotal excede a capacidade do veiculo
					cargaTotal = g->getCargaVertice( vertices[v] );
					for ( int z = 1; z < rotasForn[kAtual].size(); ++z ) cargaTotal += g->getCargaVertice( rotasForn[kAtual][z] );

					if ( (cargaTotal <= capacidade) && ( g->existeAresta( rotasForn[kAtual][rotasForn[kAtual].size()-1] , vertices[v] ) ) )
					{
						//insere o vertice e atualiza o vetor de verticesDescobertos
						rotasForn[kAtual].push_back(vertices[v]);
						vertices[v] = vertices[numVerticesDescobertos];
						k = (veic % numVeic) + 1;
						--numVerticesDescobertos;
						inseriuVertice = true;
						break;
					}
				}

				if (!inseriuVertice){
					//Essa tentativa nao deu certo, reinicia os vetores e ponteiros e passa para a proxima tentativa
					break;
				}
			}

			if (numVerticesDescobertos == 0){ //Verifica se existem arestas para as rotas retornarem ao deposito para que elas sejam viaveis
				bool rotasInviaveis = false;
				for ( int k = 0; k < numVeic; ++k )
				{
					if ( !g->existeAresta( 0, rotasForn[k][rotasForn[k].size()-1] ) )
					{
						rotasInviaveis = true;
						break;
					}
				}
			
				if ( rotasInviaveis )
				{
					for ( int k = 0; k < numVeic; ++k ) rotasForn[k].clear();
				}
				else
				{
					break; printf("doneF\n");
				}
			
			}else{ //Limpa as rotas sem sucesso obtidas, para iniciar uma nova iteracao procurando por outras rotas
				for ( int k = 0; k < numVeic; ++k ) rotasForn[k].clear();
			}
		}
	}

	if ( rotasForn[0].size() > 0 ) 
	{
		Rota** solucaoViavel = new Rota*[numVeic+1];
		for ( int k = 0; k < numVeic; ++k )
		{
			solucaoViavel[k] = new Rota();
			for (int z = 0; z < rotasForn[k].size(); ++z ) solucaoViavel[k]->inserirVerticeFim(rotasForn[k][z]);
			for (int z = 1; z < rotasCons[k].size(); ++z ) solucaoViavel[k]->inserirVerticeFim(rotasCons[k][z]);
			solucaoViavel[k]->inserirVerticeFim(depositoArtificial);
			solucaoViavel[k]->setCustoRota(g); //APENAS O CUSTO DE ROTEAMENTO (SEM CUSTO DE TROCA)
		}

		delete [] rotasForn;
		delete [] rotasCons;
		delete [] vertices;
		solucaoViavel[numVeic] = NULL;
		return solucaoViavel;
	}
	else
	{
		delete [] rotasForn;
		delete [] rotasCons;
		delete [] vertices;
		cout << "Não encontrou rotas viaveis para k = " << numVeic << endl;
		exit(0);
	}
}

Rota** geraRotasBasicas(Grafo* g, char* solucaoInicial){
	int numVeic, i;
	int depositoArtificial = 2*g->getNumReqs()+1;

	fstream fileSolucaoInicial;
	fileSolucaoInicial.open(solucaoInicial, ios::in);
	fileSolucaoInicial >> numVeic;
	Rota** solucaoViavel = new Rota*[numVeic+1];
	solucaoViavel[numVeic] = NULL;

	for ( int k = 0; k < numVeic; ++k )
	{
		solucaoViavel[k] = new Rota();

		do{
			fileSolucaoInicial >> i;
			solucaoViavel[k]->inserirVerticeFim(i);
		}while ( i != depositoArtificial );

		solucaoViavel[k]->setCustoRota(g);
	}
	return solucaoViavel;
}
