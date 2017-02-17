#ifndef MODELOCPLEX_H_
#define MODELOCPLEX_H_

#include <ilcplex/ilocplex.h>
#include <string.h>
#include "Grafo.h"
#include "Rota.h"
#include "ElementarTW.h"


class ModeloCplex{
	friend class NoArvore;
	friend class NoArvoreArcos;
	friend class ModeloHeuristica;
	private:
		int* qRotasForn;
		int* qRotasCons;
		int maxVeic;
		int nCommodities;
		int custoTrocaCD;
		int tamPontoInterior;
		IloNum* pontoInterior;
		vector<short int*>* A_qr;
		vector<short int*>* B_qr;
		vector<Rota*>* ptrRotasForn;
		vector<Rota*>* ptrRotasCons;
		static float limitePrimal;
	
		IloEnv env;
		IloModel modelPrimal, modelDual;
		IloCplex cplexPrimal, cplexDual;

		IloArray<IloNumVarArray> lambdaPrimal;	//lambda eh a variavel das rotas dos fornecedores
		IloArray<IloNumVarArray> lambdaNo;
		IloArray<IloNumVarArray> gammaPrimal;	//gamma eh a variavel das rotas dos consumidores
		IloArray<IloNumVarArray> gammaNo;
		IloArray<IloNumVarArray> tauPrimal;		//tau esta relacionada à carga/descarga de mercadorias no Cross-Docking
		IloNumVarArray artificiais;				//variaveis artificiais associadas as restricoes da arvore

		//Funcao objetivo Primal
		IloObjective costPrimal;
		//Restricoes do problema dual
		IloRangeArray constraintsPrimal1;
		IloRangeArray constraintsPrimal2;
		IloRangeArray constraintsPrimal3;
		IloRangeArray constraintsPrimal4;
		IloRangeArray constraintsPrimal5;
		IloRangeArray constraintsPrimal6;
		IloRange constraintArvoreTau0;
		IloRange constraintArvoreTau1;
		IloRangeArray constraintsArvoreLambda;
		IloRangeArray constraintsArvoreGamma;
		IloRangeArray constraintsArvoreArcos;

		//Variaveis de decisao utilizadas para montar o problema dual
		IloNumVarArray alfaDual;
		IloNumVarArray betaDual;
		IloNumVarArray thetaDual;
		IloNumVarArray muDual;
		IloArray<IloNumVarArray> piDual;
		IloArray<IloNumVarArray> xiDual;
		IloArray<IloNumVarArray> varDualArvoreLambda;
		IloArray<IloNumVarArray> varDualArvoreGamma;
		IloNumVar varDualArvoreTau0;
		IloNumVar varDualArvoreTau1;

		//Funcao objetivo Dual
		IloObjective costDual;
		//Restricoes do problema dual
		IloArray<IloRangeArray> constraintsDual1;
		IloArray<IloRangeArray> constraintsDual1_no;
		IloArray<IloRangeArray> constraintsDual2;
		IloArray<IloRangeArray> constraintsDual2_no;
		IloRangeArray constraintsDual3;
		IloRangeArray constraintsDualArtificiais;

	public:
		ModeloCplex(Grafo*, Rota**, Rota**, int, int, int, int, int);
		ModeloCplex(ModeloCplex&, vector<short int*>*, vector<short int*>*, vector<Rota*>*, vector<Rota*>*, char**);
		~ModeloCplex();
		
		//Armazena a matriz A_{qr} no modelo de uma maneira um pouco mais eficiente
		void setA_qr(Rota**);
		//Armazena a matriz B_{qr} no modelo de uma maneira um pouco mais eficiente
		void setB_qr(Rota**);

		//soluciona a relaxacao linear do master
		float solveMaster();
		
		//atualiza custos Duais no grafo
		void updateDualCostsForn(Grafo*, int);
		void updateDualCostsCons(Grafo*, int);
		
		//retorna o valor constante a ser subtraido da rota dos fornecedores, para saber se ela deve ou nao entrar no modelo
		float getAlfaDual(int);
		
		//retorna o valor constante a ser subtraido da rota dos consumidores, para saber se ela deve ou nao entrar no modelo
		float getBetaDual(int);
		
		//insere uma coluna no primal e a restricao equivalente no dual relacionada a rota dos fornecedores
		void insert_ColumnPrimal_RowDual_Forn(Rota*, int);
		
		//insere uma coluna no primal e a restricao equivalente no dual relacionada a rota dos consumidores
		void insert_ColumnPrimal_RowDual_Cons(Rota*, int);
		
		//exporta o modelo para o arquivo passado como parametro
		void exportModel(const char*, char);

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

		//FUNCOES RELACIONADAS AO PROBLEMA DUAL
		//inicializa as variaveis a serem utilizadas no modelo Dual
		void initVarsDual();
		//define os coeficientes das variaveis na funcao objetivo do modelo Dual
		void setObjectiveFunctionDual(char**);
		//atribui as restricoes (1) do modelo Dual
		void setConstraintsDual1();
		//atribui as restricoes (2) do modelo Dual
		void setConstraintsDual2();
		//atribui as restricoes (3) do modelo Dual
		void setConstraintsDual3();
		//como as restricoes estao montadas, deve-se ajustar os sinais das restricoes e inseri-las no modelo
		void geraModeloDual(char**);
		//gera diferentes multiplicadores para a funcao objetivo dual, obtem 20 (seguindo sugestao do paper)
		//pontos extremos e os combina para gerar um ponto interior da envoltoria convexa dual otima
		void geraPontoInterior(int);
		IloNum* getPontoExtremo(char**, int acresc = 0);
		void imprimeVariaveisDuais();
		void limpaVariaveisArvoreB1();
		void limpaVariaveisArvoreB2();
		void atualizaRotasBasicas();
		static float getLimitePrimal();
		static void setLimitePrimal(float);
};

#endif
