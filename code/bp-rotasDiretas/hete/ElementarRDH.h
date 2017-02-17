#ifndef ELEMENTAR_RDH_H_ /*ESSA VERSAO IMPLEMENTA O CONJUNTO DE VERTICES VISITADOS DE UM LABEL COMO UNREACHABLE (BY FEILLET)*/
#define ELEMENTAR_RDH_H_

#include "ElementarRD.h"

class ElementarRDH{
	private:
		vLabelRD* vetorLabels;
		ptrLabelRD menorLabelObtido, lixo;
		float menorCustoAtual, menorCustoObtido, valorSubtrair;
		unsigned long long int *verticesCoding;
		int numCommodities;

	public:
		ElementarRDH(Grafo*, float);
		~ElementarRDH();

		int calculaCaminhoElementar(Grafo*, int);

		bool verificaLabelDominadoEDominaF(Grafo*, unsigned long long int, int, int, int, float);
		bool verificaLabelDominadoEDominaC(Grafo*, unsigned long long int, unsigned long long int, int, int, int, float);
		
		Rota** getRotaCustoMinimo(Grafo*, int, float);
		
		void imprimeLabel(ptrLabelRD);
		
		void verificaLabelMenorCusto(Grafo*);

		char* decToBin(unsigned long long int);
};

#endif 
