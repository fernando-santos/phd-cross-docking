#ifndef _DINIC_H
#define _DINIC_H

#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#define FAILURE    0
#define SUCCESS    1
#define FALSE      0
#define TRUE       1

#define MAX_N     20000

#define MAX_CAP   100000000

/* Dimacs problem types */
#define UNDEFINED        0
#define MINCOSTFLOW      1
#define MAXFLOW          2
#define ASSIGNMENT       3

typedef struct enode {
	struct enode *next;
	struct enode *mate;
	int c;
	int f;
	int h;
	int t;
	int flag;
} EDGE2;

typedef struct {
	EDGE2 *A[MAX_N];
	int V[MAX_N];
	int size;
	int max_v;
	int edge_count;
} GRAPH;

typedef struct {
	int head, tail, size;
	int *data;
} Queue;

typedef struct {
	int ptr, size;
	int *data;
} Stack;

#define DFS          1
#define BFS          2
#define MAX_GAIN     3
#define DINIC        4
#define DINIC_NEW    5
#define KARZANOV     6
#define GOLDBERG_1   7
#define GOLDBERG_2   8
#define GOLDBERG_3   9
#define GOLDBERG_4  10
#define GOLDBERG_5  11

#define INFIN 10000000

void Barf( char *s );
void InitGraph( GRAPH * G );
void AddVertex( int v, GRAPH * G);
EDGE2 *EdgeLookup( int v1, int v2, GRAPH *G );
void AddEdge( int v1, int v2, int a, GRAPH *G );
GRAPH *CopyGraph( GRAPH * G1 );
void InitFlow( GRAPH * G );
int FindPath4(GRAPH * G, int v1, int v2, int P[]);
void AddPath(GRAPH * G, int t, int * P);
int BlockingFlow(GRAPH * L, int s, int t, int fct);
void AddFlow(GRAPH *G, GRAPH *L);
int AugmentFlow( GRAPH * G, int s, int t, int fct, GRAPH * L);
int VertexFlow(int i, GRAPH *G);
int LayeredGraph(GRAPH * G, int s, int sink, GRAPH * L);
int FindFlow( GRAPH * G, int s, int t, int fct );
int UpdatePath(GRAPH *G, int S[], int n, EDGE2 *current[]);
void Dinic(GRAPH * G, int v1, int v2);
int LG2(GRAPH *G, int s, int sink);
void freeGraph(GRAPH * G);

#endif // _DINIC_H
