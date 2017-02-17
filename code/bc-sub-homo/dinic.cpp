#include "dinic.h"

void Barf( const char *s )
{
	fprintf(stderr, "%s\n", s);
	exit(-1);
}

void InitGraph( GRAPH * G )
{
	int i;

	for (i = 0; i < MAX_N; i++)
	{
		G->A[i] = (EDGE2 *) 0;
		G->V[i] = FALSE;
	}
	G->size = 0;
	G->max_v = -1;
	G->edge_count = 0;
}

void AddVertex( int v, GRAPH * G )
{
	if (G->V[v] == TRUE)
		Barf("Vertex already present");

	G->V[v] = TRUE;
	G->size++;
	if (v > G->max_v)
		G->max_v = v;
}

EDGE2 *EdgeLookup( int v1, int v2, GRAPH *G )
{
	EDGE2 *e;

	e = G->A[v1];
	while (e != (EDGE2 *) 0){
		if (e->h == v2)
			return e;
		e = e->next;
	}
	return (EDGE2 *) 0;
}

void AddEdge( int v1, int v2, int a, GRAPH *G )
{
	EDGE2 *e1, *e2, *EdgeLookup();

	if (v1 == v2)
		Barf("No Loops");

	if ((e1 = EdgeLookup(v1, v2, G)) != (EDGE2 *) 0){
		e1->c = a;
		return;
	}

	e1 = (EDGE2 *) malloc(sizeof(EDGE2));
	e2 = (EDGE2 *) malloc(sizeof(EDGE2));

	e1->mate = e2;
	e2->mate = e1;

	e1->next = G->A[v1];
	G->A[v1] = e1;
	e1->t = v1;
	e1->h = v2;
	e1->c = a;
	e1->f = 0;

	e2->next = G->A[v2];
	G->A[v2] = e2;
	e2->t = v2;
	e2->h = v1;
	e2->c = 0;
	e2->f = 0;

	G->edge_count++;
}

GRAPH *CopyGraph( GRAPH * G1 )
{
	int i;
	EDGE2 *e;
	GRAPH *G2;

	G2 = (GRAPH *) malloc(sizeof(GRAPH));
	InitGraph(G2);

	for (i = 0; i <= G1->max_v; i++){
		if (G1->V[i] == TRUE){
			AddVertex(i, G2);
			e = G1->A[i];
			while (e != (EDGE2 *) 0){
				if (e->c > 0)
					AddEdge(i, e->h, e->c, G2);
				e = e->next;
			}
		}
	}

	return G2;
}

void InitFlow( GRAPH * G )
{
	int i;
	EDGE2 *e;

	for (i = 0; i < G->size; i++){
		e = G->A[i];
		while (e != (EDGE2 *) 0){
			e->f = 0;
			e = e->next;
		}
	}
}

int LayeredGraph(GRAPH * G, int s, int sink, GRAPH * L)
{
	int M[MAX_N], S[MAX_N], h, t, i, v, r;
	EDGE2 *e, *e1, *EdgeLookup();

	for (i = 0; i < G->size; i++)
		M[i] = -1;

	h = t = 0;
	S[0] = s;
	M[s] = 0;
	while (h >= t){
		v = S[t++];

		e = G->A[v];
		while (e != (EDGE2 *) 0){
			r = e->c - e->f;
			e1 = EdgeLookup(v, e->h, L);
			if (r > 0){
				if (M[e->h] == -1){
					M[e->h] = M[v] + 1;
					S[++h] = e->h;
					e1->c = r;
				}
				else if (M[e->h] == M[v] + 1)
					e1->c = r;
				else
					e1->c = 0;
			}
			else
				e1->c = 0;
			e = e->next;
		}
	}

	InitFlow(L);

	return M[sink] != -1;
}

int FindPath4(GRAPH * G, int v1, int v2, int P[])
{
	int M[MAX_N], S[MAX_N], sp, i, v, d;
	EDGE2 *e;
	
	for (i = 0; i < G->size; i++)
		M[i] = -1;

	sp = 0;
	S[0] = v1;
	M[v1] = v1;
	while (M[v2] == -1){
		if (sp < 0)
			return FALSE;
		v = S[sp--];

		e = G->A[v];
		while (e != (EDGE2 *) 0){
			if (M[e->h] == -1 && e->f < e->c && e->c > 0){
				M[e->h] = v;
				S[++sp] = e->h;
			}
			e = e->next;
		}
	}

	d = 1;
	v = v2;
	while (M[v] != v1){
		d++;
		v = M[v];
	}

	v = v2;
	while (d >= 0){
		P[d] = v;
		v = M[v];
		d--;
	}
	return TRUE;
}

void AddPath(GRAPH * G, int t, int * P)
{
	EDGE2 *e, *EdgeLookup();
	int i, b;

	i = 0;
	b = MAX_CAP;

	while (P[i] != t){
		e = EdgeLookup(P[i], P[i+1], G);
		b = (e->c - e->f < b) ? e->c - e->f : b;
		i++;
	}

	i = 0;
	while (P[i] != t){
		e = EdgeLookup(P[i], P[i+1], G);
		e->f += b;
		e->mate->f -= b;
		i++;
	}
}

