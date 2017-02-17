#ifndef VERTICE_H_
#define VERTICE_H_

const float MAIS_INFINITO=999999;

class Vertice{
	public:
		//construtor
		Vertice(int n) : numArestas(n){
			custoArestas = new float[numArestas];
			custoArestasDual = new float[numArestas];
		}
		
		//destrutor
		~Vertice(){
			delete [] custoArestas;
			delete [] custoArestasDual;
		}
		//quantidade de arestas que o vertice tem
		int numArestas;
		
		//custoArestas[i] representará o custo da aresta entre o vertice e seu sucessor i
		float* custoArestas;
		
		//custoArestasVertice[i] eh a soma do custo da aresta armazenado em custoArestas[i] + custoDual de i
		//estes custos sao uteis para o calculo dos caminhos elementares e nao elementares, pois evitam 
		//calcular a todo passo o custo de se atravessar o arco, considerando o custo dual do vertice sucessor
		float* custoArestasDual;

		//demanda/oferta de commodity
		int carga;
		
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
