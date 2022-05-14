#include <sys/ipc.h>
#include <sys/shm.h>
#include <bits/stdc++.h>
#include <semaphore.h>

const int limit = 100;

int shmid;
char *p;

union qwq {
    sem_t sem;
    long long awa;
};

qwq * sem_input;
qwq * sem_output;

void init() {
    shmid = shmget((key_t)0x2333, 1024, 0666|IPC_CREAT);
    if( shmid == -1 ) std::cout << "create shared memory failed.\n", exit(-1);
    p = (char *)shmat( shmid, 0, 0 );
    memset(p, 0, 1024);
    
    sem_input = new(p) qwq;
    if( sem_init( &(sem_input -> sem), 1, limit ) < 0 ) std::cout << "create sem_input failed\n", exit(-1);
    p += sizeof(sem_input);
    p += 32;
    std::cout << sem_input -> awa << "\n";
    
    sem_output = new(p) qwq;
    if( sem_init( &(sem_output -> sem), 1, 0 ) < 0 ) std::cout << "create sem_output failed\n", exit(-1);
    p += sizeof(sem_output);
    p += 32;
    std::cout << sem_output -> awa << "\n";
}

int main() {
    init();
    srand( time(NULL) );
    while( true ) {
        std::cout << sem_input -> awa << " " << sem_output -> awa << "\n";
        sem_wait(&(sem_input -> sem));
        for(int i = 0; i < limit; i ++) if( p[i] == 0 )
          { p[i] = rand() % 26 + 'a'; 
            std::cout << "productor make a '" << p[i] << "' on " << i << ".\n";
            sem_post(&(sem_output -> sem));
            sleep( log(rand()) / 25.0 );
            break;
          }
    }
}
