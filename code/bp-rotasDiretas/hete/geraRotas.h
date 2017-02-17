#ifndef GERAROTAS_H_
#define GERAROTAS_H_

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"

Rota** geraPoolRotas(Grafo*, bool, int);

typedef struct strNoListaEnc* ptrNoListaEnc;
typedef struct strNoListaEnc{
	int vertice;
	ptrNoListaEnc prox;
}NoListaEnc;

#endif
