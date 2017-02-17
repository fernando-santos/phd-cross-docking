#ifndef GRAFO_H_
#define GRAFO_H_

#include <fstream>
#include <math.h>
#include <stdlib.h>

#include "Vertice.h"

const float MAIS_INFINITO=999999999;

class Grafo{
	private:
		Vertice** vert; //um grafo eh composto por um vetor de ponteiros para vertices (nao eh matriz, eh vetor de ponteiros)
		
		//quantas commodities circularao (quantos fornecedores tem) ou (quantos fornecedores tem)
		int numCmdt;

		//o numero de vertices sera dado por 2*numCommodities + 2 (deposito e copia do deposito)
		int numVertices;

		//sera o limite imposto pela instancia quanto ao numero de veiculos
		int maxVeic;

		//capacidade do veiculo determinada tambem na leitura da instancia
		int* capacVeiculo;

	public:
		int getNumCmdt();
		int getNumVertices();
		int getMaxVeic();
		int getCapacVeiculo(int);
		int getPesoCommodity(int);
		float getCustoAresta(int, int, int);
		float getCustoArestaDual(int, int);
		float getCustoVerticeDual(int);
		bool existeArestaRD(int, int);

		void setCustoVerticeDual(int, float);
		void setCustoArestasDual(int);
		void setCustoArestaDual(int, int, float);
		void setCapacVeiculo(int, int);
		void setPesoCommodity(int, int);

		void imprimeGrafo(int);
		void imprimeGrafoDual(int);

		Grafo(char*, float*, int, int);
		~Grafo();
};

#endif
