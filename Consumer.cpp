#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <map>
#include <vector>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>

#define SHM_KEY 8455
#define SEM_KEY 8273
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./consumer <BufferSize>" << std::endl;
        return 1;
    }

    int buffer_size = std::stoi(argv[1]);
    int shm_id = shmget(SHM_KEY, sizeof(SharedBuffer) + buffer_size * sizeof(Commodity), 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        return 1;
    }

    SharedBuffer *shared_buffer = static_cast<SharedBuffer *>(shmat(shm_id, nullptr, 0));
    if (shared_buffer == (void *)-1) {
        perror("Failed to attach shared memory");
        return 1;
    }
    const char *commodities[] = {
        "Aluminum", "Copper", "Cotton", "CrudeOil", "Gold",
        "Led", "MenthaOil", "NaturalGas", "Nickel", "Silver", "Zinc"
    };
    for (int i = 0; i < buffer_size; i++) {
        if (i < 11) {
            strncpy(shared_buffer->buffer[i].name, commodities[i], 10);
            shared_buffer->buffer[i].name[10] = '\0';
        } else {
            memset(shared_buffer->buffer[i].name, '\0', sizeof(shared_buffer->buffer[i].name));
        }
        shared_buffer->buffer[i].price = 0.00;
    }
    shared_buffer->in = 0;
    shared_buffer->out = 0;

    // Access semaphores
    int sem_id = semget(SEM_KEY, 3, 0666 | IPC_CREAT);
    if (sem_id == -1) {
        perror("Failed to create semaphores");
        return 1;
    }

    semctl(sem_id, 0, SETVAL, 1);  // Mutex 
    semctl(sem_id, 1, SETVAL, buffer_size);  // Empty slots
    semctl(sem_id, 2, SETVAL, 0);  // Filled slots


    std::map<std::string, std::vector<double>> prices;
    std::map<std::string, std::string> priceTrend;
    for (const auto &commodity_name : commodities) {
        prices[commodity_name] = std::vector<double>();
    }
    for(auto &commodity_name : commodities)
    {
        priceTrend[commodity_name] = "";
    }   
    
    while (true) {
    

        sem_wait(sem_id, 2);  // Wait for items to be available
        sem_wait(sem_id, 0);  // Wait for access to buffer

        Commodity commodity = shared_buffer->buffer[shared_buffer->out];
        shared_buffer->out = (shared_buffer->out + 1) % buffer_size;

        sem_signal(sem_id, 0);  // Signal empty slot
        sem_signal(sem_id, 1);  // Signal availability of new items

        std::string name = commodity.name;
        double price = commodity.price;

        // Update prices and calculate average
        prices[name].push_back(price);
        if (prices[name].size() > 5) {
            prices[name].erase(prices[name].begin());
        }

        double avg_price = 0;
        for (double p : prices[name]) avg_price += p;
        avg_price /= prices[name].size();

        system("clear");

        std::cout << "+-------------------+--------+----------+" << std::endl;
        std::cout << "| Commodity         | Price  | AvgPrice |" << std::endl;
        std::cout << "+-------------------+--------+----------+" << std::endl;

        for (const auto &commodity_name : commodities) {
            std::string commodity_str(commodity_name);
    double current_price = prices[commodity_str].empty() ? 0.00 : prices[commodity_str].back();
    double avg_price = 0.00;

    if (!prices[commodity_str].empty()) {
        for (double p : prices[commodity_str]) avg_price += p;
        avg_price /= prices[commodity_str].size();
    }

    // Determine trend arrows and colors
    std::string avg_trend = (current_price > avg_price) ? "↑" : "↓";
    std::string arrow_color;
    if (prices[commodity_str].size() > 1) {
        priceTrend[commodity_str] = (current_price > prices[commodity_str][prices[commodity_str].size() - 2]) ? "↑" : "↓";
        arrow_color = (priceTrend[commodity_str] == "↑") ? GREEN : RED;
    } else {
        priceTrend[commodity_str] = " "; // No trend available for the first price
        arrow_color = BLUE;
    }

    // Display commodity data
    std::cout << "| " << std::setw(17) << commodity_str
              << " | " << std::fixed << std::setprecision(2) << std::setw(6) << current_price
              << arrow_color << priceTrend[commodity_str] 
              << " | " << std::setw(8) << avg_price 
              << ((avg_trend == "↑") ? GREEN : RED) << avg_trend << RESET
              << " |" << std::endl;
}

        std::cout << "+-------------------+--------+----------+" << std::endl;

        usleep(2000 * 1000);  // Refresh every 500ms
    }

    return 0;
}