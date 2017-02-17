#ifndef NOARVOREARCOS_H_
#define NOARVOREARCOS_H_
#include "ModeloCplex.h"

typedef class NoArvoreArcos* ptrNoArcos;
class NoArvoreArcos{
	private:
		static int maxV;
		static int numCommod;
		static int totalNosCriados;
		static ModeloCplex* mCplex;
		static char** variaveisComRestricao;
		
		int indiceNo;
		float limiteDual;
		int* qRFornNo;
		int* qRConsNo;
		int iFixar, jFixar, kFixar; //proximos i, j e k a serem fixados no branching
		vector<short int*>* arcosBranching; //short int*: ponteiro para um vetor de 3 posicoes: (i, j, {0,1})
		vector<short int*>* matrizA_no;
		vector<short int*>* matrizB_no;
		vector<Rota*>* ptrRForn_no;
		vector<Rota*>* ptrRCons_no;
		ptrNoArcos prox;

	public:
		~NoArvoreArcos();
		NoArvoreArcos();
		NoArvoreArcos(ptrNoArcos, int);
		static bool executaBranchingArcos(ModeloCplex*, Grafo*, char**, int, int, char, long int);
		bool alcancaRaizNoSegundoBranching(Grafo*, char);
		float procuraArcoMaisViolado();
		void setRestricoesArcos();
		void atualizaCustosDuaisForn(Grafo*, int);
		void atualizaCustosDuaisCons(Grafo*, int);
		void inserirColunaForn(Rota*, int);
		void inserirColunaCons(Rota*, int);
		static float getLimitePrimal();
		void imprimeNo();
};

#endif
