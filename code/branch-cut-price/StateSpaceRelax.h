#ifndef STATESPACERELAX_H_
#define STATESPACERELAX_H_

#include "string.h"
#include "Elementar.h"

class StateSpaceRelax{
	private:
		vLabel* vetorLabels;
		ptrLabel menorLabelObtido;
		float menorCustoAtual, menorCustoObtido, valorSubtrair;
		int numCommodities, numVerticesFixar, ajuste;
		unsigned long long int verticesFixarCoding;
		unsigned long long int *verticesCoding;
		short int* verticesFixar;
		bool encontrouRotaNegativa;

	public:
		StateSpaceRelax(Grafo*, bool, float);
		~StateSpaceRelax();

		int calculaCaminhoElementarSSR(Grafo*);

		int calculaCaminhos(Grafo*);

		bool verificaLabelDominadoEDomina(Grafo*, int, float, int, int, unsigned long long int);
		
		Rota** getRotaCustoMinimo(Grafo*, float);
};

#endif 

