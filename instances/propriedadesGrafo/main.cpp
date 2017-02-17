#include "Grafo.h"
#include <stdlib.h>

void calculaPropriedade2(Grafo*, int, int);
void calculaPropriedade3(Grafo*, int, float);
void calculaPropriedade4(Grafo*, int, float);
void calculaPropriedade5(Grafo*, int, float);
void calculaPropriedade6(Grafo*, int, float);
void calculaPropriedade7(Grafo*, int, float);
void calculaPropriedade8(Grafo*, int, float);
void calculaPropriedade9(Grafo*, int);
void calculaPropriedade10(Grafo*, int);

int main(int argc, char** argv){

	if (argc != 3){
		 cout	<< "Parametros esperados:\n  (1) Arquivo de instancias Solomon \n"
			<< "  (2) Quantidade de Fornecedores (e consequentemente Consumidores) a considerar\n\n";
		exit(0);
	}
	int nCommodities = atoi(argv[2]), aux;
	int nVertices = 2*nCommodities + 1;

	Grafo* G = new Grafo(argv[1], nCommodities);
	//G->imprimeGrafo();

	cout << "PROPRIEDADES A SEREM OBTIDAS DO GRAFO:" << endl;
	cout << "1) Distancia media entre os vertices (não inclui cross-docking)" << endl;
	cout << "2) Distancia media de cada vertice para os (n) vizinhos mais proximos" << endl;
	cout << "3) Distancia media do menor arco que sai de cada vertice para outro mais proximo," << endl;
	cout << "   que nao esteja dentro de um raio (r) deste vertice" << endl;
	cout << "4) Numero medio de vizinhos de um vertice considerando um raio (r)" << endl;
	cout << "5) Para cada vertice calcular o numero minimo de vizinhos que nao podem ser atendidos por um veiculo dentro do raio (r)" << endl;
	cout << "   devido as restricoes de capacidade dos veiculos. Tirar a media considerando todos os FORNECEDORES/CONSUMIDORES" << endl;
	cout << "6) Para cada par [fornecedor, consumidor], verificar o numero total de mercadorias (outros pares [fornecedor', consumidor'])" << endl;
	cout << "   onde o fornecedor' esteja dentro de um raio (r) do fornecedor e o consumidor' esteja fora do raio (r) de consumidor " << endl;
	cout << "   ou vice-versa (o consumidor' esteja dentro de um raio (r) do consumidor e o fornecedor' esteja fora do raio (r) de fornecedor)" << endl;
	cout << "7) A mesma propriedade de (6), mas considerando apenas vértices 'extremos' dos eixos X e Y para cada par [fornecedor, consumidor]" << endl;
	cout << "8) Dados 2 clusters de vertices tanto de forncedores quanto de consumidores, cuja capacidade eh inferior a capacidade do veiculo, " << endl;
	cout << "   verifica-se o numero de trocas de mercadorias entre os clusters e a distancia entre os pontos medios dos clusters(cada cluster 1 veiculo)\n";
	cout << "9) Soma acumulada de todas as demandas/ofertas das mercadorias" << endl;
	cout << "10) Distancia acumulada do CD aos 4 vertices extremos: direita/esquerda, forncedores/consumidores " << endl << endl;

	float distMediaForn = 0, distMediaCons = 0, tmp;
	for (int i = 1; i <= nCommodities; ++i){
		tmp = 0;
		for (int j = 1; j <= nCommodities; ++j){
			if (i != j){
				tmp += G->getCustoAresta(i,j);
			}
		}
		tmp /= (nCommodities - 1);
		distMediaForn += tmp;
	} 
	distMediaForn /= nCommodities;
	
	for (int i = nCommodities+1; i <= 2*nCommodities; ++i){
		tmp = 0;
		for (int j = nCommodities+1; j <= 2*nCommodities; ++j){
			if (i != j){
				tmp += G->getCustoAresta(i,j);
			}
		}
		tmp /= (nCommodities - 1);
		distMediaCons += tmp;
	} 
	distMediaCons /= nCommodities;
	cout << "1)" << (distMediaForn + distMediaCons) / 2 << endl;

	
	cout << "2)";
	calculaPropriedade2(G, nCommodities, 3);
	calculaPropriedade2(G, nCommodities, 5);
	cout << endl;

	cout << "3)";
	calculaPropriedade3(G, nCommodities, 10);
	calculaPropriedade3(G, nCommodities, 20);
	calculaPropriedade3(G, nCommodities, 30);
	cout << endl;

	cout << "4)";
	calculaPropriedade4(G, nCommodities, 10);
	calculaPropriedade4(G, nCommodities, 20);
	calculaPropriedade4(G, nCommodities, 30);
	cout << endl;

	cout << "5)";
	calculaPropriedade5(G, nCommodities, 10);
	calculaPropriedade5(G, nCommodities, 20);
	calculaPropriedade5(G, nCommodities, 30);
	cout << endl;

	cout << "6)";
	calculaPropriedade6(G, nCommodities, 10);
	calculaPropriedade6(G, nCommodities, 20);
	calculaPropriedade6(G, nCommodities, 30);
	cout << endl;

	cout << "7)";
	calculaPropriedade7(G, nCommodities, 10);
	calculaPropriedade7(G, nCommodities, 20);
	calculaPropriedade7(G, nCommodities, 30);
	cout << endl;

	cout << "8)";
	calculaPropriedade8(G, nCommodities, 0);
	calculaPropriedade8(G, nCommodities, 5);
	calculaPropriedade8(G, nCommodities, 10);
	cout << endl;

	cout << "9)";
	calculaPropriedade9(G, nCommodities);
	cout << endl;

	cout << "10)";
	calculaPropriedade10(G, nCommodities);
	cout << endl;
}


