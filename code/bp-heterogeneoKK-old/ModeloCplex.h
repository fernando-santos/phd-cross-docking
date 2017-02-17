#ifndef MODELOCPLEX_H_
#define MODELOCPLEX_H_

#include <ilcplex/ilocplex.h>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"
#include "Elementar.h"
#include "StateSpaceRelax.h"

Rota** retornaColuna(Grafo*, int, bool, bool, float, char, int&);

struct strHash{
	Rota* ptrRota;
	int* veicsRota;
	
	~strHash()
	{
		delete [] veicsRota;
		if ( ptrRota->decrNumApontadores() ) delete ptrRota;
	}
	
	void imprimir( int maxV )
	{
		printf("[ "); 
		for ( int k = 0; k < maxV; ++k ) printf("%d ", veicsRota[k]); 
		printf("] ");
		ptrRota->imprimir();
	}
};

class ModeloCplex{
	friend class ModeloHeuristica;

	private:
		int* qRotasForn;
		int* qRotasCons;
		int maxVeic;
		int nCommodities;
		int custoTrocaCD;
		vector<short int*>* A_qr;
		vector<short int*>* B_qr;
		vector<Rota*>* ptrRotasForn;
		vector<Rota*>* ptrRotasCons;
		vector<strHash*> vetorRotas;
		static float limitePrimal;
	
		IloEnv env;
		IloModel modelPrimal;
		IloCplex cplexPrimal;

		//Variaveis de decisao da raiz e arvore BP
		IloArray < IloNumVarArray > lambdaPrimal;
		IloArray < IloNumVarArray > lambdaNo;
		IloArray < IloNumVarArray > gammaPrimal;
		IloArray < IloNumVarArray > gammaNo;
		IloArray < IloArray < IloNumVarArray > > tauPrimal;
		IloNumVarArray aPrimal;

		//Funcao objetivo e Restricoes Primal
		IloObjective costPrimal;
		IloRangeArray constraintsPrimal1;
		IloRangeArray constraintsPrimal2;
		IloRangeArray constraintsPrimal3;
		IloRangeArray constraintsPrimal4;
		IloRangeArray constraintsPrimal5;
		IloArray < IloRangeArray > constraintsPrimal6;
		IloRangeArray constraintsPrimal7;

	public:
		ModeloCplex(Grafo*, Rota**, Rota**, int, int, int, int, int);
		~ModeloCplex();
		
		//Armazena a matriz A_{qr} no modelo de uma maneira um pouco mais eficiente
		void setA_qr(Rota**);
		//Armazena a matriz B_{qr} no modelo de uma maneira um pouco mais eficiente
		void setB_qr(Rota**);

		//soluciona a relaxacao linear do master
		float solveMaster();

		//exporta o modelo para o arquivo passado como parametro
		void exportModel(const char*);
		
		//FUNCOES RELACIONADAS AO PROBLEMA PRIMAL
		//inicializa as variaveis a serem utilizadas no modelo Primal
		void initVarsPrimal();
		//define os coeficientes das variaveis na funcao objetivo do modelo Primal
		void setObjectiveFunctionPrimal();
		//atribui as restricoes (1) do modelo Primal
		void setConstraintsPrimal1();
		//atribui as restricoes (2) do modelo Primal
		void setConstraintsPrimal2();
		//atribui as restricoes (3) do modelo Primal
		void setConstraintsPrimal3();
		//atribui as restricoes (4) do modelo Primal
		void setConstraintsPrimal4();
		//atribui as restricoes (5) do modelo Primal
		void setConstraintsPrimal5();
		//atribui as restricoes (6) e (7) do modelo Primal
		void setConstraintsPrimal6e7();

		//atualiza custos Duais no grafo
		void updateDualCostsForn(Grafo*, int);
		void updateDualCostsCons(Grafo*, int);
		
		//retorna o valor constante a ser subtraida, para saber se uma determinada rota tem ou nao custo reduzido negativo
		float getValorSubtrairForn(int);
		float getValorSubtrairCons(int);
		
		//insere uma coluna no primal relacionada a rota dos fornecedores/consumidores
		void insertColumnPrimalForn(Rota*, int);
		void insertColumnPrimalCons(Rota*, int);
		strHash* getStrHashRota(int, int&);

		static float getLimitePrimal();
		static void setLimitePrimal(float);
		void atualizaRotasBasicas();
		void imprimeRotasBasicas();
};

#endif
