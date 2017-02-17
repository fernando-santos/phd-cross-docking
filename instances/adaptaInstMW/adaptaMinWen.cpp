#include "instancias.h"

float maxX = -100000, minX = 100000, maxY = -100000, minY = 100000;

int quadrante(float y)
{
	float intervalo = (maxY - minY) / 5;
	for ( int i = 1; i <= 5; ++i )
	{
		if ( y <= ( minY + i*intervalo ) ) return i;
	}
	cout << "y = " << y << " --- problemas na funcao quadrante!" << endl;
}

int main(int argc, char **argv){

	if (argc != 6)
	{
		cout << "Parametros necessarios:\n"
			 << "  (1) Arquivo de instancia Min Wen (DADOS - COORDENADAS E DEMANDAS)\n"
			 << "  (2) Arquivo de instancia Min Wen (JANELAS DE TEMPO)\n"
		     << "  (3) Numero de Commodities da instancia Min Wen\n"
		     << "  (4) Numero de Commodities a serem geradas na nova instancia\n"
		     << "  (5) Nome do arquivo de saida\n\n";
		return 0;
	}
	srand(time(0));
	fstream entradaDados;
	entradaDados.open(argv[1], ios::in);

	fstream entradaJanelasTempo;
	entradaJanelasTempo.open(argv[2], ios::in);

	fstream arquivoSaida;
	arquivoSaida.open(argv[5], ios::out);

	float dist;
	int tmp, capacidade;
	int nCommodities = atoi(argv[3]);
	int nCommoditiesNovaInst = atoi(argv[4]);

	VerticeMW deposito;
	VerticeMW fornecedores[nCommodities+1];
	VerticeMW consumidores[nCommodities+1];

	entradaDados >> tmp >> capacidade >> deposito.tInicio >> deposito.tFim >> tmp >> tmp >> tmp;
	entradaDados >> tmp >> deposito.posX >> deposito.posY >> tmp >> tmp >> tmp;
	deposito.demanda = 0;
	deposito.posX /= 1000;
	deposito.posY /= 1000;
	deposito.tInicio *= 60;
	deposito.tFim *= 60;

	for (int i = 1; i <= nCommodities; ++i)
	{
		entradaDados >> tmp >> consumidores[i].posX >> consumidores[i].posY >> fornecedores[i].posX >> fornecedores[i].posY >> tmp;
		entradaJanelasTempo >> consumidores[i].tInicio >> consumidores[i].tFim >> fornecedores[i].tInicio >> fornecedores[i].tFim;
		fornecedores[i].demanda = consumidores[i].demanda = tmp;
		fornecedores[i].posX /= 1000;
		fornecedores[i].posY /= 1000;
		consumidores[i].posX /= 1000;
		consumidores[i].posY /= 1000;
		fornecedores[i].cluster = 0;
		consumidores[i].cluster = 0;

		if ( fornecedores[i].posX > maxX ) maxX = fornecedores[i].posX;
		if ( fornecedores[i].posX < minX ) minX = fornecedores[i].posX;
		if ( consumidores[i].posX > maxX ) maxX = consumidores[i].posX;
		if ( consumidores[i].posX < minX ) minX = consumidores[i].posX;

		if ( fornecedores[i].posY > maxY ) maxY = fornecedores[i].posY;
		if ( fornecedores[i].posY < minY ) minY = fornecedores[i].posY;
		if ( consumidores[i].posY > maxY ) maxY = consumidores[i].posY;
		if ( consumidores[i].posY < minY ) minY = consumidores[i].posY;
	}

	arquivoSaida << "MW\n\nVEHICLE\nNUMBER     CAPACITY\n  25         " << capacidade << "\n\nCUSTOMER\n";
	arquivoSaida << "CUST NO.   XCOORD.    YCOORD.    DEMAND   READY TIME   DUE DATE   SERVICE TIME\n\n";
	arquivoSaida << "    0      " << deposito.posX << "     " << deposito.posY << "       0         "
				 << deposito.tInicio << "        " << deposito.tFim << "         0\n";

	float recuoXForn = ( maxX - deposito.posX ) / 3;
	float recuoXCons = ( deposito.posX - minX ) / 3;
	int raio, carga, numVertCluster, numForn = 0, numCons = 0;
	int numClusters = 0, totalClusters = ( nCommoditiesNovaInst / 5 );

	while ( numClusters < totalClusters )
	{
		tmp = ( rand() % nCommodities ) + 1; //primeiro no a ser incluido no cluster
		if ( ( ( ( ( numClusters % 4 ) == 0 ) && ( fornecedores[tmp].posY > ( deposito.posY + 10 ) ) ) || //quadrante superior
			 ( ( numClusters % 4 ) == 1 ) && ( fornecedores[tmp].posY < ( deposito.posY - 10 ) ) ) && //quadrante inferior
			 ( ( fornecedores[tmp].cluster == 0 ) && ( fornecedores[tmp].posX > ( deposito.posX + recuoXForn ) ) ) && 
			 ( quadrante(fornecedores[tmp].posY) != 5 ) )
		{
			++numForn;
			++numClusters;
			fornecedores[tmp].cluster = numClusters;
			carga = fornecedores[tmp].demanda; numVertCluster = 1; raio = 4;
			while ( ( carga < capacidade ) && ( numVertCluster < 4 ) )
			{
				for ( int i = 1; i <= nCommodities; ++i )
				{
					dist = sqrt( pow( ( fornecedores[tmp].posX - fornecedores[i].posX ), 2 ) + pow( ( fornecedores[tmp].posY - fornecedores[i].posY ), 2 ) );
					if ( ( fornecedores[i].cluster == 0 ) && ( dist > 3 ) && ( dist < raio ) )
					{
						++numForn; ++numVertCluster;
						carga += fornecedores[i].demanda;
						fornecedores[i].cluster = numClusters;
						if ( ( carga >= capacidade ) || ( numVertCluster >= 4 ) ) break;
					}
				}
				++raio;
			}
			
			//assim que um cluster de fornecedores foi incluido, assegura seus respectivos consumidores
			numVertCluster = 1;
			for ( int i = 1; i <= nCommodities; ++i)
			{
				if ( fornecedores[i].cluster == numClusters ) //inclui um consumidor associado ao fornecedor do cluster
				{
					while ( true )
					{
						tmp = ( rand() % nCommodities ) + 1;
						if ( ( consumidores[tmp].cluster == 0 ) && ( consumidores[tmp].posX < ( deposito.posX - recuoXCons ) ) )
						{
							if ( ( numVertCluster == 1 ) && ( quadrante( consumidores[tmp].posY ) == 1 ) )
							{
								consumidores[tmp].demanda = fornecedores[i].demanda;
								consumidores[tmp].cluster = numClusters;
								++numVertCluster;
								break;
							}
							if ( ( numVertCluster == 2 ) && ( quadrante( consumidores[tmp].posY ) == 4 ) )
							{
								consumidores[tmp].demanda = fornecedores[i].demanda;
								consumidores[tmp].cluster = numClusters;
								++numVertCluster;
								break;
							}
							if ( ( numVertCluster != 1 ) && ( numVertCluster != 2 ) )
							{
								consumidores[tmp].demanda = fornecedores[i].demanda;
								consumidores[tmp].cluster = numClusters;
								++numVertCluster;
								break;
							}
						}
					}
				}
			}
		}
		else if ( ( ( ( ( numClusters % 4 ) == 2 ) && ( consumidores[tmp].posY > ( deposito.posY + 10 ) ) ) || 
				 ( ( numClusters % 4 ) == 3 ) && ( consumidores[tmp].posY < ( deposito.posY - 10 ) ) ) &&
				 ( ( consumidores[tmp].cluster == 0 ) && ( consumidores[tmp].posX < ( deposito.posX - recuoXCons ) ) ) &&
				 ( quadrante(fornecedores[tmp].posY) != 5 ) )
		{
			++numCons;
			++numClusters;
			consumidores[tmp].cluster = numClusters;
			carga = consumidores[tmp].demanda; numVertCluster = 1; raio = 4;
			while ( ( carga < capacidade ) && ( numVertCluster < 4 ) )
			{
				for ( int i = 1; i <= nCommodities; ++i )
				{
					dist = sqrt( pow( ( consumidores[tmp].posX - consumidores[i].posX ), 2 ) + pow( ( consumidores[tmp].posY - consumidores[i].posY ), 2 ) );
					if ( ( consumidores[i].cluster == 0 ) && ( dist > 3 ) && ( dist < raio ) )
					{
						++numCons; ++numVertCluster;
						carga += consumidores[i].demanda;
						consumidores[i].cluster = numClusters;
						if ( ( carga >= capacidade ) || ( numVertCluster >= 4 ) ) break;
					}
				}
				++raio;
			}

			//assim que um cluster de fornecedores foi incluido, assegura seus respectivos consumidores
			numVertCluster = 1;
			for ( int i = 1; i <= nCommodities; ++i)
			{
				if ( consumidores[i].cluster == numClusters ) //inclui um consumidor associado ao fornecedor do cluster
				{
					while ( true )
					{
						tmp = ( rand() % nCommodities ) + 1;
						if ( ( fornecedores[tmp].cluster == 0 ) && ( fornecedores[tmp].posX > ( deposito.posX + recuoXForn ) ) )
						{
							if ( ( numVertCluster == 1 ) && ( quadrante( fornecedores[tmp].posY ) == 2 ) )
							{
								fornecedores[tmp].demanda = consumidores[i].demanda;
								fornecedores[tmp].cluster = numClusters;
								++numVertCluster;
								break;
							}
							if ( ( numVertCluster == 2 ) && ( quadrante( fornecedores[tmp].posY ) == 3 ) )
							{
								fornecedores[tmp].demanda = consumidores[i].demanda;
								fornecedores[tmp].cluster = numClusters;
								++numVertCluster;
								break;
							}
							if ( numVertCluster > 2 )
							{
								fornecedores[tmp].demanda = consumidores[i].demanda;
								fornecedores[tmp].cluster = numClusters;
								++numVertCluster;
								break;
							}
						}
					}
				}
			}
		}
	}


/*	//completa o numero de requisicoes da instancia com requisicoes aleatorias
	while ( numReqs < nCommoditiesNovaInst )
	{
		tmp = ( rand() % nCommodities ) + 1;
		if ( ( !fornecedores[tmp].escolhido ) && ( fornecedores[tmp].posX > ( deposito.posX + recuoForn ) ) && 
												 ( consumidores[tmp].posX < ( deposito.posX - recuoCons ) ) )
		{
			fornecedores[tmp].escolhido = consumidores[tmp].escolhido = true;
			++numReqs;
		}
	}
*/

	int nReqs = 1;
	int *dem = new int[nCommoditiesNovaInst+1];
	for ( int c = 1; c <= numClusters; ++c )
	{
		for (int i = 1; i <= nCommodities; i++)
		{
			if ( fornecedores[i].cluster == c )
			{
				arquivoSaida << "    " << fornecedores[i].cluster << "      " << fornecedores[i].posX << "     " << fornecedores[i].posY 
							 << "       " << fornecedores[i].demanda << "        " << fornecedores[i].tInicio 
							 << "         " << fornecedores[i].tFim << "         0\n";
				dem[nReqs] = fornecedores[i].demanda;
				++nReqs;
			}
		}
	}

	nReqs = 1;
	for ( int c = 1; c <= numClusters; ++c )
	{
		for (int i = 1; i <= nCommodities; i++)
		{
			if ( consumidores[i].cluster == c )
			{
				arquivoSaida << "    " << consumidores[i].cluster << "      " << consumidores[i].posX << "     " << consumidores[i].posY 
							 << "       " << dem[nReqs] << "        " << consumidores[i].tInicio 
							 << "         " << consumidores[i].tFim << "         0\n";
				++nReqs;
			}
		}
	}

	delete [] dem;
	arquivoSaida << endl;
	arquivoSaida.close();
	entradaDados.close();
	entradaJanelasTempo.close();
}

