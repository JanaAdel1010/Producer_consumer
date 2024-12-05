#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include <random>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

struct CommodityType {
    char name[10];        
    double prices[5];     
    int priceCount;        
    double currentPrice;   
    double avgPrice;       
    int lastIndex;         
};

struct SharedBuffer {
    CommodityType commodities[10];
};