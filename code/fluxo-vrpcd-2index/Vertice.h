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
		
		//tempo de inicio para atendimento do fornecedor/consumidor
		int tInicio;
		
		//tempo final para atendimento do fornecedor/consumidor
		int tFim;
		
		//tempo necessario para atender este fornecedor/consumidor
		int tServico;
	
		//custo associado ao vertice, dado pelos custos duais na relaxação do master restrito
		float custodual;
};

#endif /*VERTICE_H_*/
