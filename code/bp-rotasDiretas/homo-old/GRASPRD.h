#include "Grafo.h"
#include "Rota.h"
#include <vector>
#include <string.h>
using namespace std;

struct RotaRD
{
	vector < int > vertices;
	int carga;
	float custo;

	void imprimir()
	{
		int tam = vertices.size();
		printf ("(%f | %d ) [ ", custo, carga);
		for (int i = 0; i < tam; ++i) printf ("%d ", vertices[i]);
		printf ("]\n");
	}
};

class GRASPRD
{
	private:
		float alfa;
		int nRequisicoes, depArtificial;
		vector < int >* fornForaRota;
		vector < RotaRD* > rotasNegativas;
		Grafo* g;

	public:
		GRASPRD(Grafo*, int, int, float);
		~GRASPRD();
		void preencheFornecedoresForaRota(RotaRD*);
		RotaRD* solucaoInicial();
		int remocao(RotaRD*, int i = 1);
		int insercao(RotaRD*, int i = 1);
		int swap(RotaRD*);

		int run(int, float);
		void insereRotaNegativa(vector < RotaRD* >&, RotaRD*);
		Rota* getRotaConvertida(int);
};