int UpdatePath(GRAPH *G, int S[], int n, EDGE2 *current[])
{
	EDGE2 *e;
	int i, b;

	i = 0;
	b = MAX_CAP;

	for (i = 0; i < n; i++){
		e = current[S[i]];
		b = (e->c - e->f < b) ? e->c - e->f : b;
	}

	for (i = 0; i < n; i++){
		e = current[S[i]];
		e->f += b;
		e->mate->f -= b;
	}
	return b;
}

void Dinic(GRAPH * G, int v1, int v2)
{
	int done, sp, v, i, S[MAX_N], flow;
	EDGE2 *current[MAX_N], *e;

	for (i = 0; i < G->size; i++)
		current[i] = G->A[i];

	flow = 0;
	sp = 0;
	v = S[0] = v1;
	done = FALSE;

	while (done == FALSE){
		e = current[v];
		while (e != (EDGE2 *) 0 && (e->f == e->c || e->flag == FALSE)){
			e = current[v] = e->next;
		}
		if (e == (EDGE2 *) 0){
			if (v == v1)
				done = TRUE;
			else {
				v = S[--sp];
				current[v] = current[v]->next;
			}
		}
		else {
			S[++sp] = v = e->h;
			if (v == v2){
				flow += UpdatePath(G, S, sp, current);
				v = v1;
				sp = 0;
			}
		}
	}

}

int BlockingFlow(GRAPH * L, int s, int t, int fct)
{
	int P[MAX_N];

	switch (fct)
	{
		case DINIC:
			while (FindPath4(L, s, t, P))
				AddPath(L, t, P);
			break;
		case DINIC_NEW:
			Dinic(L, s, t);
			break;
// 		case KARZANOV:
// 			Karzanov(L, s, t);
// 			break;
		default:
			Barf("Unexpected case in blocking flow");
			break;
	}
}

void AddFlow(GRAPH *G, GRAPH *L)
{
	int i;
	EDGE2 *e, *e1, *EdgeLookup();

	for (i = 0; i < L->size; i++){
		e = L->A[i];
		while (e != (EDGE2 *) 0){
			if (e->f > 0){
				e1 = EdgeLookup(i, e->h, G);
				e1->f += e->f;
				e1->mate->f -= e->f;
			}
			e = e->next;
		}
	}
}

int LG2(GRAPH *G, int s, int sink)
{
	int M[MAX_N], S[MAX_N], h, t, i, v, r;
	EDGE2 *e;

	for (i = 0; i < G->size; i++){
		e = G->A[i];
		while (e != (EDGE2 *) 0){
			e->flag = FALSE;
			e = e->next;
		}
		M[i] = -1;
	}

	h = t = 0;
	S[0] = s;
	M[s] = 0;
	while (h >= t){
		v = S[t++];
		e = G->A[v];
		while (e != (EDGE2 *) 0){
			r = e->c - e->f;
			if (r > 0){
				if (M[e->h] == -1){
					M[e->h] = M[v] + 1;
					S[++h] = e->h;
					e->flag = TRUE;
				}
				else if (M[e->h] == M[v] + 1)
					e->flag = TRUE;
			}
			e = e->next;
		}
	}
	return M[sink] != -1;
}

int AugmentFlow( GRAPH * G, int s, int t, int fct, GRAPH * L)
{
	int P[MAX_N], flag;

	switch (fct) {
// 		case DFS:
// 			if ((flag = FindPath1(G, s, t, P)) == TRUE)
// 				AddPath(G, t, P);
// 			break;
// 		case BFS:
// 			if ((flag = FindPath2(G, s, t, P)) == TRUE)
// 				AddPath(G, t, P);
// 			break;
// 		case MAX_GAIN:
// 			if ((flag = FindPath3(G, s, t, P)) == TRUE)
// 				AddPath(G, t, P);
// 			break;
		case DINIC:
		case KARZANOV:
			if ((flag = LayeredGraph(G, s, t ,L)) == TRUE)
			{
				BlockingFlow(L, s, t, fct);
				AddFlow(G, L);
			}
			break;
		case DINIC_NEW:
			if ((flag = LG2(G, s, t)) == TRUE){
				BlockingFlow(G, s, t, fct);
			}
			break;
		default:
			Barf("Unknown case in augment flow");
			break;
	}

	return flag;
}

int VertexFlow(int i, GRAPH *G)
{
	EDGE2 *e;
	int flow;

	flow = 0;
	e = G->A[i];
	while (e != (EDGE2 *) 0){
		flow += e->f;
		e = e->next;
	}

	return flow;
}

int FindFlow( GRAPH * G, int s, int t, int fct )
{
	int flag, count;
	GRAPH *L, *CopyGraph();

	flag = TRUE;

	InitFlow(G);
 
	switch (fct)
	{
		case DINIC:
		case KARZANOV:
			L = CopyGraph(G);
			break;
		default:
			L = (GRAPH *) 0;
			break;
	}
  
	count = 0;

	while(flag){
		flag = AugmentFlow(G, s, t, fct, L);
		count++;
	}

// 	printf("Augmentations: %d\n",count);
	return VertexFlow(s, G);
}

void freeGraph( GRAPH * G )
{
	EDGE2 *e1, *e2;
	int i;
	
	for (i = 0; i <= G->max_v; i++)
	{
		if (G->V[i] == TRUE)
		{
			e1 = G->A[i];
			while (e1 != (EDGE2 *) 0)
			{
				e2 = e1->next;
				free(e1);
				e1 = e2;
			}
		}
	}
}
