#ifndef ELEMENTARTW_H_ /*ESSA VERSAO IMPLEMENTA O CONJUNTO DE VERTICES VISITADOS DE UM LABEL COMO UNREACHABLE (BY GEANDRAU)*/
#define ELEMENTARTW_H_ /*ESSA EH A UNICA MELHORIA PROPORCIONADA, COM RELACAO A OUTRA IMPLEMENTACAO */

#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"

Rota** geraPoolRotas(Grafo*, bool, int, int, int, int&);

typedef struct strRotaInicial* ptrRotaInicial;
typedef struct strRotaInicial{
	int vertice;
	float tempoChegada;
	ptrRotaInicial prox;
}RotaInicial;

typedef struct strLabelTW* ptrLabelTW;
typedef struct strLabelTW{
	unsigned long long int vertVisitados;
	int numVertVisitados;
	int ultimoVertice;
	int verticeAntecessor;
	int cargaAcumulada;
	float tempoAcumulado;
	float custoDual;
	ptrLabelTW prox;
	ptrLabelTW pai;

	strLabelTW(unsigned long long int vertVis, int numVertVis, int ult, int antec, int carga, float tempo, float custo, ptrLabelTW p) 
				: vertVisitados(vertVis), numVertVisitados(numVertVis), ultimoVertice(ult), 
				verticeAntecessor(antec), cargaAcumulada(carga), tempoAcumulado(tempo), custoDual(custo), prox(p){}
	strLabelTW(unsigned long long int vertVis, int ult, int antec, int carga, float tempo, float custo) : vertVisitados(vertVis),
				ultimoVertice(ult), verticeAntecessor(antec), cargaAcumulada(carga), tempoAcumulado(tempo), custoDual(custo){}
} LabelTW;

struct vLabelTW{
	ptrLabelTW cabeca;
	ptrLabelTW posAtual;
	ptrLabelTW calda;
};

class ElementarTW{
	private:
		vLabelTW* vetorLabels;
		ptrLabelTW menorLabelObtido;
		float menorCustoAtual, menorCustoObtido, valorSubtrair;
		unsigned long long int *verticesCoding;
		unsigned long long int *verticesCodingCompl;
		int numCommodities, ajuste; //ajusta o indice dos vertices no grafo para calcular o caminho de consumidores
		
	public:
		ElementarTW(Grafo*, bool, float);
		~ElementarTW();

		int calculaCaminhoElementar(Grafo*, int, int);

		bool verificaLabelDominadoEDomina(Grafo*, int, float, int, float, int, unsigned long long int);
		
		Rota** getRotaCustoMinimo(Grafo*, float);
		
		void verificaLabelMenorCusto(Grafo*);
		
		ptrLabelTW getLabel(unsigned long long int, int);
};

#endif 

