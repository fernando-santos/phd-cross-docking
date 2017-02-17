#ifndef MODELOCPLEX_H_
#define MODELOCPLEX_H_

#include <ilcplex/ilocplex.h>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"
#include "Elementar.h"
#include "StateSpaceRelax.h"

Rota** retornaColuna(Grafo*, bool, float, char);

class ModeloCplex{
	friend class ModeloHeuristica;
	private:
		int qRotasForn;
		int qRotasCons;
		int maxVeic;
		int nCommodities;
		int custoTrocaCD;
		static float limitePrimal;

		vector<short int*> A_qr;
		vector<short int*> B_qr;
		vector<Rota*> ptrRotasForn;
		vector<Rota*> ptrRotasCons;
		vector<int> lambdaNaoNulo;
		vector<int> gammaNaoNulo;

		IloEnv env;
		IloModel modelPrimal;
		IloCplex cplexPrimal;

		IloNumVarArray lambdaPrimal;
		IloNumVarArray gammaPrimal;
		IloArray < IloNumVarArray > tauPrimal;
		IloNumVarArray tau;

		IloObjective costPrimal;
		IloRange constraintsPrimal1;
		IloRange constraintsPrimal2;
		IloRangeArray constraintsPrimal3;
		IloRangeArray constraintsPrimal4;
		IloRangeArray constraintsPrimal5;
		IloRangeArray constraintsPrimal6;
		IloRangeArray constraintsPrimal7;
		IloRange constraintPrimal8;

	public:
		ModeloCplex(Grafo*, Rota**, Rota**, int, int, int, int, int);
		~ModeloCplex();

		void setA_qr(Rota**);
		void setB_qr(Rota**);

		float solveMaster();
		void exportModel(const char*);

		void updateDualCostsForn(Grafo*);
		void updateDualCostsCons(Grafo*);

		float getAlfaDual();
		float getBetaDual();

		void insertColumnForn(Rota*);
		void insertColumnCons(Rota*);

		void initVarsPrimal();
		void setObjectiveFunctionPrimal();
		void setConstraintsPrimal1();
		void setConstraintsPrimal2();
		void setConstraintsPrimal3();
		void setConstraintsPrimal4();
		void setConstraintsPrimal5_6_7_8();

		static float getLimitePrimal();
		static void setLimitePrimal(float);
};

#endif
