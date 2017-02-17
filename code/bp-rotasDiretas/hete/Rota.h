#ifndef ROTA_H_
#define ROTA_H_

#include "Grafo.h"
#include <vector>
using namespace std;


class Rota{
	private:
		vector< int > vertices;
		float custo;
		float custoReduzido;
		int numApontadores;
		bool basica;
		
	public:
		Rota();
		~Rota();
		
		void setVertices(vector<int>);
		vector<int> getVertices();
		
		void setCusto(Grafo*, int);
		float getCusto();

		void setCustoReduzido(float);
		void setCustoReduzido(Grafo*);
		float getCustoReduzido();

		void incrNumApontadores();
		bool decrNumApontadores();
		int getNumApontadores();

		void setBasica();
		bool getBasica();

		void inserirVerticeFim(int);
		void inserirVerticeInicio(int);
		bool verificaVerticeRepetido();
		void imprimir();
};

#endif /*ROTA_H_*/
