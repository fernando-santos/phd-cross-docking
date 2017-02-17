#ifndef MODELOCPLEXI_H_
#define MODELOCPLEXI_H_

#include <ilcplex/ilocplex.h>
#include "Grafo.h"

class ModeloCplexI{
	private:
		int maxVeic;
		int nCommodities;
		int nVertices;
		float* classesVeic;
	
		IloEnv env;
		IloModel model;
		IloCplex cplex;
		IloArray <IloArray <IloNumVarArray> > f;
		IloArray <IloArray <IloIntVarArray> > x;
		IloArray <IloIntVarArray> y;
		IloArray <IloIntVarArray> u;
		IloArray <IloNumVarArray> t;

	public:
		ModeloCplexI(Grafo*, int, int, int, int, char*, float*);
		~ModeloCplexI();

		//inicializa as variaveis a serem utilizadas no modelo
		void initVars();

		//define os coeficientes das variaveis na funcao objetivo
		void setObjectiveFunction(Grafo*, int);
		
		//atribui as restricoes (1) do modelo
		void setConstraints1();

		//atribui as restricoes (2) do modelo
		void setConstraints2();

		//atribui as restricoes (3) do modelo
		void setConstraints3(Grafo*);

		//atribui as restricoes (4) do modelo
		void setConstraints4();

		//atribui as restricoes (5) do modelo
		void setConstraints5(Grafo*);

		//atribui as restricoes (6) do modelo
		void setConstraints6(Grafo*);
		
		//atribui as restricoes (7) do modelo
		void setConstraints7(Grafo*);

		//atribui as restricoes (8) do modelo
		void setConstraints8(Grafo*);

		//atribui as restricoes (9) do modelo
		void setConstraints9();
		
		//atribui as restricoes (10) do modelo
		void setConstraints10();

		//atribui as restricoes de janela de tempo, caso tenha sido passado o parametro como 'true'
		void setConstraintsJanela(Grafo*, int);
		
		float optimize(int);
};

#endif
