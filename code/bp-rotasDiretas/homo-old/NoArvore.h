#ifndef NOARVOREARCOS_H_
#define NOARVOREARCOS_H_
#include "ModeloCplex.h"
#include <algorithm>

typedef class NoArvore* ptrNo;
class NoArvore{
	friend class ModeloHeuristica;

	private:
		static Grafo* G;
		static ModeloCplex* mCplex;
		static int numRequests, numVertices;
		static int totalNosCriados, totalNosAtivos;

		int indiceNo;
		float limiteDual;
		int qRotas_no;
		int qRotasD_no;
		vector<short int*> a_ir_no;
		vector<short int*> d_ir_no;
		vector<Rota*> ptrRotas_no;
		vector<Rota*> ptrRotasD_no;
		vector<short int*> arcosBranching; //short int*: ponteiro para um vetor de 3 posicoes: (i, j, {0,1}) -> x_{ij} = 0 ou x_{ij} = 1
		ptrNo prox;

	public:
		~NoArvore();
		NoArvore(ModeloCplex*, Grafo*, vector<ptrNo>&, float);
		NoArvore(ptrNo, short int*);

		void setRestricoes_no();
		void setVariaveisDecisao_no();
		float executaBranching(vector<ptrNo>&, int*, int*, int, int, char, char, int);
		bool alcancaRaiz_no(int*, int*, int, int, char, char, int);
		bool defineVariavelBranching(int&, int&);
		void atualizaCustosDuaisRD(int*, int);
		void atualizaCustosDuais(int*, int);
		void inserirColunaDireta_no(Rota*);
		void inserirColuna_no(Rota*);
		void imprimir();

		int getIndiceNo();
		void setProx(ptrNo);
		ptrNo getProx();
		float getLimiteDual();
		void setMelhoresRotas(ModeloCplex*);
		static int getTotalNosCriados();
		static int getTotalNosAtivos();
};

#endif
