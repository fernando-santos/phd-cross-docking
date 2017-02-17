#include "instancias.h"

float maxX = -100000, minX = 100000, maxYF = -100000, minYF = 100000, maxYC = -100000, minYC = 100000;

int quadranteF(float y)
{
	float intervalo = (maxYF - minYF) / 5;
	for ( int i = 1; i <= 5; ++i )
	{
		if ( y <= ( minYF + i*intervalo ) ) return i;
	}
}

int quadranteC(float y)
{
	float intervalo = (maxYC - minYC) / 5;
	for ( int i = 1; i <= 5; ++i )
	{
		if ( y <= ( minYC + i*intervalo ) ) return i;
	}
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

	VerticeMW deposito, aux;
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
		if ( consumidores[i].posX < minX ) minX = consumidores[i].posX;

		if ( fornecedores[i].posY > maxYF ) maxYF = fornecedores[i].posY;
		if ( fornecedores[i].posY < minYF ) minYF = fornecedores[i].posY;
		if ( consumidores[i].posY > maxYC ) maxYC = consumidores[i].posY;
		if ( consumidores[i].posY < minYC ) minYC = consumidores[i].posY;
	}

	arquivoSaida << "MW\n\nVEHICLE\nNUMBER     CAPACITY\n  25         " << capacidade << "\n\nCUSTOMER\n";
	arquivoSaida << "CUST NO.   XCOORD.    YCOORD.    DEMAND   READY TIME   DUE DATE   SERVICE TIME\n\n";
	arquivoSaida << "    0      " << deposito.posX << "     " << deposito.posY << "       0         "
				 << deposito.tInicio << "        " << deposito.tFim << "         0\n";

	float recuoXForn = ( maxX - deposito.posX ) / 3;
	float recuoXCons = ( deposito.posX - minX ) / 2;
	int raio, carga, numVertCluster, numReqs = 0;	

	//cria o PRIMEIRO cluster ( canto inferior direito )
	int quad1 = 1, numIt = 1;
	do
	{
		tmp = ( rand() % nCommodities ) + 1; //indice do primeiro no a ser incluido no cluster
		if ( numIt == 10000 ) quad1 = 2;
		if ( ( numIt % 20000 ) == 0 ) recuoXForn /= 2;
		++numIt;
	}
	while( ( fornecedores[tmp].posX < ( deposito.posX + recuoXForn ) ) || ( quadranteF( fornecedores[tmp].posY ) != quad1 ) );

	++numReqs;
	fornecedores[tmp].cluster = 1;
	carga = fornecedores[tmp].demanda; numVertCluster = 1; raio = 4;
	
	while ( ( carga < capacidade ) && ( numVertCluster < 4 ) )
	{
		for ( int i = 1; i <= nCommodities; ++i )
		{
			dist = sqrt( pow( ( fornecedores[tmp].posX - fornecedores[i].posX ), 2 ) + pow( ( fornecedores[tmp].posY - fornecedores[i].posY ), 2 ) );
			if ( ( fornecedores[i].cluster == 0 ) && ( dist > 3 ) && ( dist < raio ) )
			{
				++numReqs; ++numVertCluster;
				fornecedores[i].cluster = 1;
				carga += fornecedores[i].demanda;
				if ( ( carga >= capacidade ) || ( numVertCluster >= 4 ) ) break;
			}
		}
		++raio;
	}

	//associa os respectivos consumidores ao cluster de fornecedores criado
	numVertCluster = 1;
	for ( int i = 1; i <= nCommodities; ++i)
	{
		if ( fornecedores[i].cluster == 1 ) //inclui um consumidor associado ao fornecedor do cluster
		{
			while ( true )
			{
				tmp = ( rand() % nCommodities ) + 1;
				if ( ( consumidores[tmp].cluster == 0 ) && ( consumidores[tmp].posX < ( deposito.posX - recuoXCons ) ) )
				{
					if ( ( numVertCluster == 1 ) && ( quadranteC( consumidores[tmp].posY ) == 1 ) )
					{
						aux = consumidores[tmp];
						consumidores[tmp] = consumidores[i];
						consumidores[i] = aux;
						consumidores[i].demanda = fornecedores[i].demanda;
						consumidores[i].cluster = 1;
						++numVertCluster;
						break;
					}
					if ( ( numVertCluster == 2 ) && ( quadranteC( consumidores[tmp].posY ) == 4 ) )
					{
						aux = consumidores[tmp];
						consumidores[tmp] = consumidores[i];
						consumidores[i] = aux;
						consumidores[i].demanda = fornecedores[i].demanda;
						consumidores[i].cluster = 1;
						++numVertCluster;
						break;
					}
					if ( ( numVertCluster > 2 ) && ( quadranteC( consumidores[tmp].posY ) < 5 ) )
					{
						aux = consumidores[tmp];
						consumidores[tmp] = consumidores[i];
						consumidores[i] = aux;
						consumidores[i].demanda = fornecedores[i].demanda;
						consumidores[i].cluster = 1;
						++numVertCluster;
						break;
					}
				}
			}
		}
	}

	//cria o SEGUNDO cluster ( canto superior direito )
	recuoXForn = ( maxX - deposito.posX ) / 3;
	int quad2 = 5; numIt = 1;
	do
	{
		tmp = ( rand() % nCommodities ) + 1; //indice do primeiro no a ser incluido no cluster
		if ( numIt == 10000 ) quad2 = 4;
		if ( ( numIt % 20000 ) == 0 ) recuoXForn /= 2;
		++numIt;
	}
	while( ( fornecedores[tmp].cluster != 0 ) || ( fornecedores[tmp].posX < ( deposito.posX + recuoXForn ) ) || ( quadranteF( fornecedores[tmp].posY ) != quad2 ) );

	++numReqs;
	fornecedores[tmp].cluster = 2;
	carga = fornecedores[tmp].demanda; numVertCluster = 1; raio = 4;
	while ( ( carga < capacidade ) && ( numVertCluster < 4 ) )
	{
		for ( int i = 1; i <= nCommodities; ++i )
		{
			dist = sqrt( pow( ( fornecedores[tmp].posX - fornecedores[i].posX ), 2 ) + pow( ( fornecedores[tmp].posY - fornecedores[i].posY ), 2 ) );
			if ( ( fornecedores[i].cluster == 0 ) && ( dist > 3 ) && ( dist < raio ) )
			{
				++numReqs; ++numVertCluster;
				fornecedores[i].cluster = 2;
				carga += fornecedores[i].demanda;
				if ( ( carga >= capacidade ) || ( numVertCluster >= 4 ) ) break;
			}
		}
		++raio;
	}

	//associa os respectivos consumidores ao cluster de fornecedores criado
	numVertCluster = 1;
	for ( int i = 1; i <= nCommodities; ++i)
	{
		if ( fornecedores[i].cluster == 2 ) //inclui um consumidor associado ao fornecedor do cluster
		{
			while ( true )
			{
				tmp = ( rand() % nCommodities ) + 1;
				if ( ( consumidores[tmp].cluster == 0 ) && ( consumidores[tmp].posX < ( deposito.posX - recuoXCons ) ) )
				{
					if ( ( numVertCluster == 1 ) && ( quadranteC( consumidores[tmp].posY ) == 1 ) )
					{
						aux = consumidores[tmp];
						consumidores[tmp] = consumidores[i];
						consumidores[i] = aux;
						consumidores[i].demanda = fornecedores[i].demanda;
						consumidores[i].cluster = 2;
						++numVertCluster;
						break;
					}
					if ( ( numVertCluster == 2 ) && ( quadranteC( consumidores[tmp].posY ) == 4 ) )
					{
						aux = consumidores[tmp];
						consumidores[tmp] = consumidores[i];
						consumidores[i] = aux;
						consumidores[i].demanda = fornecedores[i].demanda;
						consumidores[i].cluster = 2;
						++numVertCluster;
						break;
					}
					if ( ( numVertCluster > 2 ) && ( quadranteC( consumidores[tmp].posY ) < 5 ) )
					{
						aux = consumidores[tmp];
						consumidores[tmp] = consumidores[i];
						consumidores[i] = aux;
						consumidores[i].demanda = fornecedores[i].demanda;
						consumidores[i].cluster = 2;
						++numVertCluster;
						break;
					}
				}
			}
		}
	}
	
	//agora procura por requisicoes que estejam espalhadas entre os clusters
	recuoXForn = ( maxX - deposito.posX ) / 3;
	while ( numReqs < nCommoditiesNovaInst )
	{
		tmp = ( rand() % nCommodities ) + 1;
		if ( ( fornecedores[tmp].cluster == 0 ) && ( quadranteF( fornecedores[tmp].posY ) > quad1 ) && 
			( quadranteF( fornecedores[tmp].posY ) < quad2 ) && ( fornecedores[tmp].posX > ( deposito.posX + recuoXForn ) ) &&
			( quadranteC( consumidores[tmp].posY ) < 5 ) && ( consumidores[tmp].posX < ( deposito.posX - recuoXCons ) ) )
		{
			++numReqs;
			consumidores[tmp].demanda = fornecedores[tmp].demanda;
			fornecedores[tmp].cluster = consumidores[tmp].cluster = 3; //cluster 3 significa requisicoes aleatorias
		}
	}

	for ( int c = 1; c <= 3; ++c )
	{
		for (int i = 1; i <= nCommodities; i++)
		{
			if ( fornecedores[i].cluster == c )
			{
				arquivoSaida << "    " << fornecedores[i].cluster << "      " << fornecedores[i].posX << "     " << fornecedores[i].posY 
							 << "       " << fornecedores[i].demanda << "        " << fornecedores[i].tInicio 
							 << "         " << fornecedores[i].tFim << "         0\n";
			}
		}
	}

	for ( int c = 1; c <= 3; ++c )
	{
		for (int i = 1; i <= nCommodities; i++)
		{
			if ( consumidores[i].cluster == c )
			{
				arquivoSaida << "    " << consumidores[i].cluster << "      " << consumidores[i].posX << "     " << consumidores[i].posY 
							 << "       " << consumidores[i].demanda << "        " << consumidores[i].tInicio 
							 << "         " << consumidores[i].tFim << "         0\n";
			}
		}
	}

	arquivoSaida << endl;
	arquivoSaida.close();
	entradaDados.close();
	entradaJanelasTempo.close();
}
