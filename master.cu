#include "GraphManager.h"
#include <string>
#include <iostream>
#include "kernels.h"
#include <sys/time.h>
#include <sys/stat.h>

int askOptions() {
	printf("\nEnter <1> For Gupta Kulkarni.\nEnter <2> for Hedetniemi et al.\nEnter <3> for Turau.\nEnter <4> for lock generic.\nEnter <0> to exit the program.\nEnter your choice : ");
	int op = 0;
	scanf("%d", &op);
	std::cout<< "\n\n";
	
	return op;
}

int getGraphValueForFinalCheck(int* g, int row, int col) {
	int start = g[row + 1];
	return g[start + col];
}

bool finalCheckMaximallyIndependent(int* g, int *states, int *degrees, int order) {
	for(int i = 0; i < order; i++) {
		if(states[i] == 1){
			for(int jLoop = 0; jLoop < degrees[i]; jLoop++){
				int j = getGraphValueForFinalCheck(g, i, jLoop);
				if(states[j] != 0) return false;
			}
		} else {
			if(states[i] != 0) return false;
			
			bool allOut = true;
			for(int jLoop = 0; jLoop < degrees[i]; jLoop++){
				int j = getGraphValueForFinalCheck(g, i, jLoop);
				if(states[j] == 1) allOut = false;
			}
			if(allOut == true) return false;
		}
	}
	return true;
}

long int arraySum(int* a, int size){
	long int sum = 0;
	for(int i = 0; i < size; i++){
		sum += a[i];
	}
	return sum;
}

