#include "geraRotas.h"
using namespace std;

Rota** geraRotasBasicas(Grafo* g, int* vetorReqsCD, int* vetorReqsPD, int numReqsCD, int numReqsPD, int numVeic){
	bool inseriuVertice;
	int nReqs = g->getNumReqs();
	int depositoArtificial = 2*nReqs+1;
	int capacidade = g->getCapacVeiculo();
	int it, k, kAtual, v, aux, cargaTotal;
	int numReqsCDDescobertos, numReqsPDDescobertos, numReqsCDPDDescobertos;
	int* vetorReqsCDPD = new int[depositoArtificial];
	vector<int>* rotas = new vector<int>[numVeic];

	for ( it = 0; it < 10000; ++it )
	{
		kAtual = 0; cargaTotal = 0;
		numReqsCDDescobertos = numReqsCD;
		for ( int kk = 0; kk < numVeic; ++kk ) rotas[kk].clear(); //limpa as rotas anteriores mal-sucedidas
		while ( numReqsCDDescobertos > 0 ) //cobre todas aquelas requisicoes que devem obrigatoriamente passar no CD
		{
			v = rand() % numReqsCDDescobertos;
			cargaTotal += g->getCargaVertice(vetorReqsCD[v]);
			if ( cargaTotal > capacidade ) 
			{
				cargaTotal = g->getCargaVertice(vetorReqsCD[v]);
				if ( ++kAtual == numVeic ) break;
			}
			aux = vetorReqsCD[v];
			rotas[kAtual].push_back(aux);
			--numReqsCDDescobertos;
			vetorReqsCD[v] = vetorReqsCD[numReqsCDDescobertos];
			vetorReqsCD[numReqsCDDescobertos] = aux;
		}

		if ( kAtual < numVeic ) //Caso haja inviabilidade na atribuicao das requisicoes fixas no CD, inicia outra iteracao
		{
			cargaTotal = 0;
			if ( ( numReqsCD > 0 ) && ( numReqsPD > 0 ) ) ++kAtual;
			numReqsPDDescobertos = numReqsPD;
			while ( numReqsPDDescobertos > 0 )
			{
				v = rand() % numReqsPDDescobertos;
				cargaTotal += g->getCargaVertice(vetorReqsPD[v]);
				if ( cargaTotal > capacidade ) 
				{
					cargaTotal = g->getCargaVertice(vetorReqsPD[v]);
					++kAtual;
					
				}
				if ( kAtual == numVeic ) break;
				aux = vetorReqsPD[v];
				rotas[kAtual].push_back(aux);
				--numReqsPDDescobertos;
				vetorReqsPD[v] = vetorReqsPD[numReqsPDDescobertos];
				vetorReqsPD[numReqsPDDescobertos] = aux;
			}

			if ( kAtual < numVeic ) //Caso haja inviabilidade na atribuicao das requisicoes fixas (CD + PD), inicia outra iteracao
			{
				numReqsCDPDDescobertos = 0;
				for ( int i = 1; i <= nReqs; ++i)
				{
					inseriuVertice = false;
					for ( int j = 0; ( !inseriuVertice && ( j < numReqsCD ) ); ++j )
					{
						if ( i == vetorReqsCD[j] ) inseriuVertice = true;
					}
					for ( int j = 0; ( !inseriuVertice && ( j < numReqsPD ) ); ++j )
					{
						if ( i == vetorReqsPD[j] ) inseriuVertice = true;
					}
					if ( !inseriuVertice ) vetorReqsCDPD[numReqsCDPDDescobertos++] = i;
				}

				if ( ( numReqsCD == 0 ) && ( numReqsPD == 0 ) ) --kAtual;
				while ( ( numReqsCDPDDescobertos > 0 ) && ( kAtual < ( numVeic - 1 ) ) )
				{
					v = rand() % numReqsCDPDDescobertos;
					rotas[++kAtual].push_back(vetorReqsCDPD[v]);
					--numReqsCDPDDescobertos;
					vetorReqsCDPD[v] = vetorReqsCDPD[numReqsCDPDDescobertos];
				}

				//o numero de veiculos alocados eh menor que K, portanto, pega requisicoes ja alocadas e redistribui em veiculos 'vazios'
				inseriuVertice = true;
				if ( ( numReqsCDPDDescobertos == 0 ) && ( kAtual < ( numVeic - 1 ) ) ) 
				{
					k = kAtual;
					while ( ( k >= 0 ) && ( kAtual < ( numVeic - 1 ) ) )
					{
						if ( rotas[k].size() > 1 )
						{
							rotas[++kAtual].push_back(rotas[k].back());
							rotas[k].pop_back();
						}
						--k;
					}
					if ( kAtual < ( numVeic - 1 ) ) inseriuVertice = false;
				}

				while ( numReqsCDPDDescobertos > 0 ) //tem-se todos os K veiculos alocados e requisicoes CD+PD a serem inseridas nos veiculos
				{
					v = rand() % numReqsCDPDDescobertos; //seleciona-se um vertice ainda nao alocado a um veiculo
					kAtual = rand() % numVeic;
					for ( k = kAtual; k < (kAtual+numVeic); ++k ) //tenta alocar o vertice em todos os veiculos da frota (comecando por um aleatorio)
					{
						cargaTotal = 0;
						for ( int z = 0; z < rotas[k%numVeic].size(); ++z) cargaTotal += g->getCargaVertice(rotas[k%numVeic][z]);
						if ( ( cargaTotal + g->getCargaVertice( vetorReqsCDPD[v] ) ) <= capacidade )
						{
							rotas[k%numVeic].push_back(vetorReqsCDPD[v]);
							--numReqsCDPDDescobertos;
							vetorReqsCDPD[v] = vetorReqsCDPD[numReqsCDPDDescobertos];
							break;						
						}
					}
					if ( k == (kAtual+numVeic) )
					{
						inseriuVertice = false;
						break;
					}
				}

				if ( inseriuVertice ) break; //encontrou solucao viavel
			}
		}
	}

	if ( it < 10000 ) //em alguma iteracao foi encontrada uma solucao viavel, portanto, monta-se as rotas
	{
		Rota** solucaoViavel = new Rota*[numVeic+1];
		for ( int k = 0; k < numVeic; ++k )
		{
			solucaoViavel[k] = new Rota();
			solucaoViavel[k]->inserirVerticeInicio(0);
			for ( int z = 0; z < rotas[k].size(); ++z ) solucaoViavel[k]->inserirVerticeFim(rotas[k][z]);
			for ( int z = 0; z < rotas[k].size(); ++z ) solucaoViavel[k]->inserirVerticeFim(rotas[k][z]+nReqs);
			solucaoViavel[k]->inserirVerticeFim(depositoArtificial);
			solucaoViavel[k]->setCustoRota(g);
		}

		delete [] rotas;
		delete [] vetorReqsCDPD;
		solucaoViavel[numVeic] = NULL;
		return solucaoViavel;
	}
	else
	{
		delete [] rotas;
		delete [] vetorReqsCDPD;
		printf("Não encontrou rotas viaveis para k = %d\n", numVeic);
		exit(0);
	}
}

