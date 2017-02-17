#include "Rota.h"

Rota::Rota(){
	custo = 0;
	custoReduzido = 0;
	numApontadores = 0;
}

Rota::~Rota(){

}

void Rota::setVertices(vector<int> v){
	vertices = v;
}

vector<int> Rota::getVertices(){
	return vertices;
}

void Rota::setCusto(Grafo* G){
	int tam = vertices.size();
	custo = G->getCustoAresta(0, vertices[1]);
	for (int i = 2; i < tam; ++i){
		custo += G->getCustoAresta(vertices[i-1], vertices[i]);
	}
}

float Rota::getCusto(){
	return custo;
}

void Rota::setCustoReduzido(float custoR){
	custoReduzido = custoR;
}

void Rota::setCustoReduzido(Grafo* G){
	int tam = vertices.size();
	custoReduzido = G->getCustoArestaDual(0, vertices[1]);
	for (int i = 2; i < tam; ++i){
		custoReduzido += G->getCustoArestaDual(vertices[i-1], vertices[i]);
	}
}

float Rota::getCustoReduzido(){
	return custoReduzido;
}

void Rota::incrNumApontadores(){
	++numApontadores;
}

bool Rota::decrNumApontadores(){
	if (--numApontadores <= 0) return true;
	return false;
}

void Rota::inserirVerticeFim(int v){
	vertices.push_back(v);
}

void Rota::inserirVerticeInicio(int v){
	vertices.insert(vertices.begin(), 1, v);
}

bool Rota::verificaVerticeRepetido(){
	int tam = vertices.size();
	for (int i = 0; i < tam; ++i){
		for (int j = i+1; j < tam; ++j){
			if (vertices[i] == vertices[j]){
				return true;
			}
		}
	}
	return false;
}

void Rota::imprimir(){
	int tam = vertices.size();
	printf ("(%05.2f | %05.2f) [ ", custo, custoReduzido);
	for (int i = 0; i < tam; ++i){
		printf ("%d ", vertices[i]);
	}
	printf ("]\n");
}
