#include <bits/stdc++.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <iostream>

using namespace std;


#define shm_key 8455
#define sem_key 8273

int main()
{
    int shm_id = shmget(shm_key, 0, 0666);
    if (shm_id == -1)
    {
        cout << "Failed to get shared memory" << endl;
        return 1;
    }
    int sem_id = semget(sem_key, 3, 0666);
    if (sem_id == -1)
    {
        cout << "Failed to get semaphore" << endl;
        return 1;
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        cout << "Failed to delete shared memory" << endl;
        return 1;
    }
    if (semctl(sem_id, 0, IPC_RMID) == -1)
    {
        cout << "Failed to delete semaphore" << endl;
        return 1;
    }
    return 0;
}
