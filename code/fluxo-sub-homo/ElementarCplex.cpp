#include "ElementarCplex.h"
using namespace std;

ElementarCplex::ElementarCplex(Grafo* g, int custoCD){
	env = IloEnv();
	model = IloModel(env);
	cplex = IloCplex(model);
	nRequests = g->getNumReqs();
	nVertices = 2*nRequests;

	initVars();
	setObjectiveFunction(g, custoCD);
	setConstraints1(g);
	setConstraints2(g);
	setConstraints3(g);
	setConstraints4(g);
	setConstraints5(g);
	setConstraints6(g);
	setConstraints7(g);
	setConstraints8();

	cplex.setOut(env.getNullStream());
}

ElementarCplex::~ElementarCplex(){
	cplex.end();
	model.end();
	env.end();
}

void ElementarCplex::initVars(){

	f = IloArray <IloNumVarArray> (env, (nVertices+2));
	for(int i = 0; i < (nVertices+2); ++i){
		f[i] = IloNumVarArray(env, (nVertices+2), 0, 9999, ILOFLOAT);
		for (int j = 0; j < (nVertices+2); j++){
			char nome[20];
			sprintf(nome, "f_%02d_%02d", i, j);
			f[i][j].setName(nome);
		}
	}

	x = IloArray<IloIntVarArray>(env, (nVertices+2));
	for(int i = 0; i < (nVertices+2); ++i){
		x[i] = IloIntVarArray(env, (nVertices+2), 0, 1);
		for (int j = 0; j < (nVertices+2); j++){
			char nome[20];
			sprintf(nome, "x_%02d_%02d", i, j);
			x[i][j].setName(nome);
		}
	}

	y = IloIntVarArray(env, (nVertices+2), 0, 1);
	for (int j = 0; j < (nVertices+2); j++){
		char nome[20];
		sprintf(nome, "y_%02d", j);
		y[j].setName(nome);
	}

	t = IloIntVarArray(env, (nRequests+1), 0, 1);
	for (int j = 0; j <= nRequests; j++){
		char nome[20];
		sprintf(nome, "t_%02d", j);
		t[j].setName(nome);
	}
}

void ElementarCplex::setObjectiveFunction(Grafo *g, int custoCD){
	float custo;
	IloExpr obj(env);
	int depositoArtificial = nVertices+1;
	for(int i = 0; i <= depositoArtificial; ++i)
	{
		for (int j = 0; j <= depositoArtificial; j++)
		{
			if ( g->existeAresta( i , j ) ) obj += g->getCustoArestaDual( i , j ) * x[i][j];
		}
	}
	for(int i = 1; i <= nRequests; ++i) obj += custoCD * t[i];
	model.add(IloObjective(env, obj));
}

void ElementarCplex::setConstraints1(Grafo* g){
	IloExpr expr = IloExpr(env);
	for (int j = 1; j <= nRequests; ++j)
	{
		if (g->existeAresta( 0 , j ) ) expr += f[0][j];
	}
	for (int i = 1; i <= nVertices; ++i) expr -= y[i];

	model.add(expr == 1);
	expr.end();	
}

void ElementarCplex::setConstraints2(Grafo* g){
	for (int i = 1; i <= nVertices; ++i)
	{
		IloExpr expr = IloExpr(env);
		for ( int j = 0; j <= (nVertices+1); ++j )
		{
			if (g->existeAresta( i , j ) )
			{
				expr += f[i][j];
			}
		}
		for (int h = 0; h <= (nVertices+1); ++h)
		{
			if ( g->existeAresta( h , i ) )
			{
				expr -= f[h][i];
			}
		}
		expr += y[i];
		model.add(expr == 0);
		expr.end();
	}
}

void ElementarCplex::setConstraints3(Grafo* g){
	IloExpr expr = IloExpr(env);
	for (int i = nRequests+1; i <= nVertices; ++i)
	{
		if ( g->existeAresta( i , nVertices+1 ) ) expr += f[i][nVertices+1];
	}
	model.add(expr == 1);
	expr.end();	
}


void ElementarCplex::setConstraints4(Grafo* g){
	for (int i = 0; i <= (nVertices+1); ++i)
	{
		for (int j = 0; j <= (nVertices+1); ++j)
		{
			if (g->existeAresta( i , j ) )
			{
				IloExpr expr = IloExpr(env);
				expr = f[i][j] - nVertices*x[i][j];
				model.add(expr <= 0);
				expr.end();
			}
		}
	}
}

void ElementarCplex::setConstraints5(Grafo* g){
	for (int i = 0; i <= (nVertices+1); ++i)
	{
		for (int j = 0; j <= (nVertices+1); ++j)
		{
			if (g->existeAresta( i , j ) )
			{
				IloExpr expr = IloExpr(env);
				expr = x[i][j] - f[i][j];
				model.add(expr <= 0);
				expr.end();
			}
		}
	}
}

void ElementarCplex::setConstraints6(Grafo* g){
	for (int i = 1; i <= nVertices; ++i)
	{
		IloExpr expr = IloExpr(env);
		for (int j = 0; j <= (nVertices+1); ++j)
		{
			if (g->existeAresta( i , j ) )
			{
				expr += x[i][j];
			}
		}
		for (int h = 0; h <= (nVertices+1); ++h)
		{
			if (g->existeAresta( h , i ) )
			{
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
	for (int i = 1; i <= nRequests; ++i)
	{
		expr += g->getCargaVertice(i) * y[i];
	}
	model.add(expr <= g->getCapacVeiculo());
	expr.end();

	IloExpr expr2 = IloExpr(env);
	for (int i = nRequests+1; i <= nVertices; ++i)
	{
		expr2 += g->getCargaVertice(i) * y[i];
	}
	model.add(expr2 <= g->getCapacVeiculo());
	expr2.end();
}

void ElementarCplex::setConstraints8(){
	for (int i = 1; i <= nRequests; ++i)
	{
		model.add(t[i] + y[i] - y[nRequests+i] >= 0);
	}
}

void ElementarCplex::calculaCaminhoElementar(){
	cplex.solve();
}

Rota* ElementarCplex::getRotaCustoMinimo(Grafo* g){
	int proxVertice = 0, zeroLinha = nVertices+1;
	Rota* r = new Rota();

	do{
		r->inserirVerticeFim(proxVertice);
		for (int i = 1; i <= zeroLinha; ++i){
			if ( ( g->existeAresta( proxVertice , i ) ) && ( cplex.getValue(f[proxVertice][i]) > 0.5 ) )
			{
				proxVertice = i;
				break;
			}
		}
	}while(proxVertice != zeroLinha);

	r->inserirVerticeFim(zeroLinha);
	r->setCustoRota(g);
	r->setCustoReduzido(cplex.getObjValue());
	return r;
}

