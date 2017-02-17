#include "ModeloCplexI.h"
using namespace std;

ModeloCplexI::~ModeloCplexI(){
	cplex.end();
	model.end();
	env.end();
}

ModeloCplexI::ModeloCplexI(Grafo* g, int numCommodities, int maxVeiculos, char* arqSaida) : 
env(), model(env), cplex(model), maxVeic(maxVeiculos), nCommodities(numCommodities), nVertices(2*(numCommodities+1)){
	
	//instancia as variaveis de decisao do modelo																						
	initVars();
	
	//Define a funcao objetivo
	setObjectiveFunction(g);

	//Insere as restricoes no modelo
	setConstraints1();
	setConstraints2(g);
	setConstraints3();
	setConstraints4();
	setConstraints5(g);
	setConstraints6(g);
	setConstraints7(g);

	cplex.exportModel(arqSaida);
}

void ModeloCplexI::initVars(){

	f = IloArray <IloArray <IloNumVarArray> > (env, maxVeic);
	for(int q = 0; q < maxVeic; ++q){
		f[q] = IloArray<IloNumVarArray>(env, nVertices); 
		for(int i = 0; i < nVertices; ++i){
			f[q][i] = IloNumVarArray(env, nVertices, 0, INT_MAX, ILOFLOAT);
			for (int j = 0; j < nVertices; ++j){
				char nomeVar[20];
				sprintf(nomeVar, "f%d_%02d_%02d", q, i, j);
				f[q][i][j].setName(nomeVar);
			}
		}
	}

	x = IloArray <IloArray <IloIntVarArray> > (env, maxVeic);
	for(int q = 0; q < maxVeic; ++q){
		x[q] = IloArray<IloIntVarArray>(env, nVertices); 
		for(int i = 0; i < nVertices; ++i){
			x[q][i] = IloIntVarArray(env, nVertices, 0, 1);
			for (int j = 0; j < nVertices; ++j){
				char nomeVar[20];
				sprintf(nomeVar, "x%d_%02d_%02d", q, i, j);
				x[q][i][j].setName(nomeVar);
			}
		}
	}

	y = IloArray<IloIntVarArray>(env, maxVeic);
	for(int q = 0; q < maxVeic; ++q){
		y[q] = IloIntVarArray(env, nVertices, 0, 1);
		for (int i = 0; i < nVertices; ++i){
			char nomeVar[20];
			sprintf(nomeVar, "y%d_%02d", q, i);
			y[q][i].setName(nomeVar);
		}
	}
}


void ModeloCplexI::setObjectiveFunction(Grafo *g){
	IloExpr obj(env);
	float custo;

	//custo nos arcos
	for (int q = 0; q < maxVeic; ++q){ 
		for (int i = 0; i < nVertices; ++i){
			for (int j = 0; j < nVertices; ++j){
				custo = g->getCustoAresta(i,j);
				
				if (custo != MAIS_INFINITO){
					obj += custo * x[q][i][j];
				}
				
			}
		}
	}
		
	model.add(IloMinimize(env, obj));
	obj.end();
}



void ModeloCplexI::setConstraints1(){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int q = 0; q < maxVeic; ++q){
		IloExpr expr = IloExpr(env);

		for (int j = 1; j <= nCommodities; ++j){
			expr += f[q][0][j];
		}

		for (int i = 1; i < nVertices-1; ++i){
			expr -= y[q][i];
		}

		//Insere a restrição no modelo
		constr[q] = (expr == 1);
		char nome[20];
		sprintf(nome, "r1_q%d", q);
		constr[q].setName(nome);
		model.add(constr[q]);
		expr.end();	
	}
	constr.end();
}


void ModeloCplexI::setConstraints2(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nVertices);
	int c = 0;

	for (int q = 0; q < maxVeic; ++q){
		for (int i = 1; i < nVertices-1; ++i){
			IloExpr expr = IloExpr(env);
			
			for (int j = 0; j < nVertices; j++){
				if (g->getCustoAresta(i,j) != MAIS_INFINITO){
					expr += f[q][i][j];
				}
			}
			
			for (int j = 0; j < nVertices; j++){
				if (g->getCustoAresta(j,i) != MAIS_INFINITO){
					expr -= f[q][j][i];
				}
			}

			expr += y[q][i];
			constr[c] = (expr == 0);
			//Insere a restrição no modelo
			char nome[20];
			sprintf(nome, "r2_q%di%02d", q, i);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexI::setConstraints3(){
	IloRangeArray constr = IloRangeArray(env, nVertices);

	for (int i = 1; i < nVertices-1; ++i){
		IloExpr expr = IloExpr(env);

		for (int q = 0; q < maxVeic; ++q){
			expr += y[q][i];
		}
		
		constr[i] = (expr == 1);
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r3_i%d", i);
		constr[i].setName(nome);
		model.add(constr[i]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexI::setConstraints4(){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nCommodities);
	int c = 0;

	for (int q = 0; q < maxVeic; ++q){
		for (int i = 1; i <= nCommodities; ++i){
			IloExpr expr = IloExpr(env);
			expr += y[q][i] - y[q][i+nCommodities];
			constr[c] = (expr == 0);
			//Insere a restrição no modelo
			char nome[20];
			sprintf(nome, "r4_q%di%02d", q, i);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexI::setConstraints5(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nVertices*nVertices);
	int c = 0;

	for (int q = 0; q < maxVeic; ++q){
		for (int i = 0; i < nVertices; ++i){
			for (int j = 0; j < nVertices; ++j){
				IloExpr expr = IloExpr(env);
				
				if (g->getCustoAresta(i,j) != MAIS_INFINITO){
					expr = (f[q][i][j] - nVertices * x[q][i][j]);
					constr[c] = (expr <= 0);
					//Insere a restrição no modelo
					char nome[20];
					sprintf(nome, "r5_q%di%02dj%02d", q, i, j);
					constr[c].setName(nome);
					model.add(constr[c]);
					++c;
				}
				expr.end();
			}
		}
	}
	constr.end();
}


void ModeloCplexI::setConstraints6(Grafo *g){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nVertices);
	int c = 0;

	for (int q = 0; q < maxVeic; ++q){
		for (int i = 1; i < nVertices-1; ++i){
			IloExpr expr = IloExpr(env);

			for (int j = 0; j < nVertices; ++j){
				if (g->getCustoAresta(i,j) != MAIS_INFINITO){
					expr += x[q][i][j]; //aresta que SAI de i
				}
			}
			for (int j = 0; j < nVertices; ++j){
				if (g->getCustoAresta(j,i) != MAIS_INFINITO){
					expr += x[q][j][i]; //aresta que ENTRA em i
				}
			}
			expr -= 2 * y[q][i];
			constr[c] = (expr == 0);
			//Insere a restrição no modelo
			char nome[20];
			sprintf(nome, "r6_q%di%02d", q, i);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexI::setConstraints7(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int q = 0; q < maxVeic; ++q){
		IloExpr expr = IloExpr(env);

		for (int i = 1; i <= nCommodities; ++i){
			expr += g->getPesoCommodity(i) * y[q][i];
		}
		
		constr[q] = (expr <= g->getCapacVeiculo());
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r7_q%d", q);
		constr[q].setName(nome);
		model.add(constr[q]);
		expr.end();
	}
	constr.end();
}

