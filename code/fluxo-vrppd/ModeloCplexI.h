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
		IloArray <IloArray <IloNumVarArray> > f;
		IloArray <IloArray <IloIntVarArray> > x;
		IloArray <IloIntVarArray> y;

	public:
		ModeloCplexI(Grafo*, int, int, char*);
		~ModeloCplexI();

		//inicializa as variaveis a serem utilizadas no modelo
		void initVars();

		//define os coeficientes das variaveis na funcao objetivo
		void setObjectiveFunction(Grafo*);
		
		//atribui as restricoes (1) do modelo
		void setConstraints1();

		//atribui as restricoes (3) do modelo
		void setConstraints2(Grafo*);

		//atribui as restricoes (3) do modelo
		void setConstraints3();

		//atribui as restricoes (4) do modelo
		void setConstraints4();

		//atribui as restricoes (5) do modelo
		void setConstraints5(Grafo*);

		//atribui as restricoes (6) do modelo
		void setConstraints6(Grafo*);
		
		//atribui as restricoes (7) do modelo
		void setConstraints7(Grafo*);
};

#endif
