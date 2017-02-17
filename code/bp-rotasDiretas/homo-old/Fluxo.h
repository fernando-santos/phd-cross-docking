#ifndef FLUXO_H_
#define FLUXO_H_

#include <ilcplex/ilocplex.h>
#include "Grafo.h"
#include "dinic.h"
#include <vector>
using namespace std;

class Fluxo{
	private:
		GRAPH* grafoDinic;
		vector<int> conjuntoW;
		vector<bool> visitadosDFS;
		int nVerticesDinic, nVerticesDinicForn;
		int *mapeiaVerticesDinicIda, *mapeiaVerticesDinicVolta;

	public:
		Fluxo(IloArray <IloNumArray>, IloNumArray, int);
		~Fluxo();

		vector < int >& calculaFluxoMaximo(IloNumArray, int);
		void atualizaGrafoDinic(Grafo*, IloNumArray, vector < int>&);
		void dfs(int);
		int getNVerticesDinic();
};

#endif
