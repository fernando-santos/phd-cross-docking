#include "Fluxo.h"

Fluxo::Fluxo(IloArray <IloNumArray> x_values, IloNumArray y_values, int nRequests){
	int nVertices = 2*nRequests;
	int capacidadeInteira, depositoArtificial = nVertices+1;
	mapeiaVerticesDinicIda = new int[depositoArtificial];
	mapeiaVerticesDinicVolta = new int[depositoArtificial];
	mapeiaVerticesDinicIda[0] = 0;
	mapeiaVerticesDinicVolta[0] = 0;
	nVerticesDinicForn = 0;
	nVerticesDinic = 1;

	//Verifica quais vertices participam da solucao atual do modelo para inclui-los no grafo (os demais vertices nao sao necessarios)
	for (int i = 1; i <= nVertices; ++i){
		if (y_values[i] > 0.00001){
			mapeiaVerticesDinicIda[i] = nVerticesDinic;
			mapeiaVerticesDinicVolta[nVerticesDinic] = i;
			if (i <= nRequests) ++nVerticesDinicForn;
			++nVerticesDinic;
		}
	}

	//cria-se o grafo (GRAPH) baseando-se nos vertices e arestas por onde passa fluxo na solucao original
	grafoDinic = (GRAPH *) malloc (sizeof(GRAPH));
	InitGraph(grafoDinic);
	for (int i = 0; i < nVerticesDinic; ++i){
		AddVertex(i, grafoDinic);
	}

	//Cria todas as possiveis arestas deste grafo (todas com capacidade 0)
	for(int j = 1; j <= nVerticesDinicForn; ++j){
		AddEdge(0, j, 0, grafoDinic); //arestas que saem do DEPOSITO (0)
	}
	for(int i = 1; i <= nVerticesDinicForn; ++i){
		for(int j = 1; j < nVerticesDinic; ++j){
			if (i != j) AddEdge(i, j, 0, grafoDinic); //arestas que saem dos FORNECEDORES
		}
	}
	for(int i = (nVerticesDinicForn+1); i < nVerticesDinic; ++i){
		for(int j = (nVerticesDinicForn+1); j < nVerticesDinic; ++j){
			if (i != j) AddEdge(i, j, 0, grafoDinic); //arestas que saem dos CONSUMIDORES
		}
	}

	//insere as arestas com capacidade > 0. OBS.: a capacidade sera o seu valor de relaxacao linear
	for (int i = 0; i <= nVertices; ++i){
		for (int j = 1; j < depositoArtificial; ++j){
			if (x_values[i][j] > 0.00001){
				capacidadeInteira = floor(x_values[i][j] * 1000000.0);
				AddEdge(mapeiaVerticesDinicIda[i], mapeiaVerticesDinicIda[j], capacidadeInteira, grafoDinic);
			}
		}
	}
}

Fluxo::~Fluxo(){
	delete [] mapeiaVerticesDinicIda;
	delete [] mapeiaVerticesDinicVolta;
	freeGraph(grafoDinic);
	free(grafoDinic);
}

//Se o fluxo == y_values[i] de 0 -> i significa q existe um caminho e nao existe corte para viola-lo
//Mas no caso de fluxo < y_values[i], eh possivel afirmar que de 0 -> i existe um caminho 'inviavel', podendo ser estabelecido um corte
//a busca em profundidade parte do deposito e marca como 'true' aqueles vertices conectados com o deposito (conjunto W)
//os demais vertices (incluindo os que nao foram para o grafo dinic) representam V\W. O corte eh definido nas arestas x(W, V\W)
vector < int >& Fluxo::calculaFluxoMaximo(IloNumArray y_values, int destino){
	conjuntoW.clear();
	int fluxoInteiro = FindFlow( grafoDinic, 0, destino, DINIC_NEW );

	if ((fluxoInteiro / 1000000.0) < (y_values[mapeiaVerticesDinicVolta[destino]] - 0.001)){
		//atribui 'false' para todos os elementos da lista e executa uma busca em produndidade
		//ao final do dfs, o conjunto W estara marcado como 'true' e V/W como 'false', sendo possivel identificar o corte
		visitadosDFS.assign(nVerticesDinic, false);
		dfs(0);

		//aqueles vertices marcados como 'true' na busca em profundidade estao em W
		for(int i = 0; i < nVerticesDinic; ++i){
			if (visitadosDFS[i]) conjuntoW.push_back(mapeiaVerticesDinicVolta[i]);
		}
	}
	return conjuntoW;
}

void Fluxo::atualizaGrafoDinic(Grafo* g, IloNumArray y_values, vector < int>& arestasCorte){
	int tam = arestasCorte.size();
	for (int i = 1; i < tam; i+=2){
		//eh possivel que existam arestas cuja extremidade final nao esteja no grafo dinic, por este motivo eh necessario o teste abaixo
		if ( y_values[arestasCorte[i]] > 0.00001 )
		{ 
			AddEdge(mapeiaVerticesDinicIda[arestasCorte[i-1]], mapeiaVerticesDinicIda[arestasCorte[i]], 1000000.0, grafoDinic);
		}
	}
}

void Fluxo::dfs( int v ){
	EDGE2* edgesDinic = grafoDinic->A[v];
	visitadosDFS[v] = true;

	while ( edgesDinic != ((EDGE2 *) 0)){
		int r = (edgesDinic->c - edgesDinic->f), w = edgesDinic->h;
		if ( ( !visitadosDFS[w] ) && ( r > 0 ) )
		{
			dfs(w);
		}
		edgesDinic = edgesDinic->next;
	}
}

int Fluxo::getNVerticesDinic(){
	return nVerticesDinic;
}

