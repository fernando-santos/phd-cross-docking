#ifndef BRANCHING_H_
#define BRANCHING_H_
#include "ModeloCplex.h"

typedef class NoArvore* ptrNo;
class NoArvore{
	private:
		static int maxV;
		static int numCommod;
		static int totalNosCriados;
		static int totalNosArmazenados;
		static vector<short int*>* matrizA_raiz;
		static vector<short int*>* matrizB_raiz;
		static vector<short int*>* matrizD_raiz;

		int indiceNo;
		float limiteDual;
		int k_branching, q_branching; //proxima variavel a ser realizado o branching
		int* qRFornNo; //quantidade de rotas por veiculo nos Fornecedores associadas ao no
		int* qRConsNo; //quantidade de rotas por veiculo nos Consumidores associadas ao no
		int* qRDirectNo; //quantidade de rotas Diretas por veiculo associadas ao no
		char** variaveisComRestricao; //variaveis que foram fixadas no primeiro branching deste no
		vector<short int*>* matrizA_no; //matriz com os vertices Fornecedores visitados em cada rota do no 
		vector<short int*>* matrizB_no; //matriz com os vertices Consumidores visitados em cada rota do no
		vector<short int*>* matrizD_no; //matriz com os vertices visitados em cada rota Direta do no
		vector<Rota*>* ptrRForn_no; //ponteiros para as rotas do no (guarda a ordem de visita de cada no e o custo da rota)
		vector<Rota*>* ptrRCons_no; //idem, mas um ponteiro eh para rotas de Fornecedores, outro rotas de Consumidores
		vector<Rota*>* ptrRDirect_no; //ponteiros para as rotas diretas do no
		ptrNo prox;

	public:
		~NoArvore();
		NoArvore(ModeloCplex&);
		NoArvore(ptrNo, ModeloCplex&, int);
		void procuraVariavelBranching(ModeloCplex&);
		ptrNo geraFilho(ModeloCplex&, Grafo*, int, char, char, int, bool&);
		
		void setRestricoesArvore(ModeloCplex&);
		void setRestricoesArvoreTau(ModeloCplex&);
		void setRestricoesArvoreLambdaGammaDelta(ModeloCplex&);

		bool alcancaRaizNo(ModeloCplex&, Grafo*, char, char, int);
		void atualizaCustosDuaisForn(ModeloCplex&, Grafo*, int);
		void atualizaCustosDuaisCons(ModeloCplex&, Grafo*, int);
		void atualizaCustosDuaisDirect(ModeloCplex&, Grafo*, int);
		void inserirColunaForn(ModeloCplex&, Rota*, int);
		void inserirColunaCons(ModeloCplex&, Rota*, int);
		void inserirColunaDirect(ModeloCplex&, Rota*, int);

		void exportaModeloNo(ModeloCplex&, const char*);
		bool verificaFilhoViavel(Grafo*, int);
		void imprimeNo();

		int getIndiceNo();
		ptrNo getProx();
		void setProx(ptrNo);
		int getKBranching();
		int getQBranching();
		vector<short int*>* getMatrizA_no();
		vector<short int*>* getMatrizB_no();
		vector<Rota*>* getPtrRForn_no();
		vector<Rota*>* getPtrRCons_no();
		char** getVariaveisComRestricao();
		float getLimiteDual();
		int getTotalNosCriados();
		static int getTotalNosArmazenados();
};

#endif

/*REGRA DE BRANCHING:
Vamos pegar uma variavel de rota - lambda ou gamma - que seja fracionaria (lembrando que as variaveis tau^k_q serao 
naturalmente zeradas na solucao da relaxacao linear, para que zerar o custo de troca na F.O). A rota fracionaria sera
composta por vertices (mercadorias). A regra de branching seleciona uma mercadoria 'q' desta rota (que está associada a um
veículo k) fracionaria que ainda nao tenha sido fixada, e cria 4 filhos com as seguintes restricoes de branching:
	1) i sera coletada pelo veiculo k, mas NAO sera entregue por k
		1a) tau^k_i = 1
		1b) \sum_{r} A_{ir} \lambda^k_r = 1
		1c) \sum_{r'} B_{ir'} \gamma^k_r' = 0
		1d) \sum_{r} D_{ir} \delta^k_r = 0
	
	2) i NAO sera coletada pelo veiculo k, mas sera entregue por k
		2a) tau^k_i = 1
		2b) \sum_{r} A_{ir} \lambda^k_r = 0
		2c) \sum_{r'} B_{ir'} \gamma^k_r' = 1
		2d) \sum_{r} D_{ir} \delta^k_r = 0

	3) i sera coletada e entregue pelo veículo k
		3a) tau^k_i = 0
		3b) \sum_{r} A_{ir} \lambda^k_r = 1
		3c) \sum_{r'} B_{ir'} \gamma^k_r' = 1
		3d) \sum_{r} D_{ir} \delta^k_r = 0
	
	4) i não será nem coletada nem entregue pelo veículo k
		4a) tau^k_i = 0
		4b) \sum_{r} A_{ir} \lambda^k_r = 0
		4c) \sum_{r'} B_{ir'} \gamma^k_r' = 0
		4d) \sum_{r} D_{ir} \delta^k_r = 0
	
	5) i sera coletata e entregue por uma rota direta
		5a) tau^k_i = 0
		5b) \sum_{r} A_{ir} \lambda^k_r = 0
		5c) \sum_{r'} B_{ir'} \gamma^k_r' = 0
		5d) \sum_{r} D_{ir} \delta^k_r = 0
*/
