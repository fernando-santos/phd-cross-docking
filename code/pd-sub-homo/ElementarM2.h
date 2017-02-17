#ifndef ELEMENTARM2_H_ /*ESSA VERSAO IMPLEMENTA O CONJUNTO DE VERTICES VISITADOS DE UM LABEL COMO UNREACHABLE (BY FEILLET)*/
#define ELEMENTARM2_H_

#include <sys/time.h>
#include <iostream>
#include "Grafo.h"
#include "Rota.h"

typedef struct strLabel *ptrLabel;
typedef struct strLabel{
	//dois conjuntos (e suas cardinalidades) de vertices visitados para o fornecedor: um insere 1 em labels 'unreachable' e outro nao (permite controlar as trocas de mercadorias)
	unsigned long long int vertVisitadosF, vertVisitadosC;
	int numVertVisitadosF, numVertVisitadosC;
	int ultimoVertice, verticeAntecessor;
	int cargaAcumulada;
	float custoDual;
	ptrLabel prox;
	ptrLabel pai;

	strLabel(unsigned long long int vertVisF, unsigned long long int vertVisC, int numVertVisF, int numVertVisC, int ult, int antec, 
			int carga, float custo, ptrLabel p)	: vertVisitadosF(vertVisF), vertVisitadosC(vertVisC), numVertVisitadosF(numVertVisF), 
			numVertVisitadosC(numVertVisC), ultimoVertice(ult), verticeAntecessor(antec), cargaAcumulada(carga), custoDual(custo), prox(p){}
} Label;

struct vLabel{
	ptrLabel cabeca;
	ptrLabel posAtual;
	ptrLabel calda;
};

class Elementar{
	private:
		vLabel* vetorLabels;
		float *vetorXi, maiorXi;
		ptrLabel menorLabelObtido;
		unsigned long long int *verticesCoding;
		float alfa, menorCustoAtual, menorCustoObtido;
		int nRequests, nVertices, *vetorCargaProcessado;

	public:
		Elementar(Grafo*, float*, float);
		~Elementar();

		int calculaCaminhoElementar(Grafo*);
		bool verificaLabelDominadoEDominaF(Grafo*, unsigned long long int, int, int, float, int);
		bool verificaLabelDominadoEDominaC(Grafo*, unsigned long long int, unsigned long long int, int, int, int, float, int);		
		Rota* getRotaCustoMinimo(Grafo*);		
		void imprimeLabel(ptrLabel);		
		void verificaLabelMenorCusto(Grafo*);
		char* decToBin(unsigned long long int);
};

#endif 

