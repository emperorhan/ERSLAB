#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    int ssdSizeIdx = atoi(argv[1]);			//32MB기준 0.5=16384 1TB=32768 1.5=49152 2TB=65536
	int ssdBandwidthIdx = atoi(argv[2]);
    for(int nChange = 1; nChange <= 120; nChange++){
        string lruCommand = "./2 " + to_string(ssdSizeIdx) + " " + to_string(ssdBandwidthIdx) + " " + to_string(nChange);
        system(lruCommand.c_str());
    }
}