void calculaPropriedade2(Grafo* G, int nCommodities, int n){
	bool igual;
	int indiceMenor[n];
	float valorMenor, mediaMenoresForn = 0, mediaMenoresCons = 0, tmp;

	cout << "n = " << n;
	for (int i = 1; i <= nCommodities; ++i){
		tmp = 0;
		for (int j = 0; j < n; ++j){
			valorMenor = MAIS_INFINITO;
			for (int k = 1; k <= nCommodities; ++k){
				if (G->getCustoAresta(i,k) < valorMenor){
					igual = false;
					for (int l = 0; l < j; ++l){
						if (k == indiceMenor[l]){
							igual = true;
							break;
						}
					}
					if (!igual){
						valorMenor = G->getCustoAresta(i,k);
						indiceMenor[j] = k;
					}
				}
			}
			tmp += G->getCustoAresta(i,indiceMenor[j]);
		}
		mediaMenoresForn += tmp/n;
	}
	mediaMenoresForn /= nCommodities;

	for (int i = nCommodities+1; i <= 2*nCommodities; ++i){
		tmp = 0;
		for (int j = 0; j < n; ++j){
			valorMenor = MAIS_INFINITO;
			for (int k = nCommodities+1; k <= 2*nCommodities; ++k){
				if (G->getCustoAresta(i,k) < valorMenor){
					igual = false;
					for (int l = 0; l < j; ++l){
						if (k == indiceMenor[l]){
							igual = true;
							break;
						}
					}
					if (!igual){
						valorMenor = G->getCustoAresta(i,k);
						indiceMenor[j] = k;
					}
				}
			}
			tmp += G->getCustoAresta(i,indiceMenor[j]);
		}
		mediaMenoresCons += (tmp/n);
	}
	mediaMenoresCons /= nCommodities;
	cout << "(" << (mediaMenoresForn + mediaMenoresCons) / 2 << ")\n  ";
}


void calculaPropriedade3(Grafo* G, int nCommodities, float raio){
	int indiceMenor;
	float valorMenor, mediaMenorDistForaRaioForn = 0, mediaMenorDistForaRaioCons = 0;

	cout << "r = " << raio;
	for (int i = 1; i <= nCommodities; ++i){
		valorMenor = MAIS_INFINITO;
		for (int j = 1; j <= nCommodities; ++j){
			if ((G->getCustoAresta(i,j) > raio) && (G->getCustoAresta(i,j) < valorMenor)){
				valorMenor = G->getCustoAresta(i,j);
				indiceMenor = j; 
			}
		}
		mediaMenorDistForaRaioForn += G->getCustoAresta(i,indiceMenor);
	}
	mediaMenorDistForaRaioForn /= nCommodities;
	
	for (int i = nCommodities+1; i <= 2*nCommodities; ++i){
		valorMenor = MAIS_INFINITO;
		for (int j = nCommodities+1; j <= 2*nCommodities; ++j){
			if ((G->getCustoAresta(i,j) > raio) && (G->getCustoAresta(i,j) < valorMenor)){
				valorMenor = G->getCustoAresta(i,j);
				indiceMenor = j; 
			}
		}
		mediaMenorDistForaRaioCons += G->getCustoAresta(i,indiceMenor);
	}
	mediaMenorDistForaRaioCons /= nCommodities;
	cout << "(" << (mediaMenorDistForaRaioForn + mediaMenorDistForaRaioCons) / 2 << ")\n  ";

}


void calculaPropriedade4(Grafo* G, int nCommodities, float raio){
	int numVizinhosRaioForn = 0, numVizinhosRaioCons = 0;

	cout << "r = " << raio;
	for (int i = 1; i <= nCommodities; ++i){
		for (int j = 1; j <= nCommodities; ++j){
			if (G->getCustoAresta(i,j) < raio){
				++numVizinhosRaioForn;
			}
		}
	}
	numVizinhosRaioForn /= (float)nCommodities;


	for (int i = nCommodities+1; i <= 2*nCommodities; ++i){
		for (int j = nCommodities+1; j <= 2*nCommodities; ++j){
			if (G->getCustoAresta(i,j) < raio){
				++numVizinhosRaioCons;
			}
		}
	}
	numVizinhosRaioCons /= (float)nCommodities;
	cout << "(" << (numVizinhosRaioForn + numVizinhosRaioCons) / 2 << ")\n  ";

}


