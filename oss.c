
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#define maxTimeNS 1000000000
#define maxtimeS 1


char *outputFile;
int maxTime;
int maxProcess = 10;
FILE *ofp;

// process control block structure
struct pcb{
	float CPU_time;
	float total_time;
	int burst_time;
	int pid;
	int priority;
};

// clock structure;
struct clock{
	unsigned int sec;
	unsigned int nano_sec;
	float quantum;
};

// pointer to the pcb and the clock;
struct clock* clock;
struct pcb* pcb;


// alarm that kills all processes;
void alarmHandler(int sig){
        FILE *fp;
        fp = fopen(outputFile,"a");
        fprintf(fp,"OSS: Master process terminated after the the maximum amount of time: %dseconds\n",maxTime);
        fclose(fp);
        int i=0;
        // kill all the child process
	while(i <18){
		if(pcb[i].pid>0){
			kill(pcb[i].pid,SIGTERM);
		}
		i=i+1;
	}
	kill(getpid(),SIGTERM);
}

int main(int argc, char* argv[]){
	signal(SIGALRM,alarmHandler);	
	int opt;
	outputFile = "logFile";
	maxTime = 3;	
	int childSpot;


	// command line options.
	while((opt = getopt(argc,argv,"ht:f:"))!=-1){
                switch(opt){
                        case 'h':
				printf("\n the command line options are -h,t and -f\n");
				printf("use -t with an argument followed by it to change the max time the program should run\n");
				printf("use -f with an argument followed by it to change the output log file\n");
				break;
                        case 'f':
				outputFile = optarg;	
                                break;
			case 't':
				maxTime = atoi(optarg);
				break;
			case '?':
                                break;
                }
        }

	alarm(maxTime);
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
		pcb[i].priority = -1;
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
	clock->quantum =1;	
	// child process argument for shared memory
	char arg1[10];
	char arg2[10];	
	snprintf(arg1,10,"%d", pcbid);
	snprintf(arg2,10,"%d",clockid);

	srand(time(0));
	int picked = 0;
	unsigned int s;
	unsigned int ns;
	// infinite loop that will exit when the alarm goes off;
	while(1){
		// picks a random time interval then fork a child after it has passed.
		if(picked == 0){
			s = clock->sec;
			ns = clock->nano_sec + rand()%(10000000000)+1;
			if(ns>= maxTimeNS){
				s = s+1;
				ns = ns-maxTimeNS;
			}
			picked = 1;
		}
	
		// clock will continue to run.
		clock->nano_sec = clock->nano_sec +1000;
                if(clock->nano_sec >= maxTimeNS){
                        clock->sec = clock->sec +1;
                        clock->nano_sec =0;
                }
		// if the amount of time interval has passed and it is time to start another process
		if(clock->sec >= s && picked == 1){
			if(clock->nano_sec >= ns){
				// set picked to 0 then 
				picked = 0;

				// if max child is not exceeded, find a spot on the table then spawn another process;
				if(maxProcess >0){
					maxProcess = maxProcess -1;
					int child_pid = fork();
                                        if(child_pid <=0){
                                                printf("hello\n");
						execlp("./user","./user",arg1,arg2,(char *)NULL);
						exit(0);
                                        }
					// look for a empty spot on the process control table, store the forked process info;		
					int i =0;
       					while(i<18){
                				if(pcb[i].pid == -1){
							childSpot = i;
                        				pcb[i].pid = child_pid;
							pcb[i].priority = rand()%3+1;
							pcb[i].priority -= 1;			
							break;
						}
						i+=1;
					}				
				}
				ofp = fopen(outputFile,"a");
				fprintf(ofp,"OSS: Generating process PID: %d, priority level: %d, at time: %d:%d\n",
						pcb[childSpot].pid,pcb[childSpot].priority,clock->sec,clock->nano_sec);
				fclose(ofp);
				
			}
		}

	}
	return 0;	

}
