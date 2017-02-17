#include "ModeloCplexR.h"
using namespace std;

ModeloCplexR::~ModeloCplexR(){
	cplex.end();
	model.end();
	env.end();
}

ModeloCplexR::ModeloCplexR(Grafo* g, int numCommodities, int maxVeiculos, int custoCD, int janelaCD, char* arqSaida, float* classesV) : 
env(), model(env), cplex(model), maxVeic(maxVeiculos), nCommodities(numCommodities), nVertices(2*(numCommodities+1)), classesVeic(classesV){
	
	//instancia as variaveis de decisao do modelo																						
	initVars();
	
	//Define a funcao objetivo
	setObjectiveFunction(g, custoCD);

	//Insere as restricoes no modelo
	setConstraints1();
	setConstraints2();	
	setConstraints3(g);
	setConstraints4();
	setConstraints5(g);
	setConstraints6(g);
	setConstraints7(g);
	setConstraints8(g);
	setConstraints9();
	setConstraints10();
	if (janelaCD > 0) setConstraintsJanela(g, janelaCD);
	if (arqSaida != NULL) cplex.exportModel(arqSaida);
}

void ModeloCplexR::initVars(){

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

	x = IloArray <IloArray <IloNumVarArray> > (env, maxVeic);
	for(int q = 0; q < maxVeic; ++q){
		x[q] = IloArray<IloNumVarArray>(env, nVertices); 
		for(int i = 0; i < nVertices; ++i){
			x[q][i] = IloNumVarArray(env, nVertices, 0, 1, ILOFLOAT);
			for (int j = 0; j < nVertices; ++j){
				char nomeVar[20];
				sprintf(nomeVar, "x%d_%02d_%02d", q, i, j);
				x[q][i][j].setName(nomeVar);
			}
		}
	}

	y = IloArray<IloNumVarArray>(env, maxVeic);
	for(int q = 0; q < maxVeic; ++q){
		y[q] = IloNumVarArray(env, nVertices, 0, 1, ILOFLOAT);
		for (int i = 0; i < nVertices; ++i){
			char nomeVar[20];
			sprintf(nomeVar, "y%d_%02d", q, i);
			y[q][i].setName(nomeVar);
		}
	}
	
	u = IloArray<IloNumVarArray>(env, maxVeic);
	for (int q = 0; q < maxVeic; ++q){
		u[q] = IloNumVarArray(env, nCommodities+1, 0, 1, ILOFLOAT);
		for(int i = 0; i <= nCommodities; ++i){
			char nomeVar[20];
			sprintf(nomeVar, "uq%di%02d", q, i);
			u[q][i].setName(nomeVar);
		}
	}
}


void ModeloCplexR::setObjectiveFunction(Grafo *g, int custoCrossDocking){
	IloExpr obj(env);
	float custo;

	//custo nos arcos
	for (int q = 0; q < maxVeic; ++q){ 
		for (int i = 0; i < nVertices; ++i){
			for (int j = 0; j < nVertices; ++j){
				custo = g->getCustoAresta(i,j);				
				if (custo != MAIS_INFINITO) obj += (classesVeic[q]*custo) * x[q][i][j];
			}
		}
	}
	
	//custo de troca de mercadorias no cross-docking
	for (int i = 1; i <= nCommodities; ++i){
		for (int q = 0; q < maxVeic; ++q){
			obj -= custoCrossDocking*u[q][i];
		}
	}
	obj += (custoCrossDocking*nCommodities);
			
	model.add(IloMinimize(env, obj));
	obj.end();
}


