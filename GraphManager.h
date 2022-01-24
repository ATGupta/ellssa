#include "IntegerSet.h"
#include <stdio.h>
#include <fstream>
#include "StringTokenizer.h"
#include <string>
#include <stdlib.h>

class GraphManager{
	int order = 0;
	int size = 0;
	public:
	GraphManager(){}

	int getOrder(){
		return order;
	}
	
	int getSize(){
		return size;
	}
	
	IntegerSet** readGraphGenericStyle(std::string path){
		std::ifstream ifs(path);
		if(!ifs.is_open()){
			printf("input file not open.");
			exit (EXIT_FAILURE);
		}
		
		std::string s;
		StringTokenizer* st = new StringTokenizer();
		getline(ifs,s);
		getline(ifs,s);
		st->tokenize(s);
		st->setBreakCharacters(",");
		
		order = s2i(st->nextToken());
		int startNodeIndex = s2i(st->nextToken());
		for(int i = 0; i < 2; i++) st->nextToken();
		s = st->nextToken();
		if(s.length() > 0) size = s2i(s);
		
		IntegerSet** is = (IntegerSet**)malloc(order * sizeof(IntegerSet*));
		for(int i = 0; i < order; i++){
			is[i] = new IntegerSet();
		}
		
		while (getline(ifs,s)){
			st->tokenize(s);
			int node1 = s2i(st->nextToken());
			int node2 = s2i(st->nextToken());
			
			IntegerSet* a = is[node1 - startNodeIndex];
			a->add(node2 - startNodeIndex);
			
			a = is[node2 - startNodeIndex];
			a->add(node1 - startNodeIndex);
		}
		
		return is;
	}
	
	int* getDegrees(IntegerSet **is){
		int* d = (int*)malloc(order * sizeof(int));
		for(int i = 0; i < order; i++){
			d[i] = is[i]->length;
		}
		return d;
	}

	int sumOfArray(int* a, int l){
		int s = 0;
		for(int i = 0; i < l; i++){
			s = s + a[i];
		}
		return s;
	}
	
	int* graphAdjacencyTo1D(IntegerSet** is, int* degrees) {
		int sumDegree = sumOfArray(degrees, order);
		int size = 1 + (order+1) + sumDegree;
		
		int* g = (int*)malloc(size * sizeof(int));
		g[0] = order;
		int i = 1;
		int pos = 0;
		for(; i < order + 1; i++) {
			g[i] = order + 2 + pos;
			for(int j = 0; j < degrees[i - 1]; j++, pos++) {
				int* row = is[i - 1]->toArray();
				g[order + 2 + pos] = row[j];
			}
		}
		g[order+1] = size;
		
		return g;
	}
	
	int* readStates() {
		int* b = (int*)malloc(order * sizeof(int));
		std::ifstream ifs("initial-states/states");
		std::string s,f="";
		for(int i = 0; i < order; i++){
			int b_ = 0;
			ifs>>b_;
			b[i] = b_;
		}
		
		return b;
	}
};
