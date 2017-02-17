#include "ModeloCplex.h"
using namespace std;

float ModeloCplex::limitePrimal;
ModeloCplex::~ModeloCplex(){
	delete [] xiPtInterior;
	for (int r = 0; r < qRotas; ++r){
		delete [] a_ir[r];
		if (ptrRotas[r]->decrNumApontadores()) delete ptrRotas[r];
	}		
	a_ir.clear();
	ptrRotas.clear();
	env.end();
}

ModeloCplex::ModeloCplex(Grafo* g, Rota** rotasI, int maxVeiculos, int custoTroca, int nPInteriores)
: env(), modelPrimal(env), cplexPrimal(modelPrimal),  modelDual(env), cplexDual(modelDual),
  maxVeic(maxVeiculos), nRequests(g->getNumReqs()), custoTrocaCD(custoTroca), nPontosInteriores(nPInteriores){
	cplexPrimal.setOut(env.getNullStream());
	cplexDual.setOut(env.getNullStream());

	qRotas = 0;	//Sera incrementada para armazenar a quantidade de rotas obtidas na solucao inicial
	while(rotasI[qRotas] != NULL) ++qRotas;

	//aloca memoria para o vetor que armazenara os valores da variavel dual XI
	xiPtInterior = new float[nRequests+1];

	//armazena como atributos da classe as matrizes ptrRotasForn, ptrRotasCons, A_qr e B_qr
	preencheMatriz_a_ir(rotasI);

	//Montagem do modelo Primal
	constroiModeloPrimal();

	//Montagem do modelo Dual
	constroiModeloDual();
}

void ModeloCplex::preencheMatriz_a_ir(Rota** rotas){
	vector<int> v;
	short int* r;
	int tam, tamanhoVetor = 2*nRequests+1;

	//Cada celula de a_ir apontara para um vetor de inteiros que armazenara 1 caso o vertice i esteja na rota r
	//0 caso contrario, para todos os vertices do grafo, exceto o deposito e o depositoArtificial
	for (int i = 0; i < qRotas; ++i){
		r = new short int[tamanhoVetor];
		memset(r, 0, tamanhoVetor*sizeof(short int));
		a_ir.push_back(r);
		v = rotas[i]->getVertices();
		tam = v.size()-1;
		for(int j = 1; j < tam; ++j){
			a_ir[i][v[j]] = 1; //coloca-se 1 apenas naquelas posicoes em que o vertice esteja na rota
		}
		ptrRotas.push_back(rotas[i]);
		rotas[i]->incrNumApontadores();
	}
}

void ModeloCplex::constroiModeloPrimal(){
	//Variaveis de decisao
	lambda = IloNumVarArray(env, qRotas, 0, 1, ILOFLOAT);
	for (int j = 0; j < qRotas; j++){
		char nome[20];
		sprintf(nome, "lbd_%02d", j);
		lambda[j].setName(nome);
	}
	tau = IloNumVarArray(env, (nRequests+1), 0, 1, ILOFLOAT);
	for (int j = 0; j <= nRequests; j++){
		char nome[20];
		sprintf(nome, "tau_%02d", j);
		tau[j].setName(nome);
	}

	//Funcao Objetivo
	IloExpr obj(env);
	for (int j = 0; j < qRotas; ++j){
		obj += ptrRotas[j]->getCusto() * lambda[j];
	}
	for (int j = 1; j <= nRequests; ++j){
		obj += custoTrocaCD * tau[j];
	}
	costPrimal = IloObjective(env, obj);
	modelPrimal.add(costPrimal);

	//Restricoes - Constraint 1
	IloExpr exp1 = IloExpr(env);
	for (int j = 0; j < qRotas; ++j){
		exp1 += lambda[j];
	}
	constraintP1 = (exp1 == maxVeic);
	constraintP1.setName("rest1");
	modelPrimal.add(constraintP1);
	exp1.end();

	//Restricoes - Constraints 2
	constraintsP2 = IloRangeArray(env, 2*nRequests);
	for (int i = 1; i <= 2*nRequests; ++i){
		IloExpr exp2 = IloExpr(env);
		for (int j = 0; j < qRotas; ++j){
			if (a_ir[j][i] > 0){
				exp2 += a_ir[j][i] * lambda[j];
			}
		}
		constraintsP2[i-1] = (exp2 == 1);
		char nome[20];
		sprintf(nome, "rest2_i%02d", i);
		constraintsP2[i-1].setName(nome);
		modelPrimal.add(constraintsP2[i-1]);
		exp2.end();	
	}

	//Restricoes - Constraints 3
	constraintsP3 = IloRangeArray(env, nRequests);
	for (int i = 1; i <= nRequests; ++i){
		IloExpr exp3 = IloExpr(env);
		exp3 += tau[i];
		for (int j = 0; j < qRotas; ++j){
			if ((a_ir[j][i] == 0) && (a_ir[j][i+nRequests] > 0)){
				exp3 -= lambda[j];
			}
		}
		constraintsP3[i-1] = (exp3 >= 0);
		char nome[20];
		sprintf(nome, "rest3_i%02d", i);
		constraintsP3[i-1].setName(nome);
		modelPrimal.add(constraintsP3[i-1]);
		exp3.end();
	}
}

