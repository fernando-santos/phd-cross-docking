#include "ModeloBC-RD.h"
using namespace std;

ILOINCUMBENTCALLBACK2(solIntCallBackRD, ModeloBC_RD&, modBC, Grafo*, g){
	//caso a solucao seja de custo negativo, a inclui no conjunto de rotas negativas
	if ( getObjValue() < (modBC.valorSubtrair - 0.01 ) )
	{
		IloArray<IloNumArray> x_values(modBC.env, modBC.nVertices+2);
		for (int i = 0; i <= modBC.nVertices+1; ++i){
			x_values[i] = IloNumArray(modBC.env);
			getValues(x_values[i], modBC.x[i]);
		}

		//cria a rota e a inclui no vector<Rota*>
		int proxVertice = 0, zeroLinha = modBC.nVertices+1;
		Rota* r = new Rota();
		r->incrNumApontadores();
		do{
			r->inserirVerticeFim(proxVertice);
			for ( int i = 1; i <= zeroLinha; ++i )
			{
				if ( x_values[proxVertice][i] > 0.5 )
				{
					proxVertice = i;
					break;
				}
			}
		}while(proxVertice != zeroLinha);
		r->inserirVerticeFim(zeroLinha);
		r->setCustoRota(g, true);
		r->setCustoReduzido(getObjValue());
		modBC.rotasNegativas.push_back(r);		

		//libera memoria
		for (int i = 0; i <= modBC.nVertices+1; ++i) x_values[i].end();
		x_values.end();

//		abort(); //interrompe a execucao do cplex apos encontrar a primeira rota de custo reduzido negativo
	}
}


ILOLAZYCONSTRAINTCALLBACK2(cutCallBackRD, ModeloBC_RD&, modBC, Grafo*, g){	
	//pega os valores das variaveis de decisao x
	IloArray<IloNumArray> x_values(modBC.env, modBC.nVertices+2);
	for (int i = 0; i <= modBC.nVertices+1; ++i){
		x_values[i] = IloNumArray(modBC.env);
		getValues(x_values[i], modBC.x[i]);
	}

	//pega os valores das variaveis de decisao y
	IloNumArray y_values(modBC.env);
	getValues(y_values, modBC.y);

	//constroi o grafo de dinic baseando-se nos valores das variaveis de decisao
	Fluxo* flow = new Fluxo(x_values, y_values, modBC.nRequests);

	int random, destino, nVDinic = flow->getNVerticesDinic();
	int* ordem = new int[--nVDinic]; //o vertice 0 nao entra como destino, portanto, diminue 1 no numero de vertices Dinic
	for (int i = 0; i < nVDinic; ++i) ordem[i] = (i+1);	

	bool inW;
	float maiorValorY;
	int indiceMaiorY, i = nVDinic;

	//executa o algoritmo de fluxo maximo de 0 para cada vertice do grafo Dinic (escolhido aleatoriamente)
	while ( i >= 1 ){
		random = rand() % i;
		destino = ordem[random];
		ordem [random] = ordem[--i];

		//executa o algoritmo de fluxo maximo, que retornara o conjunto W (ou um conjunto vazio, caso nao seja possivel obter corte)
		vector < int > conjW = flow->calculaFluxoMaximo(y_values, destino);

		//caso o conjunto nao seja vazio (ou seja, existe um conjunto W para esta iteracao), procura pelo corte x(W, V\W)
		if (conjW.size() > 0){

			int tam = conjW.size();
			vector < int > arcosCut;
			IloExpr expCorte(modBC.env);

			for (int j = 0; j < tam; ++j){

				if (conjW[j] == 0){ //arestas partindo do deposito artificial (o vertice 0 sempre esta em W, mas nao necessariamente esta em algum corte)
					for (int x = 1; x <= modBC.nRequests; ++x){
						if ( g->existeAresta( 0 , x ) )
						{
							inW = false;
							for (int y = 0; y < tam; ++y)
							{
								if (conjW[y] == x)
								{
									inW = true;
									break;
								}
							}
							if ( !inW )
							{
								expCorte += modBC.x[0][x];
								arcosCut.push_back(0);
								arcosCut.push_back(x);
							}
						}
					}

				}else if (conjW[j] <= modBC.nRequests){ //arestas partindo dos fornecedores
					for (int x = 1; x <= modBC.nVertices; ++x)
					{
						if ( g->existeAresta( conjW[j] , x ) )
						{
							inW = false;
							for (int y = 0; y < tam; ++y)
							{
								if (conjW[y] == x)
								{
									inW = true;
									break;
								}
							}
							if ( !inW )
							{
								expCorte += modBC.x[conjW[j]][x];
								arcosCut.push_back(conjW[j]);
								arcosCut.push_back(x);
							}
						}
					}

				}else{ //arestas partindo dos consumidores
					for (int x = modBC.nRequests+1; x <= modBC.nVertices; ++x)
					{
						if ( g->existeAresta( conjW[j] , x ) )
						{
							inW = false;
							for (int y = 0; y < tam; ++y){
								if (conjW[y] == x){
									inW = true;
									break;
								}
							}
							if ( !inW )
							{
								expCorte += modBC.x[conjW[j]][x];
								arcosCut.push_back(conjW[j]);
								arcosCut.push_back(x);
							}
						}
					}
				}
			}

			if ( arcosCut.size() > 0 )
			{
				//verifica qual o maiorValorY em V-W para compor a restricao
				maiorValorY = 0;
				for ( int x = 1; x <= modBC.nVertices; ++x )
				{
					if ( y_values[x] > maiorValorY )
					{
						inW = false;
						for (int y = 0; y < tam; ++y){
							if (conjW[y] == x){
								inW = true;
								break;
							}
						}
						if ( !inW )
						{
							maiorValorY = y_values[x];
							indiceMaiorY = x;
						}
					}
				}

				//adiciona ao vetor de restricoes violadas e ao vetor de arcos dos cortes
				add(expCorte - modBC.y[indiceMaiorY] >= 0);
				expCorte.end();

				//atualiza o grafo para que as arestas incluidas neste corte nao estejam no proximo corte
				flow->atualizaGrafoDinic(g, y_values, arcosCut);
			}
		}
	}
	delete [] ordem;

	//libera a memoria armazenada pelos objetos do cplex e do fluxo
	for (int i = 0; i <= modBC.nVertices+1; ++i) x_values[i].end();
	x_values.end();
	y_values.end();
	delete flow;
}

