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
		
	public:
		Rota();
		~Rota();
		
		void setVertices(vector<int>);
		vector<int> getVertices();

		float getCusto();
		void setCusto(float);
		void setCustoRota(Grafo*, bool RD = false);

		float getCustoReduzido();
		void setCustoReduzido(float);

		void incrNumApontadores();
		bool decrNumApontadores();
		int getNumApontadores();

		void inserirVerticeFim(int);
		void inserirVerticeInicio(int);
		bool verificaVerticeRepetido();
		void imprimir();
};

#endif /*ROTA_H_*/
