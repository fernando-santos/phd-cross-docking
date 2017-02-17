#ifndef VERTICE_H_
#define VERTICE_H_

class Vertice{
	public:
		//construtor
		Vertice(int n) : numArestas(n){
			custoArestas = new float[numArestas];
		}
		
		//destrutor
		~Vertice(){
			delete [] custoArestas;
		}
		//quantidade de arestas que o vertice tem
		int numArestas;
		
		//custoArestas[i] representará o custo da aresta entre o vertice e seu sucessor i
		float* custoArestas;

		//demanda/oferta de commodity
		int pesoCommodity;
		
		//posicao (x,y) do vertice no plano euclideano
		int posX;
		int posY;
};

#endif /*VERTICE_H_*/
