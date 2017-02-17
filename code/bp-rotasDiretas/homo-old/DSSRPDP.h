#ifndef DSSRPDP_H_
#define DSSRPDP_H_
#include "Rota.h"
#include "Grafo.h"
#include <string.h>

typedef struct strLabelDSSR *ptrLabelDSSR;
typedef struct strLabelDSSR
{
	int numVisitados, numVisitadosU, ultimoVertice, cargaAcumulada;
	unsigned long long int vertVisitados, vertVisitadosU;
	ptrLabelDSSR pai, prox;
	float custoDual;

	strLabelDSSR(unsigned long long int vertVis, unsigned long long int vertVisU, int numVisit, int numVisitU, int ult, int carga, float custo, ptrLabelDSSR p, ptrLabelDSSR px) :
	vertVisitados(vertVis), vertVisitadosU(vertVisU), numVisitados(numVisit),numVisitadosU(numVisitU), ultimoVertice(ult), cargaAcumulada(carga), custoDual(custo), pai(p), prox(px){}
} LabelDSSR;

struct vLabelDSSR
{
	ptrLabelDSSR cabeca;
	ptrLabelDSSR posAtual;
	ptrLabelDSSR calda;
};

class DSSRPDP
{
	private:
		vLabelDSSR* vetorLabels;
		int nRequests, nVertices;
		bool encontrouRotaNegativa;
		ptrLabelDSSR menorLabelObtido;
		float menorCustoAtual, menorCustoObtido, valorSubtrair;
		unsigned long long int verticesFixarCoding;
		unsigned long long int *verticesCoding;
		int* indiceVertDual;

	public:
		DSSRPDP(Grafo*, float);
		~DSSRPDP();

		int calculaCaminhos(Grafo*);
		int calculaCaminhoPDP(Grafo*);
		bool verificaLabelDominadoEDominaF(Grafo*, int, float, int, unsigned long long int, unsigned long long int);
		bool verificaLabelDominadoEDominaC(Grafo*, int, float, int, unsigned long long int);
		Rota** getRotaCustoMinimo(Grafo*, float);
};

#endif
