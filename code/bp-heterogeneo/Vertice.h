#ifndef VERTICE_H_
#define VERTICE_H_

class Vertice{
	public:
		//destrutor
		~Vertice(){
			for ( int cl = 0; cl < numVeic; ++cl ) delete custoArestas[cl];
			delete [] custoArestasDual;
			delete [] custoArestas;
		}

		//construtor
		Vertice(int n, int nVeic) : numArestas(n), numVeic(nVeic)
		{
			custoArestas = new float*[numVeic];
			for ( int cl = 0; cl < numVeic; ++cl ) custoArestas[cl] = new float[numArestas];

			//custoArestasDual nao precisa de classes, pois seu valor sera atualizado em funcao das classes de custoArestas[cl]
			custoArestasDual = new float[numArestas]; 
		}

		//quantidade de arestas que o vertice tem
		int numArestas;
		
		//quantidade de veiculos da frota heterogenea
		int numVeic;
		
		//custoArestas[c][i] representará o custo da aresta entre o vertice e seu sucessor i para a classe c
		float** custoArestas;
		
		//custoArestasVertice[i] eh a soma do custo da aresta armazenado em custoArestas[c][i] + custoDual de i
		//estes custos sao uteis para o calculo dos caminhos elementares e nao elementares, pois evitam 
		//calcular a todo passo o custo de se atravessar o arco, considerando o custo dual do vertice sucessor
		float* custoArestasDual;
		
		//Armazena em quantas unidades este vertice sera deslocado no grafo dual
		//Se o VALOR desta variavel for NEGATIVO, significa que o VERTICE esta INATIVO
		//Se o VALOR desta variavel for POSITIVO, significa que o VERTICE esta ATIVO 
		//e sera deslocado este VALOR no grafo dual a esquerda no vetor
		int deslocamentoGrafoDual;

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
