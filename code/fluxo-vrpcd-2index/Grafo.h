#ifndef GRAFO_H_
#define GRAFO_H_

#include <fstream>
#include <iostream>
#include <math.h>

#include "Vertice.h"

const float MAIS_INFINITO=9999999;

class Grafo{
	private:
		Vertice** vert; //um grafo eh composto por um vetor de ponteiros para vertices (nao eh matriz, eh vetor de ponteiros)
		
		//quantas commodities circularao (quantos fornecedores tem) ou (quantos fornecedores tem)
		int numCmdt;
		
		//o numero de vertices sera dado por 2*numCommodities + 2 (deposito e copia do deposito)
		int numVertices;
		
		//sera o limite imposto pela instancia quanto ao numero de veiculos
		int maxVeicInstancia;
		
		//capacidade do veiculo determinada tambem na leitura da instancia
		int capacVeiculo;
		
		
	public:		
		int getNumCmdt();
		int getNumVertices();
		int getMaxVeicInstancia();
		int getCapacVeiculo();
		float getCustoAresta(int, int);
		void imprimeGrafo();
		
		int getPesoCommodity(int);
		int getMenorInicioJanelaCons();
		int getMaiorFimJanelaForn();
		int getTempoServico(int);
		int getInicioJanela(int);
		int getFimJanela(int);
		
		Grafo(char*, int);		
		~Grafo();
};

#endif
