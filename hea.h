__device__
bool R1_hea(int i, volatile int* g, volatile int* s, volatile int* pointers, volatile int* degrees) {
	if(s[i] == IN) {
		return false;
	}
	
	for(int jLoop = 0; jLoop < degrees[i]; jLoop++) {
		int j = getGraphValue(g, i, jLoop);
		
		if(s[j] == IN) {
			return false;
		}
	}
	
	s[i] = IN;
	return true;
}

__device__
bool R2_hea(int i, volatile int* g, volatile int* s, volatile int* pointers, volatile int* degrees) {
	if(s[i]==false) {
		return false;
	}
	
	for(int jLoop = 0; jLoop < degrees[i]; jLoop++) {
		int j = getGraphValue(g, i, jLoop);
		
		if(s[j] == IN) {
			s[i] = OUT;
			return true;
		}
	}
	
	return false;
}


__device__ 
void node_hea(int i, volatile int* g, volatile int* states, volatile int* pointers, volatile int* degrees) {
	if(R1_hea(i, g, states, pointers, degrees)) {}
	else if(R2_hea(i, g, states, pointers, degrees)) {}
}
