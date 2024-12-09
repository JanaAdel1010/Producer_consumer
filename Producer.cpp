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

#define SHM_KEY 8455
#define SEM_KEY 8273

struct Commodity {
    char name[11];
    double price;
};

struct SharedBuffer { 
    int in;  
    int out; 
    Commodity buffer[]; 
};

void sem_wait(int sem_id, int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    semop(sem_id, &op, 1);
}

void sem_signal(int sem_id, int sem_num) {
    struct sembuf op = {sem_num, 1, 0};
    semop(sem_id, &op, 1);
}

void append(SharedBuffer *shared_buffer, int buffer_size, const std::string &commodity_name, double price) {
    strncpy(shared_buffer->buffer[shared_buffer->in].name, commodity_name.c_str(), 11);
    shared_buffer->buffer[shared_buffer->in].name[10] = '\0';
    shared_buffer->buffer[shared_buffer->in].price = price;

    shared_buffer->in = (shared_buffer->in + 1) % buffer_size;
}

int main(int argc, char *argv[]) {
    std::string commodity_name;
    double mean, stddev, sleep_time;
    int buffer_size;

    if (argc != 6) {
        std::cerr << "Usage: ./producer <CommodityName> <Mean> <StdDev> <SleepTime(ms)> <BufferSize>" << std::endl;
        return 1;
    }

    commodity_name = argv[1];
    mean = std::stod(argv[2]);
    stddev = std::stod(argv[3]);
    sleep_time = std::stoi(argv[4]);
    buffer_size = std::stoi(argv[5]);

    if (commodity_name.length() > 10) {
        std::cerr << "Commodity name must be <= 10 characters!" << std::endl;
        return 1;
    }

    int shm_id = shmget(SHM_KEY, sizeof(SharedBuffer) + buffer_size * sizeof(Commodity), 0666 | IPC_CREAT);
    SharedBuffer *shared_buffer = static_cast<SharedBuffer *>(shmat(shm_id, nullptr, 0));

    int sem_id = semget(SEM_KEY, 3, 0666 | IPC_CREAT);

    std::default_random_engine generator(std::random_device{}());
    std::normal_distribution<double> distribution(mean, stddev);

    while (true) {
        double price = distribution(generator);

        std::time_t now = std::time(nullptr);
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        std::cerr << "[" << time_str << "] " << commodity_name << ": generating a new value " 
                  << std::fixed << std::setprecision(2) << price << std::endl;

        sem_wait(sem_id, 1);
        sem_wait(sem_id, 0);

        append(shared_buffer, buffer_size, commodity_name, price);

        std::cerr << "[" << time_str << "] " << commodity_name << ": placing " << price << " on shared buffer" << std::endl;

        sem_signal(sem_id, 0);
        sem_signal(sem_id, 2);

        std::cerr << "[" << time_str << "] " << commodity_name << ": sleeping for " << sleep_time << " ms" << std::endl;
        usleep(sleep_time * 1000);
    }

    return 0;
}