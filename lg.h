__device__
void node_lg(int i, volatile int* g, volatile int* states, volatile int* degrees, int order) {
	if(conflicted(i, g, states, degrees, order)) {
		states[i] = OUT;
	}
	else if(addable(i, g, states, degrees)) {
		states[i] = IN;
	}
}
