#include "ModeloCplexR.h"
using namespace std;

ModeloCplexR::~ModeloCplexR(){
	cplex.end();
	model.end();
	env.end();
}

ModeloCplexR::ModeloCplexR(Grafo* g, int numCommodities, int maxVeiculos, int custoTroca, char* arqSaida) : 
env(), model(env), cplex(model), maxVeic(maxVeiculos), nCommodities(numCommodities), nVertices(2*(numCommodities+1)){
	
	//instancia as variaveis de decisao do modelo																						
	initVars();
	
	//Define a funcao objetivo
	setObjectiveFunction(g, custoTroca);

	//Insere as restricoes no modelo
	setConstraints1();
	setConstraints2();
	setConstraints3(g);
	setConstraints4();
	setConstraints5();
	setConstraints6();
	setConstraints7(g);
	setConstraints8(g);
	setConstraints9(g);
	setConstraints10(g); 
	setConstraints11();
	setConstraints12();

	cplex.exportModel(arqSaida);
}


void ModeloCplexR::initVars(){

	f = IloArray <IloArray <IloArray <IloNumVarArray> > > (env, nVertices);
	for(int q = 0; q < nVertices; ++q){
		f[q] = IloArray <IloArray <IloNumVarArray> > (env, maxVeic);
		for(int k = 0; k < maxVeic; ++k){
			f[q][k] = IloArray<IloNumVarArray>(env, nVertices); 
			for(int i = 0; i < nVertices; ++i){
				f[q][k][i] = IloNumVarArray(env, nVertices, 0, 999999, ILOFLOAT);
				for (int j = 0; j < nVertices; ++j){
					char nomeVar[20];
					sprintf(nomeVar, "f%d_%02d_%02d_%02d", q, k, i, j);
					f[q][k][i][j].setName(nomeVar);
				}
			}
		}
	}

	x = IloArray <IloArray <IloNumVarArray> > (env, maxVeic);
	for (int k = 0; k < maxVeic; ++k){
		x[k] = IloArray <IloNumVarArray> (env, nVertices);
		for(int i = 0; i < nVertices; ++i){
			x[k][i] = IloNumVarArray(env, nVertices, 0, 1, ILOFLOAT);
			for (int j = 0; j < nVertices; ++j){
				char nomeVar[20];
				sprintf(nomeVar, "x%d_%02d_%02d", k, i, j);
				x[k][i][j].setName(nomeVar);
			}
		}
	}

	y = IloArray <IloNumVarArray> (env, nVertices);
	for (int q = 0; q < nVertices; ++q){
		y[q] = IloNumVarArray(env, maxVeic, 0, 1, ILOFLOAT);
		for(int k = 0; k < maxVeic; ++k){		
			char nomeVar[20];
			sprintf(nomeVar, "y_q%02d_k%02d", q, k);
			y[q][k].setName(nomeVar);
		}
	}

	u = IloArray<IloNumVarArray>(env, nCommodities+1);
	for (int q = 0; q <= nCommodities; ++q){
		u[q] = IloNumVarArray(env, maxVeic, 0, 1, ILOFLOAT);
		for(int k = 0; k < maxVeic; ++k){
			char nomeVar[20];
			sprintf(nomeVar, "u_q%02d_k%02d", q, k);
			u[q][k].setName(nomeVar);
		}
	}
}


void ModeloCplexR::setObjectiveFunction(Grafo *g, int custoTroca){
	IloExpr obj(env);
	float custo;

	//custo nos arcos
	for (int k = 0; k < maxVeic; ++k){
		for (int i = 0; i < nVertices; ++i){
			for (int j = 0; j < nVertices; ++j){
				custo = g->getCustoAresta(i,j);
				if (custo != MAIS_INFINITO){
					obj += custo * x[k][i][j];
				}
			}
		}
	}

	//custo de troca de mercadorias no cross-docking
	for (int q = 1; q <= nCommodities; ++q){
		for (int k = 0; k < maxVeic; ++k){
			obj -= custoTroca*u[q][k];
		}
	}
	obj += (nCommodities*custoTroca);

	model.add(IloMinimize(env, obj));
	obj.end();
}


