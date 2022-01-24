__device__ void node_gk(int, volatile int*, volatile int*, volatile int*, volatile int*, int, volatile int*, volatile int*);
__device__ bool forbidden_gk(int, volatile int*, volatile int*, volatile int*);

__device__
void node_gk(int i, volatile int* g, volatile int* states, volatile int* degrees, volatile int* miscbool, int order, volatile int* moves) {
	if(conflicted(i, g, states, degrees, order)) {
		states[i] = OUT;
	}
	else if(forbidden_gk(i, g, states, degrees)) {
		states[i] = IN;
	}
}

__device__
bool forbidden_gk(int i, volatile int* g, volatile int* states, volatile int* degrees) {
	if (states[i] == IN) {
		return false;
	}
	
	if(!addable(i, g, states, degrees)) {
		return false;
	}
	
	for (int jLoop = 0; jLoop < degrees[i]; jLoop++) {
		int j = getGraphValue(g, i, jLoop);
		if (j > i) 
			if(addable(j, g, states, degrees)) {
				return false;
			}
	}
	return true;
}