void verificaRotasBasicas(Grafo* g, Rota** rotasBasicas, int* vetorReqsCD, int* vetorReqsPD, int numReqsCD, int numReqsPD, int numVeic){
	vector <int> verticesRota;
	int nReqs = g->getNumReqs(), cargaTotal;
	bool rotaContemReqsCD, rotaContemReqsPD, temVertice;
	int totalVertices = 0, totalReqsCD = 0, totalReqsPD = 0;
	for ( int k = 0; k < numVeic; ++k )
	{
		cargaTotal = 0;
		verticesRota = rotasBasicas[k]->getVertices();
		rotaContemReqsCD = rotaContemReqsPD = temVertice = false;
		for ( int i = 1; i < verticesRota.size()-1; ++i )
		{
			for ( int j = 0; j < numReqsCD; ++j )
			{
				if ( verticesRota[i] == vetorReqsCD[j] )
				{
					rotaContemReqsCD = true;
					++totalReqsCD;
					break;
				}
			}
			for ( int j = 0; j < numReqsPD; ++j )
			{
				if ( verticesRota[i] == vetorReqsPD[j] )
				{
					rotaContemReqsPD = true;
					++totalReqsPD;
					break;
				}
			}
			++totalVertices;
			temVertice = true;
			if ( verticesRota[i] <= nReqs ) cargaTotal += g->getCargaVertice( verticesRota[i] );
		}
		if ( rotaContemReqsCD && rotaContemReqsPD )
		{
			printf("Rota (%d) contem requisicoes fixas como CD e PD\n", k);
			for ( int x = 0; rotasBasicas[x] != NULL; ++x ) rotasBasicas[x]->imprimir();
			exit(0);
			
		}
		if ( cargaTotal > g->getCapacVeiculo() )
		{
			printf("Rota (%d) excede a capacidade (%d)\n", k, cargaTotal);
			for ( int x = 0; rotasBasicas[x] != NULL; ++x ) rotasBasicas[x]->imprimir();
			exit(0);			
		}
		if ( !temVertice )
		{
			printf("Rota (%d) sem nenhuma requisicao\n", k);
			for ( int x = 0; rotasBasicas[x] != NULL; ++x ) rotasBasicas[x]->imprimir();
			exit(0);
		}
	}

	if ( ( totalVertices != ( 2*nReqs ) ) || ( totalReqsCD != numReqsCD ) || ( totalReqsPD != numReqsPD ) )
	{
		printf("A cobertura das rotas nao esta adequada aos conjuntos de requisicoes CD e PD\n");
		for ( int x = 0; rotasBasicas[x] != NULL; ++x ) rotasBasicas[x]->imprimir();
		exit(0);
	}
}
