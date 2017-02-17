#ifndef ESPPRC_H_ /*ESSA VERSAO IMPLEMENTA O CONJUNTO DE VERTICES VISITADOS DE UM LABEL COMO UNREACHABLE (BY FEILLET)*/
#define ESPPRC_H_

#include <sys/time.h>
#include <iostream>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"

typedef struct strLabel *ptrLabel;
typedef struct strLabel{
	//dois conjuntos de vertices visitados para o fornecedor: um insere 1 em labels 'unreachable' e outro nao (permite controlar as trocas de mercadorias)
	int numVisitadosF, numVisitadosC, ultimoVertice, verticeAntecessor, cargaAcumulada;
	unsigned long long int vertVisitadosF, vertVisitadosF2, vertVisitadosC;
	ptrLabel pai, prox;
	float custoDual;

	strLabel(unsigned long long int vertVisF, unsigned long long int vertVisF2, unsigned long long int vertVisC, int numVisitF, int numVisitC, int ult, int antec,
			int carga, float custo, ptrLabel p, ptrLabel px) : vertVisitadosF(vertVisF), vertVisitadosF2(vertVisF2), vertVisitadosC(vertVisC), numVisitadosF(numVisitF),
			numVisitadosC(numVisitC), ultimoVertice(ult), verticeAntecessor(antec), cargaAcumulada(carga), custoDual(custo), pai(p), prox(px){}
} Label;

struct vLabel{
	ptrLabel cabeca;
	ptrLabel posAtual;
	ptrLabel calda;
};

class ESPPRC{
	private:
		ptrLabel lixo;
		float* vetorXi;
		int* indiceVertDual;
		vLabel* vetorLabels;
		ptrLabel menorLabelObtido;
		int nRequests, nVertices;
		float alfa, menorCustoAtual, menorCustoObtido;
		unsigned long long int *verticesCoding;

	public:
		ESPPRC(Grafo*, float*, float);
		~ESPPRC();

		int calculaCaminhoElementar(Grafo*);
		bool verificaLabelDominadoEDomina(Grafo*, unsigned long long int, unsigned long long int, unsigned long long int, int, int, int, float);		
		Rota** getRotaCustoMinimo(Grafo*, float);
		void imprimeLabel(ptrLabel);
		char* decToBin(unsigned long long int);
};

#endif
