#ifndef MODELOCPLEXR_H_
#define MODELOCPLEXR_H_

#include <ilcplex/ilocplex.h>
#include "Grafo.h"
#include <stdlib.h>

class ModeloCplexR{
	private:
		int maxVeic;
		int nCommodities;
		int nVertices;
	
		IloEnv env;
		IloModel model;
		IloCplex cplex;
		IloArray <IloArray <IloArray <IloNumVarArray> > > f;
		IloArray <IloArray <IloNumVarArray> > x;
		IloArray <IloNumVarArray> y;
		IloArray <IloNumVarArray> u;

	public:
		ModeloCplexR(Grafo*, int, int, int, char*);
		~ModeloCplexR();

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
