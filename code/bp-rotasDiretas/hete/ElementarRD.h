#ifndef ELEMENTAR_RD_H_ /*ESSA VERSAO IMPLEMENTA O CONJUNTO DE VERTICES VISITADOS DE UM LABEL COMO UNREACHABLE (BY FEILLET)*/
#define ELEMENTAR_RD_H_

#include <sys/time.h>
#include <iostream>
#include "Grafo.h"
#include "Rota.h"

typedef struct strLabelRD *ptrLabelRD;
typedef struct strLabelRD{
	unsigned long long int vertVisitados1, vertVisitados2;
	int numVertVisitados1, numVertVisitados2;
	int ultimoVertice;
	int verticeAntecessor;
	int cargaAcumulada;
	float custoDual;
	ptrLabelRD prox;
	ptrLabelRD pai;

	strLabelRD(unsigned long long int vertVis1, unsigned long long int vertVis2, int numVertVis1, int numVertVis2, int ult, int antec,
			int carga, float custo, ptrLabelRD p) : vertVisitados1(vertVis1), vertVisitados2(vertVis2), numVertVisitados1(numVertVis1),
			numVertVisitados2(numVertVis2), ultimoVertice(ult), verticeAntecessor(antec), cargaAcumulada(carga), custoDual(custo), prox(p){}
} LabelRD;

struct vLabelRD{
	ptrLabelRD cabeca;
	ptrLabelRD posAtual;
	ptrLabelRD calda;
};

class ElementarRD{
	private:
		vLabelRD* vetorLabels;
		ptrLabelRD menorLabelObtido, lixo;
		float menorCustoAtual, menorCustoObtido, valorSubtrair;
		unsigned long long int *verticesCoding;
		int numCommodities;

	public:
		ElementarRD(Grafo*, float);
		~ElementarRD();

		int calculaCaminhoElementar(Grafo*, int, int);

		bool verificaLabelDominadoEDominaF(Grafo*, unsigned long long int, unsigned long long int, int, int, int, float);
		bool verificaLabelDominadoEDominaC(Grafo*, unsigned long long int, unsigned long long int, int, int, int, float);
		
		Rota** getRotaCustoMinimo(Grafo*, int, float);
		
		void imprimeLabel(ptrLabelRD);
		
		void verificaLabelMenorCusto(Grafo*);

		char* decToBin(unsigned long long int);
};

#endif 
