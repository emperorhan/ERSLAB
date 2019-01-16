#include "ssd_pc.h"

double popular[4][NUMBER_OF_VIDEO + 10];
double theta[4] = {0.2, 0.4, 0.6, 0.8};

int main(){
    for(int i=0; i<4; i++){
        find_prob(popular[i], NUMBER_OF_VIDEO, (1 - theta[i]));
        printf("{");
        for(int j=0; j<NUMBER_OF_VIDEO; j++){
            printf("%lf, ", popular[i][j]);
        }
        printf("}, ");
    }
}