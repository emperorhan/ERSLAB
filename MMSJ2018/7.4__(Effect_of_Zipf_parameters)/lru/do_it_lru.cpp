#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    for(int theta = 0; theta <= 3; theta++){
        for(int ssdBandwidthIdx = 0; ssdBandwidthIdx <= 1; ssdBandwidthIdx++){
            for(int nChange = 1; nChange <= 120; nChange++){
                string lruCommand = "./2 " + to_string(theta) + " " + to_string(ssdBandwidthIdx) + " " + to_string(nChange);
                system(lruCommand.c_str());
            }
        }
    }
}