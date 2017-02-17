#include <stdlib.h>
#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char **argv){

	if (argc != 8){
		cout<< "Parametros necessarios:\n"
			<< "  (1) Numero de requisicoes\n"
			<< "  (2) Limite l do espaco euclideano ([0,l], [0,l])\n"
			<< "  (3) Separar Fornecedores e Consumidores (S (sim), N (nao))\n"
			<< "  (4) Limite INFERIOR para o a carga dos vertices\n"
			<< "  (5) Limite SUPERIOR para o a carga dos vertices\n"
		    << "  (6) Capacidade dos veiculos\n"
		    << "  (7) Nome do arquivo de saida\n\n"
			<< "OBS.: O deposito sera localizado na coordenada (l/2, l/2)\n\n";
		return 0;
	}
	if ((argv[3][0] != 'S') && (argv[3][0] != 'N')){
		cout << "Digite 'S' ou 'N' para o parametro 3" << endl;
		exit(0);
	}

	srand(time(0));
	fstream arquivoSaida;
	arquivoSaida.open(argv[7], ios::out);
	int carga, posX, posY, nRequests = atoi(argv[1]), lado = atoi(argv[2]), cargaInf = atoi(argv[4]), cargaSup = atoi(argv[5]), capacidade = atoi(argv[6]);

	arquivoSaida << "MW\n\nVEHICLE\nNUMBER     CAPACITY\n  25         " << capacidade << "\n\nCUSTOMER\n";
	arquivoSaida << "CUST NO.   XCOORD.    YCOORD.    DEMAND   READY TIME   DUE DATE   SERVICE TIME\n\n";
	arquivoSaida << "    0      " << (lado / 2) << "     " << (lado / 2) << "       0          0         0         0\n";

	for (int i = 1; i <= nRequests; i++){
		posX = (argv[3][0] == 'S') ? (rand() % (lado / 2)) : (rand() % lado);
		posY = rand() % lado;
		carga = cargaInf + (rand() % (cargaSup - cargaInf));
		arquivoSaida << "    " << i << "      " << posX << "     " << posY << "       " << carga << "         0          0         0\n";
	}
	for (int i = nRequests+1; i <= 2*nRequests; i++){
		posX = (argv[3][0] == 'S') ? ((lado / 2) + (rand() % (lado / 2))) : (rand() % lado);
		posY = rand() % lado;
		carga = cargaInf + (rand() % (cargaSup - cargaInf));
		arquivoSaida << "    " << i << "      " << posX << "     " << posY << "       " << carga << "         0          0         0\n";
	}

	arquivoSaida << endl;
	arquivoSaida.close();
}

