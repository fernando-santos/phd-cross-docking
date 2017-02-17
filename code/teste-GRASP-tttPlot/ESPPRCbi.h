#ifndef ESPPRCBI_H_ /*ESSA VERSAO IMPLEMENTA O CONJUNTO DE VERTICES VISITADOS DE UM LABEL COMO UNREACHABLE (BY FEILLET)*/
#define ESPPRCBI_H_

#include <sys/time.h>
#include <iostream>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"

typedef struct strLabelBi *ptrLabelBi;
typedef struct strLabelBi
{
	int numVisitados, numVisitadosU, ultimoVertice, cargaAcumulada;
	unsigned long long int vertVisitados, vertVisitadosU;
	ptrLabelBi pai, prox;
	float custoDual;

	strLabelBi(unsigned long long int vertVis, unsigned long long int vertVisU, int numVisit, int numVisitU, int ult, int carga, float custo, ptrLabelBi p, ptrLabelBi px) : vertVisitados(vertVis), vertVisitadosU(vertVisU), numVisitados(numVisit),numVisitadosU(numVisitU), ultimoVertice(ult), cargaAcumulada(carga), custoDual(custo), pai(p), prox(px){}
} LabelBi;

struct vLabelBi
{
	ptrLabelBi cabeca;
	ptrLabelBi posAtual;
	ptrLabelBi calda;
};

struct rotaBi
{
	float custo;
	ptrLabelBi forward, backward;
	rotaBi (float c, ptrLabelBi f, ptrLabelBi b) : custo(c), forward(f), backward(b) {}
};

class ESPPRCbi
{
	private:
		ptrLabelBi lixo;
		float* vetorXi;
		int* indiceVertDual;
		vLabelBi* vetorLabels;
		int nRequests, nVertices;
		float alfa, menorCustoAtual, menorCustoObtido;
		unsigned long long int *verticesCoding;
		vector < rotaBi* > violados;

	public:
		ESPPRCbi(Grafo*, float*, float);
		~ESPPRCbi();

		int calculaCaminhoElementarBi(Grafo*);
		bool verificaLabelDominadoEDomina(Grafo*, unsigned long long int, unsigned long long int, int, int, int, float);
		Rota** getRotaCustoMinimo(Grafo*, float);
};

#endif
