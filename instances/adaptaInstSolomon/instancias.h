#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
using namespace std;

#define MAX_VERTICES 100

struct Vertice{
	int posX;
	int posY;
	int demanda;
	int tInicio;
	int tFim;
	int service;
};
