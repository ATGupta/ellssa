__device__ int IN = 1;
__device__ int OUT = 0;
__device__ int WAIT = -1;

__device__ int READY = 0;
__device__ int BUSY = 2;
__device__ int IDLE = -1;
__device__ int ALLOWED = 1;

#include "common.h"
#include "gk.h"
#include "hea.h"
#include "t.h"
#include "lg.h"

__global__
void run(volatile int* g, int order, volatile int* states, volatile int* degrees,
		volatile int* miscbool,
		volatile int* isCurrentlyExecuting, volatile int* timesExecuted, volatile int* wantToExecute,
		volatile int* cores, volatile int* moves)
{
	printf("Hi ATG.! I am thread %d in block %d\n", threadIdx.x, blockIdx.x);
	//while(miscbool[0] == 0) {
	//		monitor(g, states, miscbool, order, degrees);
	//}
	cores[0] = threadIdx.x+1;
}

__device__
void releaseLock(int i, volatile int* executionStatus) {
	executionStatus[i] = IDLE;
}

__device__
void acquireFilterLock(int i, volatile int* g, volatile int* level, volatile int* victim, volatile int* degrees, volatile int* miscbool, volatile int* executionStatus)
{
	for (int jLoop = 0; jLoop < degrees[i] && miscbool[1] == 0; jLoop++) {
		int j = getGraphValue(g, i, jLoop);
		if(executionStatus[j] != IDLE) {
			level[i] = j;
			victim[j] = i;
			
			bool conflicts_exist = true;
			while (conflicts_exist && miscbool[1] == 0) {
				conflicts_exist = false;
				for (int kLoop = 0; kLoop < degrees[i] && miscbool[1] == 0; kLoop++) {
					int k = getGraphValue(g, i, kLoop);
					if(executionStatus[k] != IDLE) {
						if (level[k] >= j && victim[j] == i) {
							conflicts_exist = true;
							//break;
						}
					}
				}
			}
		}
	}
	
	executionStatus[i] = BUSY;
}

__global__
void runlg(volatile int* g, int order, volatile int* states, volatile int* degrees,
		volatile int* miscbool,
		volatile int* executionStatus, volatile int* level, volatile int* victim,
		int* cores, volatile int* moves)
{
	if (miscbool[1] == 1) {
		return;
	}
	
	int gid = blockIdx.x * blockDim.x + threadIdx.x;
	if(gid >= order+1) return;
	
	//boolean[] problem = miscbool[0];
	//boolean[] stop = miscbool[1];
	
	int run = 0;
	
	if (gid == 0) {
		while(miscbool[1] == 0) monitor(g, states, miscbool, order, degrees);
		atomicAdd(&cores[0], 1);
		return;
	}
	
	gid--;
	
	for (int i = gid; ; i++) {
		if (miscbool[1] == 1) {
			if(run == 1) atomicAdd(&cores[0], 1);
			return;
		}
		
		if(executionStatus[i] == IDLE) {
			executionStatus[i] = READY;
			//printf("Hi ATG.! I am thread %d in block %d. WAIT FOR LOCK\n", threadIdx.x, blockIdx.x);
			acquireFilterLock(i, g, level, victim, degrees, miscbool, executionStatus);
			//printf("Hi ATG.! I am thread %d in block %d. LOCK ACQUIRED\n", threadIdx.x, blockIdx.x);
			
			node_lg(i, g, states, degrees, order);
			run = 1;
			
			releaseLock(i, executionStatus);
			//printf("Hi ATG.! I am thread %d in block %d. LOCK RELEASED\n", threadIdx.x, blockIdx.x);
		}
		if (i == order - 1)
			i = -1;
	}
}

__device__
void lockstepScheduler(volatile int* states, volatile int* g, volatile int* miscbool, int order, volatile int* degrees, volatile int* executionStatus, volatile int* lockstepNumber) {
	//boolean[] problem = miscbool[0];
	//boolean[] stop = miscbool[1];
	//boolean[] startNextRound = miscbool[2];
	
	if(miscbool[2] == READY) {
		bool allReady = true;
		for (int i = 0; i < order; i++) {
			if (!(executionStatus[i] == READY || executionStatus[i] == IDLE))
				allReady = false;
		}
		if (allReady) {
			++miscbool[3];
			miscbool[2] = ALLOWED;
		}
		//printf("miscbool[2] = %d, ALLOWED = %d\n", miscbool[2], ALLOWED);
	} else {
		bool allNotReady = true;
		for (int i = 0; i < order; i++) {
			if(!(executionStatus[i] != READY || lockstepNumber[i] <= miscbool[3])){
				allNotReady = false;
				//printf("i %d, executionStatus[i] = %d, lockstepNumber[i] = %d, miscbool[3] %d\n", i, executionStatus[i], lockstepNumber[i], miscbool[3]);
			}
		}
		if (allNotReady) {
			miscbool[2] = READY;
			monitor(g, states, miscbool, order, degrees);
		}
	}
}

__device__
void waitForLockstep(int i, volatile int* miscbool, volatile int* executionStatus, volatile int* lockstepNumber){
	while (!((miscbool[2] == ALLOWED && lockstepNumber[i] < miscbool[3]) || miscbool[1] == 1)) {}
	// 1st -> only that node will perform execution which has not executed in the current lockstep, that is, miscbool[3]
	executionStatus[i] = BUSY;
}

__device__
void releaseForLockstep(int i, volatile int* executionStatus, volatile int* lockstepNumber, volatile int* miscbool){
	executionStatus[i] = IDLE;
	lockstepNumber[i] = miscbool[3];
}

