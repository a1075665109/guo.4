#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

struct pcb{
        float CPU_time;
        float total_time;
        int burst_time;
        int pid;
        int priority;
};

struct clock{
	unsigned int sec;
	unsigned int nano_sec;
	float quantum;
};

struct clock *clock;
struct pcb *pcb;

FILE *fp;
// state when the process starts.
// 1. terminate process at its time quantum;
// 2. waiting for event that'll last around 0-5 seconds
// 3. gets preempted after using 1-99 amount of its assigned quatum.
// 0. terminates now
int startProcess(){
	int a = rand()%50;
	if(a>50){
		return 1;
	}else if(a > 30){
		return 2;
	}else if (a > 10){
		return 3;
	}else{
		return 0;
	}
}

// find the location of the process in the process control table
int locate(int a){
	int i = 0;
	while (i<18){
		if(pcb[i].pid == a){
			return i;
		}
		i = i+1;
	}
	return 0;
}

int main(int argc, char *argv[]){
	srand(time(NULL));

	/*
	// get into the shared memory created by oss by using the pcbid and the clockid;
	int pcbid = atoi(argv[1]);
        int clockid = atoi(argv[2]);

	pcb = shmat(pcbid,NULL,0);
	clock = shmat(clockid,NULL,0);
	*/
	

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




	// get the process id number to locate which process control block it belongs to
	int pid = getpid();	
	int childSpot = locate(pid);
	

	// start the process by seeing which option it belongs to
	int a = startProcess();
	printf("%d\n",a); 

	fp = fopen("logFile","a");       
	// exit on 0
	if(a == 0){
		fprintf(fp,"OSS: process %d exited immediately at time %d.%d\n",pid,clock->sec,clock->nano_sec);
                printf("\nExit immediately\n");
                exit(0);
        }
	// go through with the time quantum on 1
        if(a == 1){
                float time = clock->quantum;
         	fprintf(fp,"OSS: Process %d executing for full quantum time %f seconds\n",pid,time);
	        wait(time);
		fprintf(fp,"OSS: Process %d finished its full quantum time\n",pid)
        }
	// wait for event for r.s second
        if(a == 2){
                float r = rand()%5;
		float s = rand()%1000;
		s = s/1000;
		float time = r+s;
		fprintf(fp,"OSS: Process %d waiting for event for %f seconds\n",pid,time);
		wait(time);
		fprintf(fp,"OSS: Process %d has finished waiting for an event\n",pid);
        }
	// uses p amount of ites assigned quantum
	if(a == 3){
		float p = rand()%99+1;
		float time = clock->quantum * p/100;
		fprintf(fp,"OSS: Process %d executing for %d percent of the quantum time, allowed time: \n",pid,p,time);
		wait(time);
		fprintf(fp,"OSS: Process %d has finished its allowed quantum time: %d seconds \n",pid,time);
	}
	fclose(fp);
	return 0;
}