bool driver(int* d, int* g, int *s, int order, int size){
	int* miscbool = (int*)malloc(4 * sizeof(int)); miscbool[0] = 0; miscbool[1] = 0; miscbool[2] = 0; miscbool[3] = 0;
	//problem?, stop?, lockstep?, lockStepLevel
	
	int* p = (int*)malloc(order * sizeof(int)); // pointers
	int* a1 = (int*)malloc(order * sizeof(int)); //isCurrentlyExecuting
	int* a2 = (int*)malloc(order * sizeof(int)); //timesExecuted
	int* a3 = (int*)malloc(order * sizeof(int)); //wantToExecute
	int* a4 = (int*)malloc(order * sizeof(int)); //stateChanged
	int* a5 = (int*)malloc(order * sizeof(int)); //stateNotChanged
	int* a6 = (int*)malloc(order * sizeof(int)); //lockstepNumber
	
	int op = askOptions();
	if(op <= 0 || op >= 5) {
		printf("Program exiting.\n");
		return true;
	}
	
	for(int i=0; i<order; i++) {
		s[i] = 1;
		p[i] = -1;
		a1[i] = -1;
		a2[i] = -1;
		a3[i] = -1;
		a4[i] = 0;
		a5[i] = 0;
		a6[i] = 0;
	}
	
	int* cores = (int*)malloc(sizeof(int)); cores[0] = 0;
	int* moves = (int*)malloc(sizeof(int)); moves[0] = 0;
	
	int *g_cuda, *d_cuda, *s_cuda,
		*miscbool_cuda,
		*p_cuda, *a1_cuda, *a2_cuda, *a3_cuda, *a4_cuda, *a5_cuda, *a6_cuda,
		*cores_cuda, *moves_cuda;
	
	cudaMalloc(&g_cuda, g[order+1] * sizeof(int));
	cudaMalloc(&d_cuda, order * sizeof(int));
	cudaMalloc(&s_cuda, order * sizeof(int));
	cudaMalloc(&miscbool_cuda, 4 * sizeof(int));
	cudaMalloc(&p_cuda, order * sizeof(int));
	cudaMalloc(&a1_cuda, order * sizeof(int));
	cudaMalloc(&a2_cuda, order * sizeof(int));
	cudaMalloc(&a3_cuda, order * sizeof(int));
	cudaMalloc(&a4_cuda, order * sizeof(int));
	cudaMalloc(&a5_cuda, order * sizeof(int));
	cudaMalloc(&a6_cuda, order * sizeof(int));
	cudaMalloc(&cores_cuda, sizeof(int));
	cudaMalloc(&moves_cuda, sizeof(int));
	
	cudaEvent_t start, stop;
        cudaEventCreate(&start);
        cudaEventCreate(&stop);
	
	cudaMemcpy(g_cuda, g, g[order+1] * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(d_cuda, d, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(s_cuda, s, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(miscbool_cuda, miscbool, 4 * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(p_cuda, p, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(a1_cuda, a1, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(a2_cuda, a2, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(a3_cuda, a3, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(a4_cuda, a4, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(a5_cuda, a5, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(a5_cuda, a6, order * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(cores_cuda, cores, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(moves_cuda, moves, sizeof(int), cudaMemcpyHostToDevice);
	
	printf("graph read.\nmemories allocated.\ngraph order: %d\ngraph array size: %d\nstarting execution now.\n", order, g[order+1]);
	
	cudaEventRecord(start);
	
	if(op == 1) rungk<<<(order + 1 + 255)/256, 256>>>(g_cuda, order, s_cuda, d_cuda, miscbool_cuda, a1_cuda, a4_cuda, a5_cuda, cores_cuda, moves_cuda);
	else if(op == 2) runhea<<<(order + 2 + 255)/256, 256>>>(g_cuda, order, s_cuda, d_cuda, miscbool_cuda, p_cuda, a1_cuda, a4_cuda, a5_cuda, a6_cuda, cores_cuda, moves_cuda);
	else if(op == 3) runt<<<(order + 2 + 255)/256, 256>>>(g_cuda, order, s_cuda, d_cuda, miscbool_cuda, a1_cuda, a4_cuda, a5_cuda, a6_cuda, cores_cuda, moves_cuda);
	else runlg<<<(order + 2 + 255)/256, 256>>>(g_cuda, order, s_cuda, d_cuda, miscbool_cuda, a1_cuda, a2_cuda, a3_cuda, cores_cuda, moves_cuda);
	
	//run<<<1, order + 1>>>(g_cuda, order, s_cuda, d_cuda, miscbool_cuda, ice_cuda, te_cuda, we_cuda, cores_cuda, moves_cuda);
	//run<<<blocks, number of cores per block>>>(arguments);
	
	cudaEventRecord(stop);
	
	cudaError_t err = cudaGetLastError();
	if ( err != cudaSuccess ) {
		std::cout << "\nCUDA Error: " << cudaGetErrorString(err) << "\n\n";
		return true;
	}
	
	std::cout << "Execution Completed.\n";
	
	cudaMemcpy(miscbool, miscbool_cuda, 4 * sizeof(int), cudaMemcpyDeviceToHost);
	cudaMemcpy(cores, cores_cuda, sizeof(int), cudaMemcpyDeviceToHost);
	cudaMemcpy(g, g_cuda, g[order+1] * sizeof(int), cudaMemcpyDeviceToHost);
	cudaMemcpy(s, s_cuda, order * sizeof(int), cudaMemcpyDeviceToHost);
	cudaMemcpy(a4, a4_cuda, order * sizeof(int), cudaMemcpyDeviceToHost);
	cudaMemcpy(a5, a5_cuda, order * sizeof(int), cudaMemcpyDeviceToHost);	
	
	cudaEventSynchronize(stop);
	float time_taken = 0;
	cudaEventElapsedTime(&time_taken, start, stop);
	
	std::string file = "output/e";
	file.append(i2s(size));
	file.append("o");
	file.append(i2s(order));
	file.append("/");
	
	struct stat statFile = {0};
	char* fileCA = s2ca(file);
	if (stat(fileCA, &statFile) == -1) mkdir(fileCA, 0700);
	
	if(op == 1) file.append("gk");
	else if(op == 2) file.append("hea");
	else if(op == 3) file.append("t");
	else file = file.append("lg");
	
	file.append(i2s(order));
	std::ofstream ofs(file);
	
	ofs << "time diff = " << time_taken << "\n";
	std::cout << "time diff = " << time_taken << "\n";
	
	if(miscbool[0] == 0) {
		if(miscbool[1] == 0) {
			std::cout<< "False\n";
			ofs<<"False\n";
		} else {
			std::cout<< "True\n";
			ofs<<"True\n";
		}
	} else {
		std::cout<< "Null\n";
		ofs<<"Null\n";
	}
	printf("cores = %d\n", cores[0]);
	ofs<<"cores = " << cores[0] << "\n";
	
	if(finalCheckMaximallyIndependent(g, s, d, order)) printf("Maximally Independent.\n");
	else printf("Not Maximally Independent.\n");
	
	long int SC = arraySum(a4, order);
	long int SNC = arraySum(a5, order);
	ofs << "state changed " << SC << "\n";
	ofs << "state not changed " << SNC << "\n";
	std::cout << "state changed " << SC << "\n";
	std::cout << "state not changed " << SNC << "\n";
	
	if(op == 2 || op == 3) {
		std::cout << "total locksteps = " << miscbool[3];
		ofs << "total locksteps = " << miscbool[3];
	}
	
	ofs.close();
	
	cudaFree(g_cuda);
	cudaFree(d_cuda);
	cudaFree(s_cuda);
	cudaFree(miscbool_cuda);
	cudaFree(p_cuda);
	cudaFree(a1_cuda);
	cudaFree(a2_cuda);
	cudaFree(a3_cuda);
	cudaFree(a4_cuda);
	cudaFree(a5_cuda);
	cudaFree(a6_cuda);
	cudaFree(cores_cuda);
	cudaFree(moves_cuda);
	
	//total size  = 11 * addr + (11 * order + 2 * edges + 2) * sizeof(int)
	
	free(miscbool);
	free(p);
	free(a1);
	free(a2);
	free(a3);
	free(a4);
	free(a5);
	free(a6);
	free(cores);
	free(moves);
	
	return false;
}

int main(void)
{
	printf("\nHi ATG.! Please wait while I read the graph...\n");
	GraphManager* gm = new GraphManager();
	//IntegerSet **is = gm->readGraphGenericStyle("graphs/out.hiv"); //40,1,nodes,startingFrom
	IntegerSet **is = gm->readGraphGenericStyle("graphs/lastfm_asia_target.csv"); //7624,0,nodes,startingFrom,27806,edges
	//IntegerSet **is = gm->readGraphGenericStyle("graphs/musae_git_edges.csv"); //37700,0,nodes,startingFrom,289003,edges
	//IntegerSet **is = gm->readGraphGenericStyle("graphs/large_twitch_edges.csv"); //168114,0,nodes,startingFrom,6797557,edges
	//IntegerSet **is = gm->readGraphGenericStyle("graphs/out.roadNet-TX"); //1379917,1,nodes,startingFrom,1921660,edges
	
	int order = gm->getOrder();
	int size = gm->getSize();
	
	printf("Now converting to 1D (plus sorting, so, may take time)...\n");
	
	int* d = gm->getDegrees(is); // degrees
	int *g = gm->graphAdjacencyTo1D(is, d); //graph
	int* s = gm->readStates();
	
	free(gm);
	free(is);
	
	printf("I finished reading the graph. Thanks for waiting, ATG.!\n");
	bool exit = false;
	while(!exit) exit = driver(d, g, s, order, size);
	
	free(g);
	free(d);
	free(s);
}
