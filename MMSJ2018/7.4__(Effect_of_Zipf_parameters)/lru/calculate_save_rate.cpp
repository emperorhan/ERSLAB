#include <cstdio>
#include <string>
#include <vector>
using namespace std;

vector<double> data;

string targetFileName(int period){
	return "LRU_zipf_Disk_bandwidth_saved___(N_CHANGE=" + to_string(period) + ").txt";
}

int main(){
    for(int nChange = 1; nChange <= 120; nChange++){
        freopen(targetFileName(nChange).c_str(), "r", stdin);
        double input;
        scanf("%lf", &input);
        data.push_back(input);
    }
    double sum = 0;
    for(auto that : data) sum += that;

    printf("%.1lf, %ld\n", sum / data.size(), data.size());
}