void ModeloCplexR::setConstraints1(){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nVertices);
	int c = 0;

	for (int q = 1; q <= nCommodities; ++q){
		for (int k = 0; k < maxVeic; ++k){
			IloExpr expr = IloExpr(env);

			for (int j = 1; j <= nCommodities; ++j){
				expr += f[q][k][0][j];
			}
			expr -= 2 * y[q][k];

			//Insere a restrição no modelo
			constr[c] = (expr == 0);
			char nome[20];
			sprintf(nome, "r1_q%02d_k%02d", q, k);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexR::setConstraints2(){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nVertices);
	int c = 0;

	for (int q = nCommodities+1; q < nVertices-1; ++q){
		for (int k = 0; k < maxVeic; ++k){
			IloExpr expr = IloExpr(env);

			for (int j = nCommodities+1; j < nVertices-1; ++j){
				expr += f[q][k][0][j];
			}
			expr -= 2 * y[q][k];

			//Insere a restrição no modelo
			constr[c] = (expr == 0);
			char nome[20];
			sprintf(nome, "r2_q%02d_k%02d", q, k);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexR::setConstraints3(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nVertices*nVertices);
	int c = 0;

	for (int q = 1; q < nVertices-1; ++q){
		for (int k = 0; k < maxVeic; ++k){
			for (int i = 1; i < nVertices; ++i){
				IloExpr expr = IloExpr(env);
		
				for (int j = 0; j < nVertices; j++){
					if (g->getCustoAresta(i,j) != MAIS_INFINITO){
						expr += f[q][k][i][j];
					}
				}

				for (int j = 0; j < nVertices; j++){
					if (g->getCustoAresta(j,i) != MAIS_INFINITO){
						expr -= f[q][k][j][i];
					}
				}

				if ((i == q) || (i == (nVertices-1))){
					expr += y[q][k];
				}
				
				constr[c] = (expr == 0);
				//Insere a restrição no modelo
				char nome[20];
				sprintf(nome, "r3_q%02d_k%02d_i%02d", q, k, i);
				constr[c].setName(nome);
				model.add(constr[c]);
				expr.end();
				++c;
			}
		}
	}
	constr.end();
}


void ModeloCplexR::setConstraints4(){
	IloRangeArray constr = IloRangeArray(env, nVertices);
	int c = 0;

	for (int q = 1; q < nVertices-1; ++q){
		IloExpr expr = IloExpr(env);
		for (int k = 0; k < maxVeic; ++k){
			expr += y[q][k];
		}
		//Insere a restrição no modelo
		constr[c] = (expr == 1);
		char nome[20];
		sprintf(nome, "r4_q%02d", q);
		constr[c].setName(nome);
		model.add(constr[c]);
		expr.end();
		++c;
	}
	constr.end();
}


void ModeloCplexR::setConstraints5(){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k){
		IloExpr expr = IloExpr(env);
			
		for (int j = 1; j <= nCommodities; j++){
			expr += x[k][0][j];
		}		
		constr[k] = (expr == 1);
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r5_k%02d", k);
		constr[k].setName(nome);
		model.add(constr[k]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexR::setConstraints6(){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k){
		IloExpr expr = IloExpr(env);
			
		for (int j = nCommodities+1; j < nVertices-1; j++){
			expr += x[k][0][j];
		}		
		constr[k] = (expr == 1);
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r6_k%02d", k);
		constr[k].setName(nome);
		model.add(constr[k]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexR::setConstraints7(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nVertices*nVertices);
	int c = 0;

	for (int k = 0; k < maxVeic; ++k){
		for (int i = 0; i < nVertices; ++i){
			for (int j = 0; j < nVertices; ++j){
				IloExpr expr = IloExpr(env);

				if (g->getCustoAresta(i,j) != MAIS_INFINITO){
					for (int q = 1; q < nVertices-1; ++q){
						expr += f[q][k][i][j];
					}
					expr -= nVertices * x[k][i][j];

					constr[c] = (expr <= 0);
					//Insere a restrição no modelo
					char nome[20];
					sprintf(nome, "r7_k%d_i%02d_j%02d", k, i, j);
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


void ModeloCplexR::setConstraints8(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, nVertices);
	int c = 0;

	for (int i = 1; i < nVertices-1; ++i){
		IloExpr expr = IloExpr(env);
		
		for (int j = 0; j < nVertices; j++){
			for (int k = 0; k < maxVeic; ++k){
				if (g->getCustoAresta(i,j) != MAIS_INFINITO){
					expr += x[k][i][j];
				}
			}
		}

		for (int j = 0; j < nVertices; j++){
			for (int k = 0; k < maxVeic; ++k){
				if (g->getCustoAresta(j,i) != MAIS_INFINITO){
					expr += x[k][j][i];
				}
			}
		}

		constr[c] = (expr == 2);
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r8_i%02d", i);
		constr[c].setName(nome);
		model.add(constr[c]);
		expr.end();
		++c;
	}
	constr.end();
}


void ModeloCplexR::setConstraints9(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k){
		IloExpr expr = IloExpr(env);

		for (int q = 1; q <= nCommodities; ++q){
			expr += g->getPesoCommodity(q) * y[q][k];
		}
		
		constr[k] = (expr <= g->getCapacVeiculo());
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r9_k%02d", k);
		constr[k].setName(nome);
		model.add(constr[k]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexR::setConstraints10(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int k = 0; k < maxVeic; ++k){
		IloExpr expr = IloExpr(env);

		for (int q = nCommodities+1; q < nVertices-1; ++q){
			expr += g->getPesoCommodity(q) * y[q][k];
		}
		
		constr[k] = (expr <= g->getCapacVeiculo());
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r10_k%02d", k);
		constr[k].setName(nome);
		model.add(constr[k]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexR::setConstraints11(){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nCommodities);
	int c = 0;

	for (int q = 1; q <= nCommodities; ++q){
		for (int k = 0; k < maxVeic; ++k){
			IloExpr expr = IloExpr(env);
			expr = y[q][k] + y[q+nCommodities][k] - u[q][k];
			constr[c] = (expr <= 1);
			//Insere a restrição no modelo
			char nome[20];
			sprintf(nome, "r11_q%02d_k%02d", q, k);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexR::setConstraints12(){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nCommodities);
	int c = 0;

	for (int q = 1; q <= nCommodities; ++q){
		for (int k = 0; k < maxVeic; ++k){
			IloExpr expr = IloExpr(env);
			expr = y[q][k] + y[q+nCommodities][k] - 2*u[q][k];
			constr[c] = (expr >= 0);
			//Insere a restrição no modelo
			char nome[20];
			sprintf(nome, "r12_q%02d_k%02d", q, k);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}

