#ifndef ELEMENTAR_H_ /*ESSA VERSAO IMPLEMENTA O CONJUNTO DE VERTICES VISITADOS DE UM LABEL COMO UNREACHABLE (BY FEILLET)*/
#define ELEMENTAR_H_

#include <sys/time.h>
#include <iostream>
#include "Grafo.h"
#include "Rota.h"

typedef struct strLabel *ptrLabel;
typedef struct strLabel{
	unsigned long long int vertVisitados;
	int numVertVisitados;
	int ultimoVertice;
	int verticeAntecessor;
	int cargaAcumulada;
	float custoDual;
	ptrLabel prox;
	ptrLabel pai;

	strLabel(unsigned long long int vertVis, int numVertVis, int ult, int antec, int carga, float custo, ptrLabel p) 
				: vertVisitados(vertVis), numVertVisitados(numVertVis), ultimoVertice(ult), 
				verticeAntecessor(antec), cargaAcumulada(carga), custoDual(custo), prox(p){}
} Label;

struct vLabel{
	ptrLabel cabeca;
	ptrLabel posAtual;
	ptrLabel calda;
};

class Elementar{
	private:
		vLabel* vetorLabels;
		ptrLabel menorLabelObtido;
		float menorCustoAtual, menorCustoObtido, valorSubtrair;
		unsigned long long int *verticesCoding;
		unsigned long long int *verticesCodingCompl;
		int numCommodities, ajuste; //ajusta o indice dos vertices no grafo para calcular o caminho de consumidores
		
	public:
		Elementar(Grafo*, bool, float);
		~Elementar();

		int calculaCaminhoElementar(Grafo*, bool);

		bool verificaLabelDominadoEDomina(Grafo*, int, float, int, int, unsigned long long int);
		
		Rota** getRotaCustoMinimo(Grafo*, float);
		
		void imprimeLabel(ptrLabel);
		
		void verificaLabelMenorCusto(Grafo*);

		char* decToBin(unsigned long long int);
};

#endif 