void ModeloCplexR::setConstraints1(){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int q = 0; q < maxVeic; ++q){
		IloExpr expr = IloExpr(env);

		for (int j = 1; j <= nCommodities; ++j){
			expr += f[q][0][j];
		}

		for (int i = 1; i <= nCommodities; ++i){
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


void ModeloCplexR::setConstraints2(){
	IloRangeArray constr = IloRangeArray(env, maxVeic);
	int c = 0;

	for (int q = 0; q < maxVeic; ++q){
		IloExpr expr = IloExpr(env);

		for (int j = nCommodities+1; j < nVertices-1; ++j){
			expr += f[q][0][j];
		}

		for (int i = nCommodities+1; i < nVertices-1; ++i){
			expr -= y[q][i];
		}

		//Insere a restrição no modelo
		constr[c] = (expr == 1);
		char nome[20];
		sprintf(nome, "r2_q%d", q);
		constr[c].setName(nome);
		model.add(constr[c]);
		expr.end();
		++c;
	}
	constr.end();
}


void ModeloCplexR::setConstraints3(Grafo* g){
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
			sprintf(nome, "r3_q%di%02d", q, i);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexR::setConstraints4(){
	IloRangeArray constr = IloRangeArray(env, nVertices);

	for (int i = 1; i < nVertices-1; ++i){
		IloExpr expr = IloExpr(env);

		for (int q = 0; q < maxVeic; ++q){
			expr += y[q][i];
		}
		
		constr[i] = (expr == 1);
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r4_i%d", i);
		constr[i].setName(nome);
		model.add(constr[i]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexR::setConstraints5(Grafo* g){
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


void ModeloCplexR::setConstraints6(Grafo *g){
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


void ModeloCplexR::setConstraints7(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int q = 0; q < maxVeic; ++q){
		IloExpr expr = IloExpr(env);

		for (int i = 1; i <= nCommodities; ++i){
			expr += g->getPesoCommodity(i) * y[q][i];
		}
		
		constr[q] = (expr <= ceil(classesVeic[q] * g->getCapacVeiculo()));
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r7_q%d", q);
		constr[q].setName(nome);
		model.add(constr[q]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexR::setConstraints8(Grafo* g){
	IloRangeArray constr = IloRangeArray(env, maxVeic);

	for (int q = 0; q < maxVeic; ++q){
		IloExpr expr = IloExpr(env);

		for (int i = nCommodities+1; i < nVertices-1; ++i){
			expr += g->getPesoCommodity(i) * y[q][i];
		}
		
		constr[q] = (expr <= ceil(classesVeic[q] * g->getCapacVeiculo()));
		//Insere a restrição no modelo
		char nome[20];
		sprintf(nome, "r8_q%d", q);
		constr[q].setName(nome);
		model.add(constr[q]);
		expr.end();
	}
	constr.end();
}


void ModeloCplexR::setConstraints9(){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nCommodities);
	int c = 0;

	for (int q = 0; q < maxVeic; ++q){
		for (int i = 1; i <= nCommodities; ++i){
			IloExpr expr = IloExpr(env);
			expr += y[q][i] + y[q][i+nCommodities] - u[q][i];
			constr[c] = (expr <= 1);
			//Insere a restrição no modelo
			char nome[20];
			sprintf(nome, "r9_q%di%02d", q, i);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}


void ModeloCplexR::setConstraints10(){
	IloRangeArray constr = IloRangeArray(env, maxVeic*nCommodities);
	int c = 0;

	for (int q = 0; q < maxVeic; ++q){
		for (int i = 1; i <= nCommodities; ++i){
			IloExpr expr = IloExpr(env);
			expr += y[q][i] + y[q][i+nCommodities] - 2*u[q][i];
			constr[c] = (expr >= 0);
			//Insere a restrição no modelo
			char nome[20];
			sprintf(nome, "r10_q%di%02d", q, i);
			constr[c].setName(nome);
			model.add(constr[c]);
			expr.end();
			++c;
		}
	}
	constr.end();
}

void ModeloCplexR::setConstraintsJanela(Grafo* g, int janelaCD){
	int depositoArtificial = 2*nCommodities+1;
	int inicioJanelaDeposito = g->getInicioJanela(0);
	int fimJanelaDeposito = g->getFimJanela(0);
	int fimJanelaForn = g->getMaiorFimJanelaForn();
	int inicioJanelaCons = g->getMenorInicioJanelaCons();
	int diferenca = inicioJanelaCons - fimJanelaForn;
	if (diferenca >= 0){
		fimJanelaForn += ((diferenca - janelaCD) / 2);
		inicioJanelaCons -= ((diferenca - janelaCD) / 2);
	}else{
		fimJanelaForn -= ((janelaCD/2) - (diferenca/2));
		inicioJanelaCons += ((janelaCD/2) - (diferenca/2));
	}
	cout << "Janelas:" << endl << "[" << inicioJanelaDeposito << "," << fimJanelaForn << "] : [" << inicioJanelaCons << "," << fimJanelaDeposito << "]\n";

	//inicializa as variaveis de decisao 't'
	t = IloArray <IloNumVarArray> (env, maxVeic);
	for (int q = 0; q < maxVeic; ++q){
		t[q] = IloNumVarArray(env, nVertices, 0, INT_MAX, ILOFLOAT);
		for (int j = 0; j < nVertices; j++){
			char nome[20];
			sprintf(nome, "t_%01d_%02d", q, j);
			t[q][j].setName(nome);
		}
	}

	//RESTRICOES DE PRECEDENCIA
	for (int q = 0; q < maxVeic; ++q){

		//do deposito ate todos os fornecedores
		for (int j = 1; j <= nCommodities; ++j){
			IloExpr expr = IloExpr(env);
			expr = t[q][j] - 3000 * x[q][0][j];
			model.add(expr >= -3000 + inicioJanelaDeposito + g->getCustoAresta(0, j));
			expr.end();
		}

		//do deposito ate todos os consumidores
		for (int j = nCommodities+1; j <= 2*nCommodities; ++j){
			IloExpr expr = IloExpr(env);
			expr = t[q][j] - 3000 * x[q][0][j];
			model.add(expr >= -3000 + inicioJanelaCons + g->getCustoAresta(0, j));
			expr.end();
		}

		//de todos os fornecedores ate o cross-docking
		for (int j = 1; j <= nCommodities; ++j){
			IloExpr expr = IloExpr(env);
			expr = fimJanelaForn - t[q][j] - 3000 * x[q][j][depositoArtificial];
			model.add(expr >= -3000 + g->getCustoAresta(j, depositoArtificial));
			expr.end();
		}

		//de todos os consumidores ate o cross-docking
		for (int j = nCommodities+1; j <= 2*nCommodities; ++j){
			IloExpr expr = IloExpr(env);
			expr = fimJanelaDeposito - t[q][j] - 3000 * x[q][j][depositoArtificial];
			model.add(expr >= -3000 + g->getCustoAresta(j, depositoArtificial));
			expr.end();
		}

		for (int i = 1; i <= 2*nCommodities; ++i){
			for (int j = 1; j <= 2*nCommodities; ++j){
				if (g->getCustoAresta(i,j) != MAIS_INFINITO){
					IloExpr expr = IloExpr(env);
					expr = t[q][j] - t[q][i] - 3000 * x[q][i][j];
					model.add(expr >= -3000 + g->getCustoAresta(i, j));
					expr.end();
				}
			}
		}
	}

	//RESTRICOES DE VIABILIDADE DE JANELA
	for (int q = 0; q < maxVeic; ++q){
		for (int i = 1; i <= 2*nCommodities; ++i){
			model.add(t[q][i] >= g->getInicioJanela(i));
			model.add(t[q][i] <= g->getFimJanela(i));
		}
	}
}

float ModeloCplexR::optimize(int timeLimit){
	cplex.setParam(IloCplex::TiLim, timeLimit);
	cplex.solve();
	if ((cplex.getStatus() == IloAlgorithm::Feasible) || (cplex.getStatus() == IloAlgorithm::Optimal)){
		return cplex.getObjValue();
	}else{
		return MAIS_INFINITO;
	}
}
