#ifndef ELEMENTARCPLEX_H_
#define ELEMENTARCPLEX_H_

#include <ilcplex/ilocplex.h>
#include "Grafo.h"
#include "Rota.h"

class ElementarCplex{
	private:
		static int nCommodities;
		static int ajuste;

		static IloEnv env;
		static IloModel model;
		static IloCplex cplex;
		static IloArray <IloNumVarArray> f;
		static IloArray <IloIntVarArray> x;
		static IloIntVarArray y;
		static IloNumVarArray t;
		static IloObjective fObjetivo;

	public:
		ElementarCplex();
		static void initModel(Grafo*);
		static void freeModel();

		//inicializa as variaveis a serem utilizadas no modelo
		static void initVars(Grafo*);

		//define os coeficientes das variaveis na funcao objetivo
		static void setObjectiveFunction(Grafo*);
		
		static void setConstraints1();
		static void setConstraints2();
		static void setConstraints3();
		static void setConstraints4();
		static void setConstraints5();
		static void setConstraints6();
		static void setConstraints7(Grafo*);
		static void setConstraintsJanela(Grafo*);

		static void calculaCaminhoElementar(Grafo*, bool, bool);
		static Rota** getRotaCustoMinimo(Grafo*, float);
};

#endif