void ModeloCplex::constroiModeloDual(){
	//Variaveis de decisao
	alfa = IloNumVar(env, -IloInfinity, +IloInfinity, ILOFLOAT);
	alfa.setName("alfa");
	theta = IloNumVarArray(env, 2*nRequests+1, -IloInfinity, +IloInfinity, ILOFLOAT);
	for (int i = 0; i <= 2*nRequests; ++i){
		char nome[20];
		sprintf(nome, "teta_%02d", i);
		theta[i].setName(nome);
	}
	xi = IloNumVarArray(env, (nRequests+1), 0, +IloInfinity, ILOFLOAT);
	for (int i = 0; i <= nRequests; ++i){
		char nome[20];
		sprintf(nome, "xi_%02d", i);
		xi[i].setName(nome);
	}

	//A funcao objetivo sera determinada antes de resolver o dual, pois o dual sera executado n vezes
	//para obter n pontos extremos da envoltoria dual, para isto, valores aleatorios sao atribuidos
	//aos coeficientes das variaveis de decisao dual na funcao objetivo no metodo 'setObjectiveDual()'
	costDual = IloObjective(env);
	modelDual.add(costDual);

	//Restricoes - Constraints 1
	constraintsD1 = IloRangeArray(env, qRotas);
	for (int r = 0; r < qRotas; ++r){
		IloExpr expD1 = IloExpr(env);
		expD1 = alfa;
		for (int i = 1; i <= 2*nRequests; ++i){
			if (a_ir[r][i] > 0){
				expD1 += a_ir[r][i] * theta[i];
			}
		}
		for (int i = 1; i <= nRequests; ++i){
			if ((a_ir[r][i] == 0) && (a_ir[r][i+nRequests] > 0)){
				expD1 -= xi[i];
			}
		}
		constraintsD1[r] = (expD1 <= ptrRotas[r]->getCusto());
		char nome[20];
		sprintf(nome, "rest1_r%02d", r);
		constraintsD1[r].setName(nome);
		modelDual.add(constraintsD1[r]);
		expD1.end();
	}

	//Restricoes - Constraints 2
	constraintsD2 = IloRangeArray(env, nRequests);
	for (int i = 0; i < nRequests; ++i){
		constraintsD2[i] = (xi[i+1] <= custoTrocaCD);
		char nome[20];
		sprintf(nome, "rest2_i%02d", i+1);
		constraintsD2[i].setName(nome);
		modelDual.add(constraintsD2[i]);
	}
}

