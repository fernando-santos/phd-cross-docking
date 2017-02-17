#ifndef PDP_H_
#define PDP_H_

#include "ElementarRD.h"

class PDP{
	private:
		int numRequests;
		vLabelRD* vetorLabels;
		ptrLabelRD menorLabelObtido, lixo;
		float menorCustoAtual, menorCustoObtido, valorSubtrair;
		unsigned long long int *verticesCoding;

	public:
		~PDP();
		PDP(Grafo*, float);

		int calculaCaminhoElementar(Grafo*, int);
		bool verificaLabelDominadoEDomina(Grafo*, unsigned long long int, unsigned long long int, int, int, int, int, float);
		Rota** getRotaCustoMinimo(Grafo*, float);
};

#endif 
