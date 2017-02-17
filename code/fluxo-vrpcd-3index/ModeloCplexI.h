#ifndef MODELOCPLEXI_H_
#define MODELOCPLEXI_H_

#include <ilcplex/ilocplex.h>
#include "Grafo.h"

class ModeloCplexI{
	private:
		int maxVeic;
		int nCommodities;
		int nVertices;
	
		IloEnv env;
		IloModel model;
		IloCplex cplex;
		IloArray <IloArray <IloArray <IloNumVarArray> > > f;
		IloArray <IloArray <IloIntVarArray> > x;
		IloArray <IloIntVarArray> y;
		IloArray <IloIntVarArray> u;

	public:
		ModeloCplexI(Grafo*, int, int, int, char*);
		~ModeloCplexI();

		//inicializa as variaveis a serem utilizadas no modelo
		void initVars();

		//define os coeficientes das variaveis na funcao objetivo
		void setObjectiveFunction(Grafo*, int);
		
		//atribui as restricoes (1') do modelo
		void setConstraints1();

		//atribui as restricoes (2) do modelo
		void setConstraints2();
		
		//atribui as restricoes (3) do modelo
		void setConstraints3(Grafo*);

		//atribui as restricoes (4) do modelo
		void setConstraints4();

		//atribui as restricoes (5) do modelo
		void setConstraints5();

		//atribui as restricoes (5) do modelo
		void setConstraints6();

		//atribui as restricoes (6) do modelo
		void setConstraints7(Grafo*);

		//atribui as restricoes (8) do modelo
		void setConstraints8(Grafo*);

		//atribui as restricoes (9) do modelo
		void setConstraints9(Grafo*);
		
		//atribui as restricoes (10) do modelo
		void setConstraints10(Grafo*);

		//atribui as restricoes (11) do modelo
		void setConstraints11();

		//atribui as restricoes (12) do modelo
		void setConstraints12();
};

#endif
