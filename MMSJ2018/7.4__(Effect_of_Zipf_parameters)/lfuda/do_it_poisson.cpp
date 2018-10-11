#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    for(int thetaIdx = 0; thetaIdx <= 3; thetaIdx++){
        for(int nChange = 1; nChange <= 120; nChange++){
            string poissonCommand = "./1 " + to_string(thetaIdx) + " " + to_string(nChange);
            system(poissonCommand.c_str());
        }
    }
}