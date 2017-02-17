#include "ElementarCplex.h"
using namespace std;

int ElementarCplex::nCommodities;
int ElementarCplex::ajuste;
IloEnv ElementarCplex::env;
IloModel ElementarCplex::model;
IloCplex ElementarCplex::cplex;
IloArray <IloNumVarArray> ElementarCplex::f;
IloArray <IloIntVarArray> ElementarCplex::x;
IloIntVarArray ElementarCplex::y;
IloNumVarArray ElementarCplex::t;
IloObjective ElementarCplex::fObjetivo;

ElementarCplex::ElementarCplex(){

}

void ElementarCplex::freeModel(){
	cplex.end();
	model.end();
	env.end();
}

void ElementarCplex::initModel(Grafo* g){
	env = IloEnv();
	model = IloModel(env);
	cplex = IloCplex(model);
	nCommodities = g->getNumCmdt();
	
	cplex.setOut(env.getNullStream());

	initVars(g);
	setConstraints1();
	setConstraints2();	
	setConstraints3();
	setConstraints4();
	setConstraints5();
	setConstraints6();
	setConstraints7(g);
}

void ElementarCplex::initVars(Grafo* g){

	f = IloArray <IloNumVarArray> (env, nCommodities+2);
	for(int i = 0; i < (nCommodities+2); ++i){
		f[i] = IloNumVarArray(env, (nCommodities+2), 0, INT_MAX, ILOFLOAT);
		for (int j = 0; j < (nCommodities+2); j++){
			char nome[20];
			sprintf(nome, "f_%02d_%02d", i, j);
			f[i][j].setName(nome);
		}
	}

	x = IloArray<IloIntVarArray>(env, nCommodities+2);
	for(int i = 0; i < (nCommodities+2); ++i){
		x[i] = IloIntVarArray(env, (nCommodities+2), 0, 1);
		for (int j = 0; j < (nCommodities+2); j++){
			char nome[20];
			sprintf(nome, "x_%02d_%02d", i, j);
			x[i][j].setName(nome);
		}
	}

	y = IloIntVarArray(env, (nCommodities+2), 0, 1);
	for (int j = 0; j < (nCommodities+2); j++){
		char nome[20];
		sprintf(nome, "y_%02d", j);
		y[j].setName(nome);
	}

	//Funcao objetivo, que recebera valores dos coeficientes antes da execucao
	fObjetivo = IloObjective(env);
	model.add(fObjetivo);
}

void ElementarCplex::setObjectiveFunction(Grafo *g){
	float custo;
	IloExpr obj(env);
	int depositoArtificial = 2*nCommodities+1;

	//Inicializa o custo daquelas arestas que nao existem no grafo com valor MAIS_INFINITO
	obj += MAIS_INFINITO * x[0][0];
	obj += MAIS_INFINITO * x[0][nCommodities+1];
	for (int j = 0; j < (nCommodities+2); ++j){
		obj += MAIS_INFINITO * x[nCommodities+1][j];
	}
	for (int i = 1; i <= nCommodities; ++i){
		obj += MAIS_INFINITO * x[i][0];
	}

	//Inicializa os demais custos (alguns podem ser MAIS_INFINITO, como as arestas (i,i)
	for (int j = 1; j <= nCommodities; j++){
		obj += g->getCustoArestaDual(0, j+ajuste) * x[0][j];
		obj += g->getCustoArestaDual(j+ajuste, depositoArtificial) * x[j][nCommodities+1];
	}
	for(int i = 1; i <= nCommodities; ++i){
		for (int j = 1; j <= nCommodities; j++){
			obj += g->getCustoArestaDual(i+ajuste, j+ajuste) * x[i][j];
		}
	}

	//atribui a nova expressao a funcao objetivo
	fObjetivo.setExpr(obj);
}

void ElementarCplex::setConstraints1(){
	IloExpr expr = IloExpr(env);
	for (int j = 1; j <= nCommodities; ++j){
		expr += f[0][j];
	}
	for (int i = 1; i <= nCommodities; ++i){
		expr -= y[i];
	}
	model.add(expr == 1);
	expr.end();	
}

void ElementarCplex::setConstraints2(){
	for (int i = 1; i <= nCommodities; ++i){
		IloExpr expr = IloExpr(env);

		for (int j = 1; j < nCommodities+2; ++j){
			if (i != j){
				expr += f[i][j];
			}
		}
		for (int h = 0; h <= nCommodities; ++h){
			if (i != h){
				expr -= f[h][i];
			}
		}
		expr += y[i];
		model.add(expr == 0);
		expr.end();
	}
}

void ElementarCplex::setConstraints3(){
	IloExpr expr = IloExpr(env);
	for (int i = 1; i <= nCommodities; ++i){
		expr += f[i][nCommodities+1];
	}
	model.add(expr == 1);
	expr.end();	
}


