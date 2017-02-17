#ifndef LISTANOSATIVOS_H_
#define LISTANOSATIVOS_H_
#include "NoArvore.h"

class ListaNosAtivos{
	private:
		//os Nos serao armazenados em um hash simples, que os armazenara ao fazer 'indiceNo % 10'
		ptrNo cabeca[10], menorNo[10], antecessorMenorNo[10];

	public:
		bool vazia();
		ListaNosAtivos();
		void insereNo(ptrNo);
		ptrNo retornaProximo();
		void imprimeLista(int);
		void podaPorLimiteDual(float);
};

#endif
