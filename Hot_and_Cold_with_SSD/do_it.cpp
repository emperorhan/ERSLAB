#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    int workload = atoi(argv[1]);
    int ssdSize = atoi(argv[2]);
    int diskSize = atoi(argv[3]);

    if(!diskSize){ // 2TB
        for(int disk=12; disk<=30; disk+=2){
            string command = "./1 " + to_string(disk) + " " + to_string(workload) + " " + to_string(ssdSize) + " " + to_string(diskSize);
            system(command.c_str());
        }
    }
    else{
        for(int disk=11; disk<=20; disk++){
            string command = "./1 " + to_string(disk) + " " + to_string(workload) + " " + to_string(ssdSize) + " " + to_string(diskSize);
            system(command.c_str());
        }
    }
}