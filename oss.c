#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

char *outputFile;

// process control block structure
struct pcb{
	float CPU_time;
	float total_time;
	int burst_time;
	int pid;
};

// clock structure;
struct clock{
	unsigned int sec;
	unsigned int nano_sec;
};

// pointer to the pcb and the clock;
struct clock* clock;
struct pcb* pcb;


int main(int argc, char* argv[]){
	int opt;
	outputFile = "logFile";
	
	// command line options.
	while((opt = getopt(argc,argv,"hf:"))!=-1){
                switch(opt){
                        case 'h':
				printf("\n the command line options are -h and -f\n");
				printf("use -f with an argument followed by it to change the output log file\n");
				break;
                        case 'f':
				outputFile = optarg;	
                                break;
			case '?':
                                break;
                }
        }

	// initializing shared memory for 18 process control blocks
	int pcbid;
        int pctsize = 18 * sizeof(pcb);
	pcbid = shmget(0x1235,pctsize,0666|IPC_CREAT);
        if(pcbid == -1){
                perror("Shared memory\n");
                return 0;
        }
        pcb = shmat(pcbid,NULL,0);
        if(pcb == (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }

	// initialize all process control blocks to 0 for times and -1 for pid
	int i = 0;
	while(i< 18){
		pcb[i].CPU_time = 0.0;
		pcb[i].total_time = 0.0;
		pcb[i].burst_time = 0;
		pcb[i].pid = -1;
		i =i+1;
	}

	// initializing shared memory for the clock;
	int clockid;
	int clocksize = sizeof(clock);
	clockid = shmget(0x2235,clocksize,0666|IPC_CREAT);
	if(clockid == -1){
                perror("Shared memory\n");
                return 0;
        }
        clock = shmat(clockid,NULL,0);
        if(clock == (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }
	clock->sec=0;
	clock->nano_sec=0;





	return 0;	
}