void calculaPropriedade5(Grafo* G, int nCommodities, float raio){
	int numVizinhosRaioForn, aux;
	float cargaVizinhosRaioForn;
	float mediaVizinhosCargaForn = 0;

	cout << "r = " << raio;
	for (int i = 1; i <= nCommodities; ++i){
		numVizinhosRaioForn = 0;
		cargaVizinhosRaioForn = 0;
		for (int j = 1; j <= nCommodities; ++j){
			if (G->getCustoAresta(i,j) < raio){
				++numVizinhosRaioForn;
				cargaVizinhosRaioForn += G->getPesoCommodity(j);
			}
		}

		//Caso a demanda dos vizinhos excedam com a capacidade de carga do veiculo
		//copia-se todas as demandas dos vizinhos para um vetor, as ordena e retira
		//uma a uma até que o total das demandas seja menor ou igual a capacidade
		if (cargaVizinhosRaioForn > G->getCapacVeiculo()){
			int *vetorCargas = new int[nCommodities];

			//Copia as cargas para um vetor, que sera ordenado
			for (int i = 0; i < nCommodities; ++i){
				vetorCargas[i] = G->getPesoCommodity(i+1);
			}

			//ordena o vetor de cargas em ordem decrescente
			for (int i = 0; i < nCommodities; ++i){
				for (int j = i+1; j < nCommodities; ++j){
					if (vetorCargas[i] > vetorCargas[j]){
						aux = vetorCargas[i];
						vetorCargas[i] = vetorCargas[j];
						vetorCargas[j] = aux;
					}
				}
			}

			//retira as maiores cargas, até que a capacidade do veiculo seja atendida
			aux = 0;
			while(cargaVizinhosRaioForn > G->getCapacVeiculo()){
				cargaVizinhosRaioForn -= vetorCargas[aux++];
				numVizinhosRaioForn--;
			}

			delete [] vetorCargas;
		}
		mediaVizinhosCargaForn += numVizinhosRaioForn;
	}
	mediaVizinhosCargaForn /= (float)nCommodities;


	int numVizinhosRaioCons;
	float cargaVizinhosRaioCons;
	float mediaVizinhosCargaCons = 0;
	for (int i = nCommodities+1; i <= 2*nCommodities; ++i){
		numVizinhosRaioCons = 0;
		cargaVizinhosRaioCons = 0;
		for (int j = nCommodities+1; j <= 2*nCommodities; ++j){
			if (G->getCustoAresta(i,j) < raio){
				++numVizinhosRaioCons;
				cargaVizinhosRaioCons += G->getPesoCommodity(j);
			}
		}

		//Caso a demanda dos vizinhos excedam com a capacidade de carga do veiculo
		//copia-se todas as demandas dos vizinhos para um vetor, as ordena e retira
		//uma a uma até que o total das demandas seja menor ou igual a capacidade
		if (cargaVizinhosRaioCons > G->getCapacVeiculo()){
			int *vetorCargas = new int[nCommodities];

			//Copia as cargas para um vetor, que sera ordenado
			for (int i = 0; i < nCommodities; ++i){
				vetorCargas[i] = G->getPesoCommodity(nCommodities+i+1);
			}

			//ordena o vetor de cargas em ordem decrescente
			for (int i = 0; i < nCommodities; ++i){
				for (int j = i+1; j < nCommodities; ++j){
					if (vetorCargas[i] > vetorCargas[j]){
						aux = vetorCargas[i];
						vetorCargas[i] = vetorCargas[j];
						vetorCargas[j] = aux;
					}
				}
			}

			//retira as maiores cargas, até que a capacidade do veiculo seja atendida
			aux = 0;
			while(cargaVizinhosRaioCons > G->getCapacVeiculo()){
				cargaVizinhosRaioCons -= vetorCargas[aux++];
				numVizinhosRaioCons--;
			}

			delete [] vetorCargas;
		}
		mediaVizinhosCargaCons += numVizinhosRaioCons;
	}
	mediaVizinhosCargaCons /= (float)nCommodities;
	cout << "(" << (mediaVizinhosCargaForn + mediaVizinhosCargaCons) / 2 << ")\n  ";
}


