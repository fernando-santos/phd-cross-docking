#ifndef GRAFO_H_
#define GRAFO_H_

#include <fstream>
#include <math.h>
#include <stdlib.h>
#include "Vertice.h"

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
		int getCargaVertice(int);
		float getCustoAresta(int, int);
		float getCustoArestaDual(int, int);
		float getCustoVerticeDual(int);

		void setCustoVerticeDual(int, float);
		void setCustoArestasDual(char);
		void setCustoArestaDual(int, int, float);
		void setCapacVeiculo(int);
		void setPesoCommodity(int, int);

		void imprimeGrafo();
		void imprimeGrafoDual();

		Grafo(char*, int);		
		~Grafo();
};

#endif
