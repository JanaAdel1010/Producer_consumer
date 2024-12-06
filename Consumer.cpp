#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <random>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <map>
#include <vector>

#define SHM_KEY 1234
#define SEM_KEY 5678

struct Commodity {
    char name[10];
    double price;
};

struct SharedBuffer {
    Commodity buffer[100];  // Default to 100 entries
    int in;  // Index to add to buffer
    int out; // Index to remove from buffer
};

// Semaphore functions
void semaphore_wait(int sem_id, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    semop(sem_id, &sb, 1);
}

void semaphore_signal(int sem_id, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    semop(sem_id, &sb, 1);
}

void clear_screen() {
    printf("\e[1;1H\e[2J");
}
Commodity take_from_buffer(SharedBuffer* shared_buffer, int buffer_size){
        Commodity commodity = shared_buffer->buffer[shared_buffer->out];
        shared_buffer->out = (shared_buffer->out + 1) % buffer_size;
        return commodity;
}
int main(int argc, char *argv[]) {
    int buffer_size;

    if (argc != 2) {
        std::cerr << "Usage: ./consumer <BufferSize>" << std::endl;
        return 1;
    }

    buffer_size = std::stoi(argv[1]);

    // Attach to shared memory
    int shm_id = shmget(SHM_KEY, sizeof(SharedBuffer) + buffer_size * sizeof(Commodity), 0666 | IPC_CREAT);
    SharedBuffer *shared_buffer = static_cast<SharedBuffer *>(shmat(shm_id, nullptr, 0));

    int sem_id = semget(SEM_KEY, 3, 0666 | IPC_CREAT);

    std::map<std::string, std::vector<double>> prices;

    while (true) {
        clear_screen();

        std::cout << "+-------------------+--------+----------+" << std::endl;
        std::cout << "| Commodity         | Price  | AvgPrice |" << std::endl;
        std::cout << "+-------------------+--------+----------+" << std::endl;

        semaphore_wait(sem_id, 2); 
        semaphore_wait(sem_id, 0); 

        Commodity commodity =take_from_buffer(shared_buffer,buffer_size);
    
        semaphore_signal(sem_id, 0); 
        semaphore_signal(sem_id, 1); 

        std::string name = commodity.name;
        double price = commodity.price;

        prices[name].push_back(price);
        if (prices[name].size() > 5) {
            prices[name].erase(prices[name].begin());
        }

        double avg_price = 0;
        for (double p : prices[name]) avg_price += p;
        avg_price /= prices[name].size();

        for (auto &[commodity_name, price_list] : prices) {
            double current_price = price_list.back();
            double previous_avg = avg_price;
            std::string trend = (current_price > previous_avg) ? "↑" : "↓";

            std::cout << "| " << std::setw(17) << commodity_name
                      << " | " << std::fixed << std::setprecision(2) << std::setw(6) << current_price
                      << " | " << std::setw(8) << avg_price << trend << " |" << std::endl;
        }

        std::cout << "+-------------------+--------+----------+" << std::endl;
        usleep(500 * 1000); 
    }

    return 0;
}
