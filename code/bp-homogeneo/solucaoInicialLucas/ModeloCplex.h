#ifndef MODELOCPLEX_H_
#define MODELOCPLEX_H_

#include <ilcplex/ilocplex.h>
#include <sys/time.h>
#include "Grafo.h"
#include "Rota.h"
#include "GRASP.h"

class ModeloCplex{
	friend class NoArvore;
	friend class ModeloHeuristica;

	private:
		int qRotas;
		int maxVeic;
		int nRequests;
		int custoTrocaCD;
		int nPontosInteriores;
		float* xiPtInterior;
		float alfaPtInterior;
		vector<Rota*> ptrRotas;
		vector<short int*> a_ir;
		static float limitePrimal;
	
		IloEnv env;
		IloModel modelPrimal, modelDual;
		IloCplex cplexPrimal, cplexDual;

		//PRIMAL
		IloNumVarArray lambda;
		IloNumVarArray tau;
		IloObjective costPrimal;
		IloRange constraintP1;
		IloRangeArray constraintsP2;
		IloRangeArray constraintsP3;
		//variaveis e restricoes associadas ao branching
		IloNumVarArray lambda_no;
		IloNumVarArray artificiais;
		IloRangeArray constraintsArvore;

		//DUAL
		IloNumVar alfa;
		IloNumVarArray theta;
		IloNumVarArray xi;
		IloObjective costDual;
		IloRangeArray constraintsD1;
		IloRangeArray constraintsD2;

	public:
		ModeloCplex(Grafo*, Rota**, int, int, int);
		~ModeloCplex();
		
		//Armazena a matriz a_{ir} no modelo
		void preencheMatriz_a_ir(Rota**);

		//soluciona a relaxacao linear do master
		float solveMaster();

		//atualiza custos Duais no grafo
		void updateDualCosts(Grafo*);

		//insere uma coluna no primal e a restricao equivalente no dual relacionada a rota dos fornecedores
		void insert_ColumnPrimal_RowDual(Rota*);

		//retorna os valores das variaveis duais a serem usadas no pricing
		float getAlfa();
		float* getXi();

		//exporta o modelo para o arquivo passado como parametro
		void exportModel(const char*, char);

		//constroi os modelos primal e dual
		void constroiModeloPrimal();
		void constroiModeloDual();
		void geraDualPontosInteriores();
		void setObjectiveDualPontosInteriores();

		//verifica se as variaveis de decisao do modelo sao inteiras
		bool currentSolInteira(bool);

		//funcoes para manipular o valor de limite primal
		static float getLimitePrimal();
		static void setLimitePrimal(float);
		void imprimeRotasBasicas();
};

#endif
