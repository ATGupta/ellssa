__device__ bool maximallyIndependent(volatile int*, volatile int*, volatile int*, volatile int*, int);
__device__ void makeCopy(volatile int*, volatile int*, int);
__device__ bool unconflicted(int, volatile int*, volatile int*, volatile int*, int);
__device__ bool conflicted(int, volatile int*, volatile int*, volatile int*, int);
__device__ bool addable(int, volatile int*, volatile int*, volatile int*);

__device__
int getGraphValue(volatile int* g, int row, int col) {
	int start = g[row + 1];
	return g[start + col];
}

__device__
bool addable(int i, volatile int* g, volatile int* states, volatile int* degrees) {
	for (int jLoop = 0; jLoop < degrees[i]; jLoop++) {
		int j = getGraphValue(g, i, jLoop);

		if (states[j] == IN) {
			return false;
		}
	}
	return true;
}

__device__
bool unconflicted(int i, volatile int* g, volatile int* states, volatile int* degrees, int order) {
	if (states[i] == OUT) {
		return true;
	}
	
	for (int jLoop = 0; jLoop < degrees[i]; jLoop++) {
		int j = getGraphValue(g, i, jLoop);
		
		if (states[j] == IN)
			return false;
	}
	return true;
}

__device__
bool conflicted(int i, volatile int* g, volatile int* states, volatile int* degrees, int order) {
	if(unconflicted(i, g, states, degrees, order)) return false;
	else return true;
}

__device__
void monitor(volatile int* g, volatile int* states, volatile int* miscbool, int order, volatile int* degrees) {
	//boolean[] problem = miscbool[0];
	//boolean[] stop = miscbool[1];
	
	for(int i = 0; i < order; i++) {
                if(states[i] == IN){
                        for(int jLoop = 0; jLoop < degrees[i]; jLoop++){
                                int j = getGraphValue(g, i, jLoop);
                                if(states[j] != OUT) return;
                        }
                } else if(states[i] == OUT) {
                        bool allOut = true;
                        for(int jLoop = 0; jLoop < degrees[i]; jLoop++){
                                int j = getGraphValue(g, i, jLoop);
                                if(states[j] == IN) allOut = false;
                        }
                        if(allOut == true) return;
                }
		else return;
        }
	miscbool[1] = 1;
}