void ModeloCplex::updateDualCosts(Grafo* g){
	int nVertices = 2*nRequests;

	if (nPontosInteriores == 0){

		//insere nos vertices do grafo os valores das variaveis duais theta
		IloNumArray dualValuesConstraintsP2(env);
		cplexPrimal.getDuals(dualValuesConstraintsP2, constraintsP2);
		for (int i = 1; i <= nVertices; ++i){
			g->setCustoVerticeDual(i, -dualValuesConstraintsP2[i-1]);
		}
		g->setCustoArestasDual();

		//armzena nas variaveis da classe os valores de alfa e xi, para serem retornados pelas funcoes 'get'
		alfaPtInterior = cplexPrimal.getDual(constraintP1);
		IloNumArray dualValuesConstraintsP3(env);
		cplexPrimal.getDuals(dualValuesConstraintsP3, constraintsP3);
		xiPtInterior[0] = 0;
		for (int i = 0; i < nRequests; ++i){
			xiPtInterior[i+1] = dualValuesConstraintsP3[i];
		}

	}else{
		geraDualPontosInteriores();
		int count = 0;
		alfaPtInterior = 0;
		float* dualValues = new float[nVertices+1];
		memset(dualValues, 0, (nVertices+1)*sizeof(float));
		memset(xiPtInterior, 0, (nRequests+1)*sizeof(float));
		
		for (int i = 0; i < nPontosInteriores; ++i){
			setObjectiveDualPontosInteriores();
			if (cplexDual.solve()){
				for (int i = 1; i <= nVertices; ++i){
					dualValues[i] += cplexDual.getValue(theta[i]);
					if (i <= nRequests) xiPtInterior[i] += cplexDual.getValue(xi[i]);
				}
				alfaPtInterior += cplexDual.getValue(alfa);
				++count;
			}
		}
		if (count){
			for (int i = 1; i <= 2*nRequests; ++i){
				g->setCustoVerticeDual(i, -dualValues[i]/count);
				if (i <= nRequests) xiPtInterior[i] /= count;
			}
			alfaPtInterior /= count;
			g->setCustoArestasDual();
			delete [] dualValues;

		}else{
			cout << "Modelo Dual inviavel para todas iteracoes" << endl;
			exit(0);
		}
	}
}

void ModeloCplex::insert_ColumnPrimal_RowDual(Rota* r){
	//Armazena a rota na matriz de rotas, que sera usada posteriormente
	short int* rota = new short int[2*nRequests+1];
	memset(rota, 0, (2*nRequests+1) * sizeof(short int));
	a_ir.push_back(rota);
	ptrRotas.push_back(r);
	r->incrNumApontadores();

	vector <int> vertRota = r->getVertices();
	int numVertAtualizar = vertRota.size()-1;
	for(int j = 1; j < numVertAtualizar; ++j){
		++a_ir[qRotas][vertRota[j]]; //+1 apenas nas posicoes que o vertice esteja na rota
	}

	//CRIA-SE UMA COLUNA (PRIMAL) E UMA RESTRICAO (DUAL) ASSOCIADA A ROTA
	IloNumColumn col = costPrimal(r->getCusto());
	col += constraintP1(1);
	IloExpr res = alfa;

	for (int i = 1; i <= 2*nRequests; i++){
		if (a_ir[qRotas][i] > 0){
			col += constraintsP2[i-1](a_ir[qRotas][i]);
			res += a_ir[qRotas][i] * theta[i];
		}
	}
	for (int i = 1; i <= nRequests; i++){
		if ((a_ir[qRotas][i] == 0) && (a_ir[qRotas][i+nRequests] > 0)){
			col += constraintsP3[i-1](-1);
			res -= xi[i];
		}
	}

	char nome[20];
	sprintf(nome, "lbd%d", qRotas);
	lambda.add(IloNumVar(col, 0, 1, ILOFLOAT, nome));
	col.end();
	constraintsD1.add(res <= r->getCusto());
	modelDual.add(constraintsD1[qRotas]);
	res.end();

	++qRotas;
}

float ModeloCplex::solveMaster(){
	cplexPrimal.solve();
	return cplexPrimal.getObjValue();
}

void ModeloCplex::exportModel(const char* arqExport, char opcao){
	if (opcao == 'P'){
		cplexPrimal.exportModel(arqExport);
	}else{
		cplexDual.exportModel(arqExport);
	}
}

