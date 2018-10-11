#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    for(int ssdSizeIdx = 0; ssdSizeIdx <= 3; ssdSizeIdx++){
        for(int ssdBandwidthIdx = 0; ssdBandwidthIdx <= 1; ssdBandwidthIdx++){
            for(int nChange = 1; nChange <= 120; nChange++){
                string poissonCommand = "./1 " + to_string(nChange);
                system(poissonCommand.c_str());
            }
        }
    }
}