__global__
void runt(volatile int* g, int order, volatile int* states, volatile int* degrees,
		volatile int* miscbool,
		volatile int* executionStatus, volatile int* stateChanged, volatile int* stateNotChanged, volatile int* lockstepNumber,
		int* cores, volatile int* moves)
{
	int gid = blockIdx.x * blockDim.x + threadIdx.x;
	if(gid >= order+1) return;
	
	//boolean[] problem = miscbool[0];
	//boolean[] stop = miscbool[1];
	//boolean[] startNextRound = miscbool[2];
	
	int run = 0;
	
	if (gid == 0) {
		while(miscbool[1] == 0) lockstepScheduler(states, g, miscbool, order, degrees, executionStatus, lockstepNumber);
		atomicAdd(&cores[0], 1);
		return;
	}
	
	gid--;
	
	for (int i = gid; ; i++) {
		if (miscbool[1] == 1) {
			if(run == 1) atomicAdd(&cores[0], 1);
			return;
		}
		
		if (executionStatus[i] == IDLE) {
			executionStatus[i] = READY;
			//printf("Hi ATG.! I, %d, am thread %d in block %d. WAIT FOR LOCKSTEP\n", i, threadIdx.x, blockIdx.x);
			waitForLockstep(i, miscbool, executionStatus, lockstepNumber);
			//printf("Hi ATG.! I am thread %d in block %d. IN LOCKSTEP\n", threadIdx.x, blockIdx.x);
			
			int prevState = states[i];
			node_t(i, g, states, degrees);
			run = 1;
			if(prevState == states[i]) ++stateNotChanged[i];
                        else ++stateChanged[i];

			
			releaseForLockstep(i, executionStatus, lockstepNumber, miscbool);
			//printf("Hi ATG.! I am thread %d in block %d. LOCKSTEP RELEASED\n", threadIdx.x, blockIdx.x);
		}
		
		if (i == order - 1)
			i = -1;
	}
}

__global__
void runhea(volatile int* g, int order, volatile int* states, volatile int* degrees,
		volatile int* miscbool,
		volatile int* pointers, volatile int* executionStatus, volatile int* stateChanged, volatile int* stateNotChanged, volatile int* lockstepNumber,
		int* cores, volatile int* moves)
{
	int gid = blockIdx.x * blockDim.x + threadIdx.x;
	if(gid >= order+1) return;
	
	//printf("gid %d\n", gid);
	//boolean[] problem = miscbool[0];
	//boolean[] stop = miscbool[1];
	//boolean[] startNextRound = miscbool[2];
	
	int run = 0;
	
	if (gid == 0) {
                while(miscbool[1] == 0) lockstepScheduler(states, g, miscbool, order, degrees, executionStatus, lockstepNumber);
                atomicAdd(&cores[0], 1);
                return;
        }
	
	gid--;
	
	for (int i = gid;; i++) {
		if (miscbool[1] == 1) {
			if(run == 1) atomicAdd(&cores[0], 1);
			return;
		}
		
		if (executionStatus[i] == IDLE) {
			executionStatus[i] = READY;
			//printf("Hi ATG.! I, %d, am thread %d in block %d. WAIT FOR LOCKSTEP. lockstep %d, miscbool[3] %d, miscbool[2] %d, READY %d\n", i, threadIdx.x, blockIdx.x, lockstepNumber[i], miscbool[3], miscbool[2], READY);
			waitForLockstep(i, miscbool, executionStatus, lockstepNumber);
			//printf("Hi ATG.! I am thread %d in block %d. IN LOCKSTEP\n", threadIdx.x, blockIdx.x);
			
			int prevState = states[i];
			node_t(i, g, states, degrees);
			run = 1;
			if(prevState == states[i]) ++stateNotChanged[i];
                        else ++stateChanged[i];

			releaseForLockstep(i, executionStatus, lockstepNumber, miscbool);
			//printf("Hi ATG.! I am thread %d in block %d. LOCKSTEP RELEASED\n", threadIdx.x, blockIdx.x);
		}		
		if (i == order - 1)
			i = -1;
	}
}

__global__
void rungk(volatile int* g, int order, volatile int* states, volatile int* degrees,
		volatile int* miscbool,
		volatile int* executionStatus, volatile int* stateChanged, volatile int* stateNotChanged,
		int* cores, volatile int* moves)
{
	int gid = blockIdx.x * blockDim.x + threadIdx.x;
	if(gid >= order+1) return;

	bool run = false;
	
	if (gid == 0) {
		while(miscbool[1] == 0) monitor(g, states, miscbool, order, degrees);
		atomicAdd(&cores[0], 1);
		return;
	}

	gid--;
	
	for(int i=gid; ; i++) {
	//for(int i=gid; ; i=i+8) {
		if(miscbool[1] == 1) {
			if(run == true) atomicAdd(&cores[0], 1);
			return;
		}
		if(executionStatus[i] == IDLE) {
			executionStatus[i] = BUSY;
			//printf("Hi ATG.! I am thread %d in block %d. STARTING EXECUTION.\n", threadIdx.x, blockIdx.x);
			
			int prevState = states[i];
			node_gk(i, g, states, degrees, miscbool, order, moves);
			
			//printf("Hi ATG.! I am thread %d in block %d. FINISHED EXECUTION.\n", threadIdx.x, blockIdx.x);
			run = true;
			if(prevState == states[i]) ++stateNotChanged[i];
			else ++stateChanged[i];
			executionStatus[i] = IDLE;
		}

		if(i == order) i=-1;
	}
}
