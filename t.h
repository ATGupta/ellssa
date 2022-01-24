__device__
bool inNeighbour(int v, volatile int* g, volatile int* states, volatile int* degrees) {
	for(int wLoop=0; wLoop<degrees[v]; wLoop++) {
		int w = getGraphValue(g, v, wLoop);
		
		if(states[w] == IN) {
			return true;
		}
	}
	return false;
}

__device__
bool waitNeighbourWithLowerID(int v, volatile int* g, volatile int* states, volatile int* degrees) {
	for(int wLoop=0; wLoop<degrees[v]; wLoop++) {
		int w = getGraphValue(g, v, wLoop);
		
		if(states[w] == WAIT && w<v) {
			return true;
		}
	}
	return false;
}

__device__
bool inNeighbourWithLowerID(int v, volatile int* g, volatile int* states, volatile int* degrees) {
	for(int wLoop=0; wLoop<degrees[v]; wLoop++) {
		int w = getGraphValue(g, v, wLoop);
		
		if(states[w] == IN && w<v) {
			return true;
		}
	}
	return false;
}

__device__
bool R1_t(int v, volatile int* g, volatile int* states, volatile int* degrees) {
	if(states[v] == OUT && !(inNeighbour(v, g, states, degrees))) {
		states[v] = WAIT;
		return true;
	}
	return false;
}

__device__
bool R2_t(int v, volatile int* g, volatile int* states, volatile int* degrees) {
	if(states[v] == WAIT && inNeighbour(v, g, states, degrees)) {
		states[v] = OUT;
		return true;
	}
	return false;
}

__device__
bool R3_t(int v, volatile int* g, volatile int* states, volatile int* degrees) {
	if(states[v] == WAIT && !inNeighbour(v, g, states, degrees) && !waitNeighbourWithLowerID(v, g, states, degrees)) {
		states[v] = IN;
		return true;
	}
	return false;
}

__device__
bool R4_t(int v, volatile int* g, volatile int* states, volatile int* degrees) {
	if(states[v] == IN && inNeighbour(v, g, states, degrees)) {
		states[v] = OUT;
		return true;
	}
	return false;
}

__device__
void node_t(int i, volatile int* g, volatile int* states, volatile int* degrees) {
	if(R1_t(i,g,states,degrees)) {}
	else if(R2_t(i,g,states,degrees)) {}
	else if(R3_t(i,g,states,degrees)) {}
	else if(R4_t(i,g,states,degrees)) {}
}