void ElementarCplex::setConstraints4(){
	for (int i = 1; i <= nCommodities; ++i){
		IloExpr expr = IloExpr(env);
		expr = f[0][i] - nCommodities*x[0][i];
		model.add(expr <= 0);
		expr.end();

		IloExpr expr2 = IloExpr(env);
		expr2 = f[i][nCommodities+1] - nCommodities*x[i][nCommodities+1];
		model.add(expr2 <= 0);
		expr2.end();
	}
	for (int i = 1; i <= nCommodities; ++i){
		for (int j = 1; j <= nCommodities; ++j){
			if (i != j){
				IloExpr expr = IloExpr(env);
				expr = f[i][j] - nCommodities*x[i][j];
				model.add(expr <= 0);
				expr.end();
			}
		}
	}
}

void ElementarCplex::setConstraints5(){
	for (int i = 1; i <= nCommodities; ++i){
		IloExpr expr = IloExpr(env);
		expr = x[0][i] - f[0][i];
		model.add(expr <= 0);
		expr.end();

		IloExpr expr2 = IloExpr(env);
		expr2 = x[i][nCommodities+1] - f[i][nCommodities+1];
		model.add(expr2 <= 0);
		expr2.end();
	}
	for (int i = 1; i <= nCommodities; ++i){
		for (int j = 1; j <= nCommodities; ++j){
			if (i != j){
				IloExpr expr = IloExpr(env);
				expr = x[i][j] - f[i][j];
				model.add(expr <= 0);
				expr.end();
			}
		}
	}
}

void ElementarCplex::setConstraints6(){
	for (int i = 1; i <= nCommodities; ++i){
		IloExpr expr = IloExpr(env);

		for (int j = 1; j < nCommodities+2; ++j){
			if (i != j){
				expr += x[i][j];
			}
		}
		for (int h = 0; h <= nCommodities; ++h){
			if (i != h){
				expr += x[h][i];
			}
		}
		expr -= 2*y[i];
		model.add(expr == 0);
		expr.end();
	}
}

void ElementarCplex::setConstraints7(Grafo* g){
	IloExpr expr = IloExpr(env);
	for (int i = 1; i <= nCommodities; ++i){
		expr += g->getPesoCommodity(i) * y[i];
	}
	model.add(expr <= g->getCapacVeiculo());
	expr.end();
}

void ElementarCplex::setConstraintsJanela(Grafo* g){
	int depositoArtificial = 2*nCommodities+1;
	t = IloNumVarArray(env, (nCommodities+2), 0, INT_MAX, ILOFLOAT);
	for (int j = 0; j < (nCommodities+2); j++){
		char nome[20];
		sprintf(nome, "t_%02d", j);
		t[j].setName(nome);
	}

	//RESTRICOES DE PRECEDENCIA
	for (int j = 1; j <= nCommodities; ++j){
		IloExpr expr = IloExpr(env);
		expr = t[j] - 3000 * x[0][j];
		model.add(expr >= -3000 + g->getCustoAresta(0, j+ajuste));
		expr.end();

		IloExpr expr2 = IloExpr(env);
		expr2 = t[nCommodities+1] - t[j] - 3000 * x[j][nCommodities+1];
		model.add(expr2 >= -3000 + g->getCustoAresta(j+ajuste, depositoArtificial));
		expr2.end();
	}
	for (int i = 1; i <= nCommodities; ++i){
		for (int j = 1; j <= nCommodities; ++j){
			if (i != j){
				IloExpr expr = IloExpr(env);
				expr = t[j] - t[i] - 3000 * x[i][j];
				model.add(expr >= -3000 + g->getCustoAresta(i+ajuste, j+ajuste));
				expr.end();
			}
		}
	}


	//RESTRICOES DE VIABILIDADE DE JANELA
	for (int i = 1; i <= nCommodities; ++i){
		model.add(t[i] >= g->getInicioJanela(i+ajuste));
		model.add(t[i] <= g->getFimJanela(i+ajuste));
	}
	model.add(t[nCommodities+1] >= g->getInicioJanela(0));
	model.add(t[nCommodities+1] <= g->getFimJanela(0));
}

void ElementarCplex::calculaCaminhoElementar(Grafo* g, bool camForn, bool janelaTempo){
	ajuste = camForn ? 0 : nCommodities;
	setObjectiveFunction(g);
	if (janelaTempo) setConstraintsJanela(g);
	cplex.solve();
}

Rota** ElementarCplex::getRotaCustoMinimo(Grafo* g, float valorSubtrair){
	if (cplex.getObjValue() < (valorSubtrair - 0.001)){
		int proxVertice = 0, zeroLinha = nCommodities+1;
		Rota** r = new Rota*[2];

		r[0] = new Rota();
		r[0]->inserirVerticeInicio(0);
		r[1] = NULL;

		for (int i = 1; i <= zeroLinha; ++i){			
			if (cplex.getValue(f[0][i]) > 0){
				proxVertice = i;
				break;
			}
		}

		do{
			r[0]->inserirVerticeFim(proxVertice+ajuste);
			for (int i = 1; i <= zeroLinha; ++i){
				if ((proxVertice != i) && (cplex.getValue(f[proxVertice][i]) > 0)){
					proxVertice = i;
					break;
				}
			}
		}while(proxVertice != zeroLinha);

		r[0]->inserirVerticeFim(2*nCommodities+1);
		r[0]->setCustoReduzido(g);
		r[0]->setCusto(g);
		
		return r;
	}else{
		
		return NULL;
	}
}
