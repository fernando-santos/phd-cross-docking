#ifndef GRAFO_H_
#define GRAFO_H_

#include <fstream>
#include <math.h>

#include "Vertice.h"

const float MAIS_INFINITO=99999;

class Grafo{
	private:
		Vertice** vert; //um grafo eh composto por um vetor de ponteiros para vertices (nao eh matriz, eh vetor de ponteiros)
		
		//Valor a ser usado pelo algoritmo de caminho elementar para eliminar labels usando uma estimativa do custo ao passar pelo vertice
		float* subEstimativaCustos;
		
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
		int getPesoCommodity(int);
		float getCustoAresta(int, int);
		float getCustoArestaDual(int, int);
		float getCustoVerticeDual(int);
		float getSubEstimativaCustos(int);
		int getTempoServico(int);
		int getInicioJanela(int);
		int getFimJanela(int);
		
		void setCustoVerticeDual(int, float);
		void setCustoArestasDual();
		void setCapacVeiculo(int);
		void setPesoCommodity(int, int);

		void imprimeGrafo();
		void imprimeGrafoDual();
		void calculaSubEstimativaCustos();
		
		Grafo(char*, int);		
		~Grafo();
};

#endif

