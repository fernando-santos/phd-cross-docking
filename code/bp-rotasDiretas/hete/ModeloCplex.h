#ifndef MODELOCPLEX_H_
#define MODELOCPLEX_H_

#include <ilcplex/ilocplex.h>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"
#include "Elementar.h"
#include "StateSpaceRelax.h"
#include "ModeloBC-RD.h"
#include "ElementarRD.h"
#include "ElementarRDH.h"

Rota** retornaColuna(Grafo*, int, float, char, bool, bool, int&);
Rota** retornaColuna(Grafo*, int, float, char, int);

class ModeloCplex{
	friend class NoArvore;
	friend class ModeloHeuristica;

	private:
		int* qRotasForn;
		int* qRotasCons;
		int* qRotasDiretas;
		int maxVeic;
		int nCommodities;
		int custoTrocaCD;
		vector<short int*>* A_qr;
		vector<short int*>* B_qr;
		vector<short int*>* D_qr;
		vector<Rota*>* ptrRotasForn;
		vector<Rota*>* ptrRotasCons;
		vector<Rota*>* ptrRotasDiretas;
		static float limitePrimal;

		IloEnv env;
		IloModel modelPrimal;
		IloCplex cplexPrimal;

		IloArray<IloNumVarArray> lambdaPrimal;
		IloArray<IloNumVarArray> gammaPrimal;
		IloArray<IloNumVarArray> deltaPrimal;
		IloArray<IloNumVarArray> tauPrimal;

		//variaveis associadas ao branching
		IloNumVarArray artificiais;
		IloArray<IloNumVarArray> lambdaNo;
		IloArray<IloNumVarArray> gammaNo;
		IloArray<IloNumVarArray> deltaNo;

		//Funcao objetivo e restricoes
		IloObjective costPrimal;
		IloRangeArray constraintsPrimal1;
		IloRangeArray constraintsPrimal2;
		IloRangeArray constraintsPrimal3;
		IloRangeArray constraintsPrimal4;
		IloRangeArray constraintsPrimal5;
		IloRangeArray constraintsPrimal6;
		
		//restricoes associadas ao branching
		IloRange constraintArvoreTau0;
		IloRange constraintArvoreTau1;
		IloRangeArray constraintsArvoreLambda;
		IloRangeArray constraintsArvoreGamma;
		IloRangeArray constraintsArvoreDelta;

	public:
		~ModeloCplex();
		ModeloCplex(Grafo*, Rota**, Rota**, int, int, int, int, int);
		ModeloCplex(ModeloCplex&, vector<short int*>*, vector<short int*>*, vector<short int*>*, vector<Rota*>*, vector<Rota*>*, vector<Rota*>*, char**);
		
		//Armazena a matriz A_{qr} no modelo de uma maneira um pouco mais eficiente
		void setA_qr(Rota**);
		//Armazena a matriz B_{qr} no modelo de uma maneira um pouco mais eficiente
		void setB_qr(Rota**);

		//soluciona a relaxacao linear do master
		float solveMaster();
		
		//atualiza custos Duais no grafo
		void updateDualCostsForn(Grafo*, int);
		void updateDualCostsCons(Grafo*, int);
		void updateDualCostsDirect(Grafo*, int);
		
		//retorna o valor constante a ser subtraido da rota dos fornecedores, para saber se ela deve ou nao entrar no modelo
		float getAlfaDual(int);
		
		//retorna o valor constante a ser subtraido da rota dos consumidores, para saber se ela deve ou nao entrar no modelo
		float getBetaDual(int);
		
		//insere uma coluna no primal e a restricao equivalente no dual relacionada a rota
		void insert_ColumnPrimal_Forn(Rota*, int);
		void insert_ColumnPrimal_Cons(Rota*, int);
		void insert_ColumnPrimal_Direct(Rota*, int);
		
		//exporta o modelo para o arquivo passado como parametro
		void exportModel(const char*);

		//FUNCOES RELACIONADAS AO PROBLEMA DUAL
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
		//atribui as restricoes (6) do modelo Primal
		void setConstraintsPrimal6();

		void limpaVariaveisArvoreB1();
		static float getLimitePrimal();
		static void setLimitePrimal(float);
		void atualizaRotasBasicas();
		void imprimeRotasBasicas();
};

#endif
