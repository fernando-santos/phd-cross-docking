#ifndef BRANCHING_H_
#define BRANCHING_H_
#include "ModeloCplex.h"

typedef class NoArvore* ptrNo;
class NoArvore{
	private:
		static int maxV;
		static int numCommod;
		static int totalNosCriados;
		static int tempoCrossDocking;
		static int numPontosExtremos;
		static int totalNosArmazenados;
		static vector<short int*>* matrizA_raiz;
		static vector<short int*>* matrizB_raiz;

		int indiceNo;
		float limiteDual;
		int k_branching, q_branching; //proxima variavel a ser realizado o branching
		int* qRFornNo; //quantidade de rotas por veiculo nos Fornecedores associadas ao no
		int* qRConsNo; //quantidade de rotas por veiculo nos Consumidores associadas ao no
		char** variaveisComRestricao; //variaveis que foram fixadas no primeiro branching deste no
		vector<short int*>* matrizA_no; //matriz com os vertices Fornecedores visitados em cada rota do no 
		vector<short int*>* matrizB_no; //matriz com os vertices Consumidores visitados em cada rota do no
		vector<Rota*>* ptrRForn_no; //ponteiros para as rotas do no (guarda a ordem de visita de cada no e o custo da rota)
		vector<Rota*>* ptrRCons_no; //idem, mas um ponteiro eh para rotas de Fornecedores, outro rotas de Consumidores
		ptrNo prox;

	public:
		~NoArvore();
		NoArvore(ModeloCplex&, int, int);
		NoArvore(ptrNo, ModeloCplex&, int);
		void procuraVariavelBranching(ModeloCplex&);
		ptrNo geraFilho(ModeloCplex&, Grafo*, int, char, bool&, bool&);
		void setRestricoesArvore(ModeloCplex&);
		void setRestricoesArvoreTau(ModeloCplex&);
		void setRestricoesArvoreLambdaGamma(ModeloCplex&);
		bool alcancaRaizNo(ModeloCplex&, Grafo*, char);
		void atualizaCustosDuaisForn(ModeloCplex&, Grafo*, int);
		float atualizaCustosDuaisPontosInterioresForn(ModeloCplex&, Grafo*, int);
		void atualizaCustosDuaisCons(ModeloCplex&, Grafo*, int);
		float atualizaCustosDuaisPontosInterioresCons(ModeloCplex&, Grafo*, int);
		void inserirColunaForn(ModeloCplex&, Rota*, int);
		void inserirColunaCons(ModeloCplex&, Rota*, int);
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
	a) q sera coletada e entregue pelo veículo k
		a1) tau^k_q = 0
		a2) \sum_{r} A_{qr} \lambda^k_r = 1
		a3) \sum_{r'} B_{qr'} \gamma^k_r' = 1
	
	b) q não será nem coletada nem entregue pelo veículo k
		b1) tau^k_q = 0
		b2) \sum_{r} A_{qr} \lambda^k_r = 0
		b3) \sum_{r'} B_{qr'} \gamma^k_r' = 0
		
	c) q sera coletada pelo veiculo k, mas NAO sera entregue por k
		c1) tau^k_q = 1
		c2) \sum_{r} A_{qr} \lambda^k_r = 1
		c3) \sum_{r'} B_{qr'} \gamma^k_r' = 0
	
	d) q NAO sera coletada pelo veiculo k, mas sera entregue por k
		d1) tau^k_q = 1
		d2) \sum_{r} A_{qr} \lambda^k_r = 0
		d3) \sum_{r'} B_{qr'} \gamma^k_r' = 1
*/
