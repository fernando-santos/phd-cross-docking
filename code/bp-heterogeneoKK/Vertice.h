#ifndef VERTICE_H_
#define VERTICE_H_

class Vertice{
	public:
		//destrutor
		~Vertice()
		{
			delete [] custoArestasDual;
			delete [] custoArestas;
		}

		//construtor
		Vertice(int n) : numArestas(n)
		{
			custoArestas = new float[numArestas];
			custoArestasDual = new float[numArestas]; 
		}

		//quantidade de arestas que o vertice tem
		int numArestas;
		
		//custoArestas[i] e custoArestasDual [i] representará o custo da aresta (e o seu valor dual) entre o vertice e seu sucessor i
		float* custoArestas;
		float* custoArestasDual;

		//demanda/oferta de commodity
		int pesoCommodity;
		
		//tempo de inicio para atendimento do fornecedor/consumidor
		int tInicio;
		
		//tempo final para atendimento do fornecedor/consumidor
		int tFim;
		
		//tempo necessario para atender este fornecedor/consumidor
		int tServico;
	
		//custo associado ao vertice, dado pelos custos duais na relaxação do master restrito
		float custoDual;
};

#endif /*VERTICE_H_*/
