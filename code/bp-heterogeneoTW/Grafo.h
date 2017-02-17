#ifndef GRAFO_H_
#define GRAFO_H_

#include <fstream>
#include <math.h>

#include "Vertice.h"

const float MAIS_INFINITO=999999;

class Grafo{
	private:
		Vertice** vert; //um grafo eh composto por um vetor de ponteiros para vertices (nao eh matriz, eh vetor de ponteiros)
		
		//matriz tridimensional onde cada celula armazena uma subestimativa de custo para um vertice j: c_ij + c_jk - c_ik
		//a primeira dimensao da matriz define o vertice i, enquanto as outras duas dimensoes da matriz  definem os vertices adjacentes j e k
		float*** subEstimativaVertices;
		
		//vetor que armazenara a posicao original de um vertice 
		//(as posicoes podem mudar devido a eliminacao de vertices duais)
		int *vetorPosicaoOriginal;
		
		//quantas commodities circularao (quantos fornecedores tem) ou (quantos fornecedores tem)
		int numCmdt;
		
		//o numero de vertices sera dado por 2*numCommodities + 2 (deposito e copia do deposito)
		int numVertices;
		
		//Como o numero de vertices para o grafo dual pode variar (devido a eliminacao de vertices devido aos custos no problema)
		//Eh necessario armazenar o numero de fornecedores e consumidores duais
		int numFornDuais, numConsDuais;

		//sera o limite imposto pela instancia quanto ao numero de veiculos
		int maxVeicInstancia;
		
		//capacidade do veiculo determinada tambem na leitura da instancia
		int capacVeiculo;

	public:		
		int getNumCmdt();
		int getNumVertices();
		int getNumFornDuais();
		int getNumConsDuais();
		int getMaxVeicInstancia();
		int getCapacVeiculo();
		int getPesoCommodity(int);
		float getCustoAresta(int, int);
		float getCustoArestaDual(int, int);
		float getCustoVerticeDual(int);
		int* getVetorPosicaoOriginal();
		//Usada apenas na classe NoArvore para podar um no por capacidade
		int getPesoCommoditySemCorrigirPosicao(int);
		int getMenorInicioJanelaCons();
		int getMaiorFimJanelaForn();
		int getTempoServico(int);
		int getInicioJanela(int);
		int getFimJanela(int);

		void setCustoVerticeDual(int, float);
		void setCustoArestasDual(char);
		void setCustoArestaDual(int, int, float);
		void setCapacVeiculo(int);
		void setPesoCommodity(int, int);

		void imprimeGrafo();
		void imprimeGrafoDual();
		void calculaSubEstimativaVertices();
		void eliminaVerticesDuais();

		Grafo(char*, int);		
		~Grafo();
};

#endif
