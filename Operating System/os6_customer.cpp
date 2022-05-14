#include <sys/ipc.h>
#include <sys/shm.h>
#include <bits/stdc++.h>
#include <semaphore.h>

const int limit = 100;

int shmid;
char *p;

sem_t * sem_input;
sem_t * sem_output;

void init() {
    shmid = shmget((key_t)0x2333, 1024, 0666|IPC_CREAT);
    if( shmid == -1 ) std::cout << "create shared memory failed.\n", exit(-1);
    p = (char *)shmat( shmid, 0, 0 );
    
    sem_input = (sem_t *)p;
    p += sizeof(sem_input);
    p += 32;
    
    sem_output = (sem_t *)p;
    p += sizeof(sem_output);
    p += 32;
}


int main() {
    init();
    srand( time(NULL) );
    while( true ) {
        sem_wait(sem_output);
        for(int i = 0; i < limit; i ++) if( p[i] != 0 )
          { std::cout << "customer take a '" << p[i] << "' on " << i << ".\n";
            p[i] = 0;
            sem_post(sem_input);
            sleep( log(rand()) / 25.0 );
            break;
          }
    }  
}
