#ifndef GERAROTAS_H_
#define GERAROTAS_H_

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"

Rota** geraRotasBasicas(Grafo*, int*, int*, int, int, int);
void verificaRotasBasicas(Grafo*, Rota**, int*, int*, int, int, int);

#endif
