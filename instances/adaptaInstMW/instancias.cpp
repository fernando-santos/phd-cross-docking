#include "instancias.h"

void imprimeVertices(Vertice **vv){
	for (int i = 0; i <= 100; i++){
		cout << i << " " << vv[i]->posX << " " << vv[i]->posY << endl;
	}
}

int main(int argc, char **argv){

	if (argc != 5){
		cout << "Parametros necessarios:\n  (1) Arquivo de instancia original Solomon\n"
		     << "  (2) Numero de Commodities na rede\n  (3) Capacidade do veiculo\n  (4) Nome do arquivo de saida\n\n";
		return 0;
	}
	srand(time(0));

	fstream inFile;
	inFile.open(argv[1], ios::in);

	fstream outFile;
	outFile.open(argv[4], ios::out);

	int nCommodities = atoi(argv[2]);

	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 9; i++){
		char buffer[200];
		inFile.getline(buffer, 200, '\n');

		if (i == 4){
			outFile << "  25         " << argv[3] << endl;
		}else{
			outFile << buffer << endl;
		}
	}

	//Lê todos os dados da instancia e armazena no vetor para as alteracoes necessarias
	int tmp;
	Vertice** v = new Vertice*[101];
	for (int i = 0; i <= 100; i++){
		v[i] = new Vertice();
		inFile >> tmp >> v[i]->posX >> v[i]->posY >> v[i]->demanda >> v[i]->tInicio >> v[i]->tFim >> v[i]->service;
	}
	inFile.close();

	//Insere o vertice deposito
	outFile << "    " << 0 << "      " << v[0]->posX << "         " << v[0]->posY << "         "  << v[0]->demanda 
			<< "         " << v[0]->tInicio << "        " << v[0]->tFim << "         " << v[0]->service << endl;

	//ordena os vertices em funcao de sua posicao Y
	int minPosY;
	Vertice* aux;
	for (int i = 1; i <= 100; i++){
		minPosY = i;
		for (int j = i+1; j <= 100; ++j){
			if (v[j]->posY < v[minPosY]->posY){
				minPosY = j;
			}
		}
		aux = v[i];
		v[i] = v[minPosY];
		v[minPosY] = aux;
	}

	//aloca memoria para o vetor que armazenara todas as demandas dos vertices escolhidos como fornecedores,
	//para que estas sejam as mesmas dos vertices consumidores, assegurando a viabilidade das solucoes
	int *demandas = new int[nCommodities];

	//Uma vez que os vertices estao ordenados pela cordenada Y, seleciona-se aleatoriamente nCommodities vertices para serem fornecedores
	//entre os (nCommodities + 10) vertices de menor cordenada Y
	int limite = (nCommodities <= 40) ? (nCommodities+10) : 50;
	for (int i = 1; i <= nCommodities; ++i){
		do{
			tmp = (rand() % limite) + 1;
		}while(tmp < i);

		//tmp armazena o indice do vertice que sera inserido no arquivo resultante
		aux = v[tmp];
		v[tmp] = v[i];
		v[i] = aux;
		demandas[i-1] = v[i]->demanda;

		outFile << "    " << i << "      " << v[i]->posX << "         " << v[i]->posY << "         "  << v[i]->demanda 
			<< "         " << v[i]->tInicio << "        " << v[i]->tFim << "         " << v[i]->service << endl;
	}


	//O mesmo procedimento eh feito para selecionar os consumidores cujas cordenadas sejam as mais distantes
	limite = (nCommodities <= 40) ? (nCommodities+10) : 50;
	int j = 0;
	for (int i = 100; i > (100-nCommodities); --i){
		do{
			tmp = 100 - (rand() % limite);
		}while(tmp > i);

		//tmp armazena o indice do vertice que sera inserido no arquivo resultante
		aux = v[tmp];
		v[tmp] = v[i];
		v[i] = aux;

		outFile << "    " << nCommodities+(101-i) << "      " << v[i]->posX << "         " << v[i]->posY << "         "  << demandas[j++]
			<< "         " << v[i]->tInicio << "        " << v[i]->tFim << "         " << v[i]->service << endl;
	}

	outFile << endl;
	outFile.close();
}
