#include <stdlib.h>
#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char** argv){
	if (argc != 3){
		cout << "Parametros esperados:\n  (1) Arquivo de Instancia\n  (2) Numero de Vertices (Fornecedores + Consumidores + Deposito)\n\n";
		return 0;
	}
	int posX, posY, tmp, numVertices = atoi(argv[2]);

	//Abre o arquivo de instancia para ser processado e montar o grafo
	fstream inFile;
	inFile.open(argv[1], ios::in);

	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 4; ++i){
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}	
	inFile >> tmp >> tmp;
	//loop para retirar o cabecalho do arquivo
	for (int i = 0; i < 5; ++i){
		char buffer[200];
		inFile.getline(buffer, 200, '\n');
	}

	//Lê todos os dados da instancia e armazena no vetor para as alteracoes necessarias
	cout << "@(x) ";
	inFile >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp; //desconsidera o deposito
	for (int i = 1; i < numVertices-1; ++i){
		inFile >> tmp >> posX >> posY >> tmp >> tmp >> tmp >> tmp;
		cout << "sqrt ((" << posX << " - x(1)).^2 + (" << posY << " - x(2)).^2) + ";
	}
	inFile >> tmp >> posX >> posY >> tmp >> tmp >> tmp >> tmp;
	cout << "sqrt ((" << posX << " - x(1)).^2 + (" << posY << " - x(2)).^2)\n";
}
