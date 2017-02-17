#ifndef GRAFO_H_
#define GRAFO_H_

#include <fstream>
#include <math.h>
#include <stdlib.h>
#include "Vertice.h"
#include <vector>

class Grafo{
	private:
		Vertice** vert; //um grafo eh composto por um vetor de ponteiros para vertices (nao eh matriz, eh vetor de ponteiros)
		
		int numReqs;
		int numVertices;
		int maxVeic;
		int capacVeiculo;

	public:
		~Grafo();
		Grafo(char*, char*, int, int);
		Grafo(char*, int);

		int getNumReqs();
		int getMaxVeic();
		int getNumVertices();
		int getCapacVeiculo();
		int getCargaVertice(int);
		bool existeAresta(int, int);
		float getCustoAresta(int, int);
		float getCustoArestaDual(int, int);
		float getCustoVerticeDual(int);

		void setCustoVerticeDual(int, float);
		void setCustoArestasDual();
		void setCustoArestaDual(int, int, float);
		void setCapacVeiculo(int);
		void setCargaVertice(int, int);

		void imprimeGrafo();
		void imprimeGrafoDual();
		void imprimeConexoes();
};

#endif