void calculaPropriedade6(Grafo* G, int nCommodities, float raio){
	int numVizinhosRaio;

	cout << "r = " << raio;
	numVizinhosRaio = 0;
	for (int i = 1; i <= nCommodities; ++i){
		for (int j = 1; j <= nCommodities; ++j){
			if ((i != j) &&
				((G->getCustoAresta(i, j) < raio) && (G->getCustoAresta(i+nCommodities, j+nCommodities) > raio)) ||
				((G->getCustoAresta(i, j) > raio) && (G->getCustoAresta(i+nCommodities, j+nCommodities) < raio))) {
				numVizinhosRaio++;
			}
		}
	}
	cout << "(" << numVizinhosRaio << ")\n  ";
}


void calculaPropriedade7(Grafo* G, int nCommodities, float raio){
	int numVizinhosRaioForn, numVizinhosRaioCons, fornEsq, fornDir, fornInf, consEsq, consDir, consSup, tmpX, tmpY;
	float valFornEsq = MAIS_INFINITO, valFornDir = 0, valFornInf = MAIS_INFINITO, valConsEsq = MAIS_INFINITO, valConsDir = 0, valConsSup = 0;

	//Obtem os vertices extremos dos FORNECEDORES
	for (int i = 1; i <= nCommodities; i++){
		tmpX = G->getPosX(i);
		tmpY = G->getPosY(i);
		if (tmpX < valFornEsq){
			valFornEsq = tmpX;
			fornEsq = i;
		}
		if (tmpX > valFornDir){
			valFornDir = tmpX;
			fornDir = i;
		}
		if (tmpY < valFornInf){
			valFornInf = tmpY;
			fornInf = i;
		}
	}

	//Obtem os vertices extremos dos CONSUMIDORES
	for (int i = nCommodities+1; i <= 2*nCommodities; i++){
		tmpX = G->getPosX(i);
		tmpY = G->getPosY(i);
		if (tmpX < valConsEsq){
			valConsEsq = tmpX;
			consEsq = i;
		}
		if (tmpX > valConsDir){
			valConsDir = tmpX;
			consDir = i;
		}
		if (tmpY > valConsSup){
			valConsSup = tmpY;
			consSup = i;
		}
	}

	cout << "r = " << raio;
	numVizinhosRaioForn = numVizinhosRaioCons = 0;
	for (int j = 1; j <= nCommodities; ++j){
		if ((fornEsq != j) &&
			((G->getCustoAresta(fornEsq, j) < raio) && (G->getCustoAresta(fornEsq+nCommodities, j+nCommodities) > raio)) ||
			((G->getCustoAresta(fornEsq, j) > raio) && (G->getCustoAresta(fornEsq+nCommodities, j+nCommodities) < raio))) {
			numVizinhosRaioForn++;
		}
	}
	for (int j = 1; j <= nCommodities; ++j){
		if ((fornDir != j) &&
			((G->getCustoAresta(fornDir, j) < raio) && (G->getCustoAresta(fornDir+nCommodities, j+nCommodities) > raio)) ||
			((G->getCustoAresta(fornDir, j) > raio) && (G->getCustoAresta(fornDir+nCommodities, j+nCommodities) < raio))) {
			numVizinhosRaioForn++;
		}
	}
	for (int j = 1; j <= nCommodities; ++j){
		if ((fornInf != j) &&
			((G->getCustoAresta(fornInf, j) < raio) && (G->getCustoAresta(fornInf+nCommodities, j+nCommodities) > raio)) ||
			((G->getCustoAresta(fornInf, j) > raio) && (G->getCustoAresta(fornInf+nCommodities, j+nCommodities) < raio))) {
			numVizinhosRaioForn++;
		}
	}
	for (int j = 1+nCommodities; j <= 2*nCommodities; ++j){
		if ((consEsq != j) &&
			((G->getCustoAresta(consEsq, j) < raio) && (G->getCustoAresta(consEsq-nCommodities, j-nCommodities) > raio)) ||
			((G->getCustoAresta(consEsq, j) > raio) && (G->getCustoAresta(consEsq-nCommodities, j-nCommodities) < raio))) {
			numVizinhosRaioCons++;
		}
	}
	for (int j = 1+nCommodities; j <= 2*nCommodities; ++j){
		if ((consDir != j) &&
			((G->getCustoAresta(consDir, j) < raio) && (G->getCustoAresta(consDir-nCommodities, j-nCommodities) > raio)) ||
			((G->getCustoAresta(consDir, j) > raio) && (G->getCustoAresta(consDir-nCommodities, j-nCommodities) < raio))) {
			numVizinhosRaioCons++;
		}
	}
	for (int j = 1+nCommodities; j <= 2*nCommodities; ++j){
		if ((consSup != j) &&
			((G->getCustoAresta(consSup, j) < raio) && (G->getCustoAresta(consSup-nCommodities, j-nCommodities) > raio)) ||
			((G->getCustoAresta(consSup, j) > raio) && (G->getCustoAresta(consSup-nCommodities, j-nCommodities) < raio))) {
			numVizinhosRaioCons++;
		}
	}

	cout << "(" << (numVizinhosRaioForn + numVizinhosRaioCons) / 2 << ")\n  ";
}