ModeloBC_RD::ModeloBC_RD(Grafo* g, float valSub){
	model = IloModel(env);
	cplex = IloCplex(model);
	nRequests = g->getNumReqs();
	nVertices = 2*nRequests;
	valorSubtrair = valSub;

	cplex.setOut(env.getNullStream());
	cplex.setWarning(env.getNullStream());

	initVars();
	setObjectiveFunction(g);
	setConstraints1e2(g);
	setConstraints3e4(g);
	setConstraints6e7(g);
}

ModeloBC_RD::~ModeloBC_RD(){
	env.end();
	for (int i = 0; i < rotasNegativas.size(); ++i)
	{
		if (rotasNegativas[i]->decrNumApontadores()) delete rotasNegativas[i];
	}
	rotasNegativas.clear();
}

void ModeloBC_RD::initVars(){
	x = IloArray<IloIntVarArray>( env, ( nVertices+2 ) );
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
}

void ModeloBC_RD::setObjectiveFunction( Grafo *g ){
	IloExpr obj(env);
	int depositoArtificial = nVertices+1;
	for(int i = 0; i <= depositoArtificial; ++i)
	{
		for (int j = 0; j <= depositoArtificial; j++)
		{
			if ( g->existeAresta( i , j ) ) 
			{
				obj += g->getCustoArestaDual( i , j ) * x[i][j];
			}
			else 
			{
				obj += MAIS_INFINITO * x[i][j];
			}
		}
	}
	obj += MAIS_INFINITO * y[depositoArtificial];
	obj += MAIS_INFINITO * y[0];

	model.add(IloObjective(env, obj));
}

void ModeloBC_RD::setConstraints1e2(Grafo* g){
	int depositoArtificial = nVertices+1;
	IloExpr expr = IloExpr(env);
	for (int j = 1; j <= nRequests; ++j){
		if ( g->existeAresta( 0 , j ) ) expr += x[0][j];
	}
	model.add(expr == 1);
	expr.end();	


	IloExpr expr2 = IloExpr(env);
	for (int i = (nRequests+1); i <= nVertices; ++i){
		if ( g->existeAresta( i , depositoArtificial ) ) expr2 += x[i][depositoArtificial];
	}
	model.add(expr2 == 1);
	expr2.end();	
}

void ModeloBC_RD::setConstraints3e4( Grafo* g ){
	for (int i = 1; i <= nVertices; ++i){
		IloExpr expr = IloExpr(env);
		for (int h = 0; h <= nVertices; ++h){
			if ( g->existeAresta( h , i ) ) expr += x[h][i];
		}
		expr -= y[i];
		model.add(expr == 0);
		expr.end();


		IloExpr expr2 = IloExpr(env);
		for (int j = 1; j <= (nVertices+1); ++j){
			if ( g->existeAresta( i , j ) )	expr2 += x[i][j];
		}
		expr2 -= y[i];
		model.add(expr2 == 0);
		expr.end();
	}	
}

void ModeloBC_RD::setConstraints6e7(Grafo* g){
	for (int i = 1; i <= nRequests; ++i){
		model.add(y[i] - y[nRequests+i] == 0);
	}	

	IloExpr expr = IloExpr(env);
	for (int i = 1; i <= nRequests; ++i) expr += g->getCargaVertice(i) * y[i];
	model.add( expr <= g->getCapacVeiculo() );
	expr.end();
}

void ModeloBC_RD::calculaCaminhoElementar(Grafo* g, int tempo){
	//inclusao dos callbacks para incluir cortes e capturar solucoes inteiras
	cplex.use( cutCallBackRD( env, *this, g ) );
	cplex.use( solIntCallBackRD( env, *this, g ) );

	//parametros para trabalhar com modelos sem incluir todas as restricoes
	cplex.setParam( IloCplex::PreInd, 0 );
	cplex.setParam( IloCplex::AggInd, 0 );
    cplex.setParam( IloCplex::HeurFreq, -1 );
    int timeLimit = 14400 - ( time(0) - tempo );
    if ( timeLimit < 10 ) timeLimit = 10;
	cplex.setParam( IloCplex::TiLim, timeLimit );
   
	//strong branching
	cplex.setParam( IloCplex::VarSel, 3 );
		
	//seta maior prioridade de branching nas variaveis y
	IloNumArray priorities(env, (nVertices+2));
	priorities[0] = 0; priorities[nVertices+1] = 0;
	for (int i = 1; i <= nVertices; ++i) priorities[i] = 1;
	cplex.setPriorities(y, priorities);

	//executa o cplex com o modelo e os parametros definidos acima
	cplex.solve();
}

