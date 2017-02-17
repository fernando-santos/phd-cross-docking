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
		
		int* indiceVerticesDual;
		int numReqs, numReqsDual;
		int numVertices, numVerticesDual;
		int maxVeic, capacVeiculo;

	public:
		~Grafo();
		Grafo(char*, int, char);

		int getMaxVeic();
		int getNumReqs();
		int getNumVertices();
		int getCapacVeiculo();
		int getCargaVertice(int);
		bool existeAresta(int, int);
		float getCustoAresta(int, int, bool RD = false);
		
		int getNumReqsDual();
		int getNumVerticesDual();
		bool existeArestaDual(int, int);
		float getCustoArestaDual(int, int);
		float getCustoVerticeDual(int);
		int* getIndiceVerticesDual();

		void setCustoVerticeDual(int, float);
		void setCustoArestasDual(int*, int, bool);
		void setCustoArestaDual(int, int, float);
		void setCapacVeiculo(int);
		void setCargaVertice(int, int);

		void imprimeGrafo(bool);
		void imprimeGrafoDual();
};

#endif