void calculaPropriedade8(Grafo* G, int nCommodities, float limite){
	int numVizinhosRaioForn, numVizinhosRaioCons, fornEsq, fornDir, consEsq, consDir, tmpX, tmpY, raioEsq, raioDir;
	float valFornEsq = MAIS_INFINITO, valFornDir = 0, valConsEsq = MAIS_INFINITO, valConsDir = 0;

	//Obtem os vertices extremos dos FORNECEDORES
	for (int i = 1; i <= nCommodities; i++){
		tmpX = G->getPosX(i);
		if (tmpX < valFornEsq){
			valFornEsq = tmpX;
			fornEsq = i;
		}
		if (tmpX > valFornDir){
			valFornDir = tmpX;
			fornDir = i;
		}
	}

	//Obtem os vertices extremos dos CONSUMIDORES
	for (int i = nCommodities+1; i <= 2*nCommodities; i++){
		tmpX = G->getPosX(i);
		if (tmpX < valConsEsq){
			valConsEsq = tmpX;
			consEsq = i;
		}
		if (tmpX > valConsDir){
			valConsDir = tmpX;
			consDir = i;
		}
	}

	//Identifica os clusters nos fornecedores
	int cargaEsq = 0, cargaDir = 0, capVeic = G->getCapacVeiculo();
	bool continuaEsq = true, continuaDir = true;
	raioEsq = 1; raioDir = 1;
	while((continuaEsq || continuaDir) && ((G->getPosX(fornEsq) + raioEsq) < (G->getPosX(fornDir) - raioDir))){
		if (continuaEsq){
			cargaEsq = G->getPesoCommodity(fornEsq);
			for (int j = 1; j <= nCommodities; j++){
				if (G->getCustoAresta(fornEsq, j) <= raioEsq) {
					cargaEsq += G->getPesoCommodity(j);
				}
			}
			if (cargaEsq < capVeic){
				raioEsq++;
			}else if (cargaEsq > capVeic){
				raioEsq--;
				continuaEsq = false;
			}else{
				continuaEsq = false;
			}
		}

		if (continuaDir){
			cargaDir = G->getPesoCommodity(fornDir);
			for (int j = 1; j <= nCommodities; j++){
				if (G->getCustoAresta(fornDir, j) <= raioDir) {
					cargaDir += G->getPesoCommodity(j);
				}
			}
			if (cargaDir < capVeic){
				raioDir++;
			}else if (cargaDir > capVeic){
				raioDir--;
				continuaDir = false;
			}else{
				continuaDir = false;
			}
		}
	}

	int clusterFornEsq[nCommodities], clusterFornDir[nCommodities], cardClusterFornEsq = 0, cardClusterFornDir = 0;
	clusterFornEsq[cardClusterFornEsq++] = fornEsq;
	clusterFornDir[cardClusterFornDir++] = fornDir;
	for (int j = 1; j <= nCommodities; j++){
		if ((j != fornEsq) && (j != fornDir)){
			if (G->getCustoAresta(fornEsq, j) <= raioEsq) {
				clusterFornEsq[cardClusterFornEsq++] = j;
			}else if (G->getCustoAresta(fornDir, j) <= raioDir){
				clusterFornDir[cardClusterFornDir++] = j;
			}else if ((G->getCustoAresta(j, fornEsq) <= G->getCustoAresta(j, fornDir))){
				clusterFornEsq[cardClusterFornEsq++] = j;
			}else{
				clusterFornDir[cardClusterFornDir++] = j;
			}
		}
	}


	//Identifica os clusters nos consumidores
	cargaEsq = 0, cargaDir = 0;
	continuaEsq = true, continuaDir = true;
	raioEsq = 1; raioDir = 1;
	while((continuaEsq || continuaDir) && ((G->getPosX(consEsq) + raioEsq) < (G->getPosX(consDir) - raioDir))){
		if (continuaEsq){
			cargaEsq = G->getPesoCommodity(consEsq);
			for (int j = nCommodities+1; j <= 2*nCommodities; j++){
				if (G->getCustoAresta(consEsq, j) <= raioEsq) {
					cargaEsq += G->getPesoCommodity(j);
				}
			}
			if (cargaEsq < capVeic){
				raioEsq++;
			}else if (cargaEsq > capVeic){
				raioEsq--;
				continuaEsq = false;
			}else{
				continuaEsq = false;
			}
		}

		if (continuaDir){
			cargaDir = G->getPesoCommodity(consDir);
			for (int j = nCommodities+1; j <= 2*nCommodities; j++){
				if (G->getCustoAresta(consDir, j) <= raioDir) {
					cargaDir += G->getPesoCommodity(j);
				}
			}
			if (cargaDir < capVeic){
				raioDir++;
			}else if (cargaDir > capVeic){
				raioDir--;
				continuaDir = false;
			}else{
				continuaDir = false;
			}
		}
	}

	int clusterConsEsq[nCommodities], clusterConsDir[nCommodities], cardClusterConsEsq = 0, cardClusterConsDir = 0;
	clusterConsEsq[cardClusterConsEsq++] = consEsq;
	clusterConsDir[cardClusterConsDir++] = consDir;
	for (int j = nCommodities+1; j <= 2*nCommodities; j++){
		if ((j != consEsq) && (j != consDir)){
			if (G->getCustoAresta(consEsq, j) <= raioEsq) {
				clusterConsEsq[cardClusterConsEsq++] = j;
			}else if (G->getCustoAresta(consDir, j) <= raioDir){
				clusterConsDir[cardClusterConsDir++] = j;
			}else if ((G->getCustoAresta(j, consEsq) <= G->getCustoAresta(j, consDir))){
				clusterConsEsq[cardClusterConsEsq++] = j;
			}else{
				clusterConsDir[cardClusterConsDir++] = j;
			}
		}
	}


	//pos-processamento: aqueles fornecedores/consumidores que estiverem em clusters distintos,
	//mas a distancia entre eles for menor que ?7.5? serao excluidos dos clusters, pois devido a curta
	//distancia, eles podem ser atendidos pelo mesmo veiculo, seja ele indo para a direita ou esquerda
	bool eliminaEsq;
	int numEliminados = 0;
	for (int i = 0; i < cardClusterFornEsq; i++){
		eliminaEsq = false;
		for (int j = 0; j < cardClusterFornDir; ++j){
			if (G->getCustoAresta(clusterFornEsq[i],clusterFornDir[j]) < limite){
				for (int k = 0; k < cardClusterConsDir; k++){
					if (clusterConsDir[k] == clusterFornDir[j]+nCommodities){
						clusterConsDir[k] = clusterConsDir[--cardClusterConsDir];
						break;
					}
				}
				for (int k = 0; k < cardClusterConsEsq; k++){
					if (clusterConsEsq[k] == clusterFornDir[j]+nCommodities){
						clusterConsEsq[k] = clusterConsEsq[--cardClusterConsEsq];
						break;
					}
				}
				clusterFornDir[j--] = clusterFornDir[--cardClusterFornDir];
				++numEliminados;
				eliminaEsq = true;
			}
		}
		if (eliminaEsq){
			for (int k = 0; k < cardClusterConsDir; k++){
				if (clusterConsDir[k] == clusterFornEsq[i]+nCommodities){
					clusterConsDir[k] = clusterConsDir[--cardClusterConsDir];
					break;
				}
			}
			for (int k = 0; k < cardClusterConsEsq; k++){
				if (clusterConsEsq[k] == clusterFornEsq[i]+nCommodities){
					clusterConsEsq[k] = clusterConsEsq[--cardClusterConsEsq];
					break;
				}
			}
			clusterFornEsq[i--] = clusterFornEsq[--cardClusterFornEsq];
			++numEliminados;
		}	
	}
	for (int i = 0; i < cardClusterConsEsq; i++){
		eliminaEsq = false;
		for (int j = 0; j < cardClusterConsDir; ++j){
			if (G->getCustoAresta(clusterConsEsq[i],clusterConsDir[j]) < limite){
				for (int k = 0; k < cardClusterFornDir; k++){
					if (clusterFornDir[k] == clusterConsDir[j]-nCommodities){
						clusterFornDir[k] = clusterFornDir[--cardClusterFornDir];
						break;
					}
				}
				for (int k = 0; k < cardClusterFornEsq; k++){
					if (clusterFornEsq[k] == clusterConsDir[j]-nCommodities){
						clusterFornEsq[k] = clusterFornEsq[--cardClusterFornEsq];
						break;
					}
				}
				clusterConsDir[j--] = clusterConsDir[--cardClusterConsDir];
				++numEliminados;
				eliminaEsq = true;
			}
		}
		if (eliminaEsq){
			for (int k = 0; k < cardClusterFornDir; k++){
				if (clusterFornDir[k] == clusterConsEsq[i]-nCommodities){
					clusterFornDir[k] = clusterFornDir[--cardClusterFornDir];
					break;
				}
			}
			for (int k = 0; k < cardClusterFornEsq; k++){
				if (clusterFornEsq[k] == clusterConsEsq[i]-nCommodities){
					clusterFornEsq[k] = clusterFornEsq[--cardClusterFornEsq];
					break;
				}
			}
			clusterConsEsq[i] = clusterConsEsq[--cardClusterConsEsq];
			++numEliminados;
		}	
	}

	int mercadoriasDentroClusterEsq = 0;
	bool encontrou;
	for (int i = 0; i < cardClusterFornEsq; ++i){
		encontrou = false;
		for (int j = 0; j < cardClusterConsEsq; ++j){
			if (clusterFornEsq[i]+nCommodities == clusterConsEsq[j]){
				encontrou = true;
				break;
			}
		}
		if (encontrou){
			mercadoriasDentroClusterEsq++;
		}
	}

	int mercadoriasDentroClusterDir = 0;
	for (int i = 0; i < cardClusterFornDir; ++i){
		encontrou = false;
		for (int j = 0; j < cardClusterConsDir; ++j){
			if (clusterFornDir[i]+nCommodities == clusterConsDir[j]){
				encontrou = true;
				break;
			}
		}
		if (encontrou){
			mercadoriasDentroClusterDir++;
		}
	}

	bool veiculoTrocaLado = false;
	int totalInsercoesV1 = cardClusterConsEsq - mercadoriasDentroClusterEsq;
	int totalInsercoesV2 = cardClusterConsDir - mercadoriasDentroClusterDir;
	if (((cardClusterConsDir - (cardClusterFornEsq - mercadoriasDentroClusterEsq)) + 
			(cardClusterConsEsq - (cardClusterFornDir - mercadoriasDentroClusterEsq))) < (totalInsercoesV1 + totalInsercoesV2)){
		totalInsercoesV1 = cardClusterConsDir - (cardClusterFornEsq - mercadoriasDentroClusterEsq);
		totalInsercoesV2 = cardClusterConsEsq - (cardClusterFornDir - mercadoriasDentroClusterDir);
		veiculoTrocaLado = true;
	}

	//Calcula o 'centro' do cluster baseando-se no ponto medio das suas extremidades X e Y
	//e entao verifica a distancia envolvida em uma troca de mercadorias
	//CLUSTER FORNECEDOR ESQUERDA
	int maiorX = 0, menorX = MAIS_INFINITO, maiorY = 0, menorY = MAIS_INFINITO;
	int pontoFornEsqX, pontoFornEsqY, pontoFornDirX, pontoFornDirY, pontoConsEsqX, pontoConsEsqY, pontoConsDirX, pontoConsDirY;
	for (int i = 0; i < cardClusterFornEsq; i++){
		tmpX = G->getPosX(clusterFornEsq[i]);
		tmpY = G->getPosY(clusterFornEsq[i]);
		if (tmpX > maiorX){
			maiorX = tmpX;
		}
		if (tmpX < menorX){
			menorX = tmpX;
		}
		if (tmpY > maiorY){
			maiorY = tmpY;
		}
		if (tmpY < menorY){
			menorY = tmpY;
		}
	}
	pontoFornEsqX = (maiorX + menorX) / 2;
	pontoFornEsqY = (maiorY + menorY) / 2;

	//CLUSTER FORNECEDOR DIREITA
	maiorX = 0, menorX = MAIS_INFINITO, maiorY = 0, menorY = MAIS_INFINITO;
	for (int i = 0; i < cardClusterFornDir; i++){
		tmpX = G->getPosX(clusterFornDir[i]);
		tmpY = G->getPosY(clusterFornDir[i]);
		if (tmpX > maiorX){
			maiorX = tmpX;
		}
		if (tmpX < menorX){
			menorX = tmpX;
		}
		if (tmpY > maiorY){
			maiorY = tmpY;
		}
		if (tmpY < menorY){
			menorY = tmpY;
		}
	}
	pontoFornDirX = (maiorX + menorX) / 2;
	pontoFornDirY = (maiorY + menorY) / 2;

	//CLUSTER CONSUMIDOR ESQUERDA
	maiorX = 0, menorX = MAIS_INFINITO, maiorY = 0, menorY = MAIS_INFINITO;
	for (int i = 0; i < cardClusterConsEsq; i++){
		tmpX = G->getPosX(clusterConsEsq[i]);
		tmpY = G->getPosY(clusterConsEsq[i]);
		if (tmpX > maiorX){
			maiorX = tmpX;
		}
		if (tmpX < menorX){
			menorX = tmpX;
		}
		if (tmpY > maiorY){
			maiorY = tmpY;
		}
		if (tmpY < menorY){
			menorY = tmpY;
		}
	}
	pontoConsEsqX = (maiorX + menorX) / 2;
	pontoConsEsqY = (maiorY + menorY) / 2;

	//CLUSTER CONSUMIDOR DIREITA
	maiorX = 0, menorX = MAIS_INFINITO, maiorY = 0, menorY = MAIS_INFINITO;
	for (int i = 0; i < cardClusterConsDir; i++){
		tmpX = G->getPosX(clusterConsDir[i]);
		tmpY = G->getPosY(clusterConsDir[i]);
		if (tmpX > maiorX){
			maiorX = tmpX;
		}
		if (tmpX < menorX){
			menorX = tmpX;
		}
		if (tmpY > maiorY){
			maiorY = tmpY;
		}
		if (tmpY < menorY){
			menorY = tmpY;
		}
	}
	pontoConsDirX = (maiorX + menorX) / 2;
	pontoConsDirY = (maiorY + menorY) / 2;

	float distDep_FornEsq = sqrt (pow ((G->getPosX(0) - pontoFornEsqX), 2) + pow((G->getPosY(0) - pontoFornEsqY), 2));
	float distDep_FornDir = sqrt (pow ((G->getPosX(0) - pontoFornDirX), 2) + pow((G->getPosY(0) - pontoFornDirY), 2));
	float distDep_ConsEsq = sqrt (pow ((G->getPosX(0) - pontoConsEsqX), 2) + pow((G->getPosY(0) - pontoConsEsqY), 2));
	float distDep_ConsDir = sqrt (pow ((G->getPosX(0) - pontoConsDirX), 2) + pow((G->getPosY(0) - pontoConsDirY), 2));
	float distFornEsq_FornDir = sqrt (pow ((pontoFornEsqX - pontoFornDirX), 2) + pow ((pontoFornEsqY - pontoFornDirY), 2));
	float distConsEsq_ConsDir = sqrt (pow ((pontoConsEsqX - pontoConsDirX), 2) + pow ((pontoConsEsqY - pontoConsDirY), 2));
	float dist2Clusters, dist3Clusters, distMedia;

	if (veiculoTrocaLado){
		dist2Clusters = 2*distDep_FornEsq + 2*distDep_ConsDir;
	}else{
		dist2Clusters = 2*distDep_FornEsq + 2*distDep_ConsEsq;
	}
	dist3Clusters = 2*distDep_FornEsq + distDep_ConsEsq + distConsEsq_ConsDir + distDep_ConsDir;
	if (veiculoTrocaLado){
		if ((2*distDep_ConsDir + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir) < dist3Clusters){
			dist3Clusters = (2*distDep_ConsDir + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir);
		}
	}else{
		if ((2*distDep_ConsEsq + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir) < dist3Clusters){
			dist3Clusters = (2*distDep_ConsEsq + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir);
		}
	}
	distMedia = dist3Clusters - dist2Clusters;

	if (veiculoTrocaLado){
		dist2Clusters = 2*distDep_FornDir + 2*distDep_ConsEsq;
	}else{
		dist2Clusters = 2*distDep_FornDir + 2*distDep_ConsDir;
	}
	dist3Clusters = 2*distDep_FornDir + distDep_ConsEsq + distConsEsq_ConsDir + distDep_ConsDir;
	if (veiculoTrocaLado){
		if ((2*distDep_ConsEsq + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir) < dist3Clusters){
			dist3Clusters = (2*distDep_ConsEsq + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir);
		}
	}else{
		if ((2*distDep_ConsDir + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir) < dist3Clusters){
			dist3Clusters = (2*distDep_ConsDir + distDep_FornEsq + distFornEsq_FornDir + distDep_FornDir);
		}
	}
	distMedia += dist3Clusters - dist2Clusters;
	cout << "T (" << (totalInsercoesV1 + totalInsercoesV2) / 2 << "), D (" << distMedia / 2 << ") : raio elimina vizinho = " << limite << "\n  ";
}