void ModeloCplex::geraDualPontosInteriores(){
	//para gerar o modelo dual deve-se fixar os valores de modo que a complementaridade de folga
	//(Ax - b) y = 0 seja atendida para todas as restricoes e variaveis do par primal-dual
	IloNum rhs;
	float ZERO = 0.00005;

	//fixa os valores das restricoes duais em funcao das variaveis de decisao do primal
	IloNumArray lambdaValues(env);
	cplexPrimal.getValues(lambdaValues, lambda);
	for (int r = 0; r < qRotas; ++r){
		rhs = constraintsD1[r].getUB();
		if ((lambdaValues[r] >= -ZERO) && (lambdaValues[r] <= ZERO)){
			constraintsD1[r].setBounds(-IloInfinity, rhs);
		}else{
			constraintsD1[r].setBounds(rhs, rhs);
		}
	}
	lambdaValues.end();

	IloNum tauValue;
	for(int i = 1; i <= nRequests; ++i){
		tauValue = cplexPrimal.getValue(tau[i]);
		if ((tauValue >= -ZERO) && (tauValue <= ZERO)){
			constraintsD2[i-1].setBounds(-IloInfinity, custoTrocaCD);
		}else{
			constraintsD2[i-1].setBounds(custoTrocaCD, custoTrocaCD);
		}
	}
	
	//fixa os valores das variaveis de decisao do dual em funcao das variaveis de folga das restricoes do primal
	rhs = cplexPrimal.getSlack(constraintP1);
	if ((rhs <= -ZERO) || (rhs >= ZERO)){
		alfa.setBounds(0, 0);
	}else{
		alfa.setBounds(-IloInfinity, +IloInfinity);
	}
	
	IloNumArray slacksP2(env);
	cplexPrimal.getSlacks(slacksP2, constraintsP2);
	for (int i = 1; i <= (2*nRequests); ++i){
		if  ((slacksP2[i-1] <= -ZERO) || (slacksP2[i-1] >= ZERO)){
			theta[i].setBounds(0, 0);
		}else{
			theta[i].setBounds(-IloInfinity, +IloInfinity);
		}
	}
	slacksP2.end();

	IloNumArray slacksP3(env);
	cplexPrimal.getSlacks(slacksP3, constraintsP3);
	for (int i = 1; i <= nRequests; ++i){
		if  ((slacksP3[i-1] <= -ZERO) || (slacksP3[i-1] >= ZERO)){
			xi[i].setBounds(0, 0);
		}else{
			xi[i].setBounds(0, +IloInfinity);
		}
	}
	slacksP3.end();
}

void ModeloCplex::setObjectiveDualPontosInteriores(){
	IloExpr multiplicadoresObj(env);
	multiplicadoresObj = /*((rand()%101) / 100.0) * */maxVeic * alfa;
	for (int i = 1; i <= (2*nRequests); ++i){
		multiplicadoresObj += /*((rand()%101) / 100.0) * */theta[i];
	}
	costDual.setExpr(multiplicadoresObj);
	costDual.setSense(IloObjective::Maximize);
}

bool ModeloCplex::currentSolInteira(bool raiz) {
	IloNumArray lambdaValues(env);
	cplexPrimal.getValues(lambdaValues, lambda);
	for (int i = 0; i < lambdaValues.getSize(); ++i) { //variaveis lambda do no raiz, validas globalmente
		if ((lambdaValues[i] > 0.00001) && (lambdaValues[i] < 0.99999)) return false;
	}

	if ( !raiz ) {
		cplexPrimal.getValues(lambdaValues, lambda_no);
		for (int i = 0; i < lambdaValues.getSize(); ++i) { //variaveis lambda deste no
			if ((lambdaValues[i] > 0.00001) && (lambdaValues[i] < 0.99999)) return false;
		}
	}
	return true;
}

float ModeloCplex::getAlfa(){
	return alfaPtInterior;
}

float* ModeloCplex::getXi(){
	return xiPtInterior;
}

void ModeloCplex::setLimitePrimal(float lP){
	limitePrimal = lP;
}

float ModeloCplex::getLimitePrimal(){
	return limitePrimal;
}

void ModeloCplex::imprimeRotasBasicas(){
	IloNumArray values(env);
	cplexPrimal.getValues(values, lambda);
	for (int r = 0; r < qRotas; ++r)
	{
		if (values[r] >= 0.00001)
		{
			printf("lambda[%d](%f): ", r, values[r]);
			ptrRotas[r]->imprimir();
		}
	}
	values.end();
}
