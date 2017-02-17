#include "ListaNosAtivos.h"

ListaNosAtivos::ListaNosAtivos(){
	for (int i = 0; i < 10; ++i) cabeca[i] = menorNo[i] = antecessorMenorNo[i] = NULL;
}

void ListaNosAtivos::insereNo(ptrNo p){
	int indiceHash = (p->getIndiceNo() % 10);
	p->setProx(cabeca[indiceHash]);
	cabeca[indiceHash] = p;

	if ((menorNo[indiceHash] == NULL) || (p->getLimiteDual() < menorNo[indiceHash]->getLimiteDual())){
		//Caso o limite dual do No inserido seja menor que o limite do No apontado por menorNo, atualiza este ponteiro
		menorNo[indiceHash] = p;
		antecessorMenorNo[indiceHash] = NULL;
	}else if (antecessorMenorNo[indiceHash] == NULL){
		antecessorMenorNo[indiceHash] = cabeca[indiceHash];
	}
}

ptrNo ListaNosAtivos::retornaProximo(){
	int indiceVetor;
	ptrNo aux, auxAnt, retorno;
	float menorValor = MAIS_INFINITO;
	for (int i = 0; i < 10; ++i){
		if ((menorNo[i] != NULL) && (menorNo[i]->getLimiteDual() < menorValor)){
			menorValor = menorNo[i]->getLimiteDual();
			indiceVetor = i;
		}
	}
	if (menorValor == MAIS_INFINITO) return NULL;
	
	if (antecessorMenorNo[indiceVetor] == NULL){ //significa q menorNo[indiceVetor] == cabeca[indiceVetor]
		retorno = menorNo[indiceVetor];
		cabeca[indiceVetor] = menorNo[indiceVetor]->getProx();
	}else{ //significa que menorNo[indiceVetor] aponta para um No no meio da lista
		retorno = menorNo[indiceVetor];
		antecessorMenorNo[indiceVetor]->setProx(menorNo[indiceVetor]->getProx()); 
	}

	//passa pela lista para encontrar o menor elemento e armazenar em menorNo[indiceVetor]
	aux = cabeca[indiceVetor];
	if (aux != NULL){
		menorNo[indiceVetor] = aux;
		antecessorMenorNo[indiceVetor] = NULL;
		menorValor = menorNo[indiceVetor]->getLimiteDual();
		auxAnt = aux;
		aux = aux->getProx();
		while (aux != NULL){
			if (aux->getLimiteDual() < menorValor){
				antecessorMenorNo[indiceVetor] = auxAnt;
				menorValor = aux->getLimiteDual();
				menorNo[indiceVetor] = aux;
			}
			auxAnt = aux;
			aux = aux->getProx();
		}
	}else{
		menorNo[indiceVetor] = antecessorMenorNo[indiceVetor] = NULL;
	}

	return retorno;
}

void ListaNosAtivos::podaPorLimiteDual(float limitePrimal){
	for (int i = 0; i < 10; ++i){
		ptrNo no = cabeca[i];
		ptrNo anterior = NULL;
		while(no != NULL){
			if (no->getLimiteDual() >= limitePrimal){
				//caso menorNo[i] aponte para um No com custo dual superior ao primal 
				//significa que ao final deste laco todos os Nos da lista serao podados
				if (no == menorNo[i]){
					menorNo[i] = NULL;
					antecessorMenorNo[i] = NULL;
				}

				if (no == antecessorMenorNo[i]){
					antecessorMenorNo[i] = anterior;
				}

				if (anterior != NULL){					
					anterior->setProx(no->getProx());
					delete no;
					no = anterior->getProx();
				}else{
					cabeca[i] = no->getProx();
					delete no;
					no = cabeca[i];
				}
			}else{
				anterior = no;
				no = no->getProx();
			}
		}
	}
}

bool ListaNosAtivos::vazia(){
	bool result = true;
	for (int i = 0; i < 10; ++i) result = (result && (cabeca[i] == NULL));
	return result;
}

void ListaNosAtivos::imprimeLista(int i){
	ptrNo aux = cabeca[i];
	while (aux != NULL){
		aux->imprimir();
		aux = aux->getProx();
	}
}