void calculaPropriedade9(Grafo* G, int nCommodities){
	int cargaAcum = 0;
	for (int i = 1; i <= nCommodities; ++i){
		cargaAcum += G->getPesoCommodity(i);
	}
	cout << "CargaMerc (" << cargaAcum << "), CapacVeic (" << G->getCapacVeiculo() << 
					") : Ocupacao (" << (cargaAcum / (float)G->getCapacVeiculo()) * 100 << "%)\n";
}


void calculaPropriedade10(Grafo* G, int nCommodities){
	int valorMaiorX = 0, valorMenorX = MAIS_INFINITO, vertMaiorX, vertMenorX, tmpX;

	//VERTICES EXTREMOS DOS FORNECEDORES
	for (int i = 1; i <= nCommodities; i++){
		tmpX = G->getPosX(i);
		if (tmpX > valorMaiorX){
			valorMaiorX = tmpX;
			vertMaiorX = i;
		}
		if (tmpX < valorMenorX){
			valorMenorX = tmpX;
			vertMenorX = i;
		}
	}
	float distAcumulada = G->getCustoAresta(0, vertMenorX) + G->getCustoAresta(0, vertMaiorX);

	//VERTICES EXTREMOS DOS CONSUMIDORES
	valorMaiorX = 0, valorMenorX = MAIS_INFINITO;
	for (int i = nCommodities+1; i <= 2*nCommodities; i++){
		tmpX = G->getPosX(i);
		if (tmpX > valorMaiorX){
			valorMaiorX = tmpX;
			vertMaiorX = i;
		}
		if (tmpX < valorMenorX){
			valorMenorX = tmpX;
			vertMenorX = i;
		}
	}
	distAcumulada += G->getCustoAresta(0, vertMenorX) + G->getCustoAresta(0, vertMaiorX);
	cout << "distAcumulada (" << distAcumulada << ")" << endl << endl;
}
