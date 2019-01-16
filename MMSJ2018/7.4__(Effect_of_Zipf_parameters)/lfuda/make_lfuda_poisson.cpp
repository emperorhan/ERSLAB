#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "ssd_pc.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

int windowSize = 40;	//6h마다 shift될 window size

//i 번째에서 j 영화의 확률
double getP(int thetaIdx, int i, int j) {
	return P[thetaIdx][(j - 1 + i * windowSize) % NUMBER_OF_VIDEO];
}

void init_poisson(int i) {
	/*
	i번 영화에 대하여 람다를 기준으로하여
	랜덤한 넥스트 값을 계속 생성하여 다음도착 시간을 설정한다.
	poisson과 poisson size를 설정해준다.
	*/

	const gsl_rng_type* T;
	T = gsl_rng_default;
	gsl_rng* r;
	r = gsl_rng_alloc(T);

	int temp = 0;
	while (1) {
		temp += (gsl_ran_poisson(r, 1.0 / R[i]) + 10);
		int view_ran = (int)((double)L[i] * (((double)zipf_rand[rand() % 20000]) / 2000.0));
		if (temp > SIMULATION_TERM) break;
		printf("%d %d %d ", i, temp, temp + view_ran);      //영화번호 시작시간 끝시간 출력
	}
}

string GetOutputFileName(int period, double theta){
    return "LFUDA_zipf_poisson_" + to_string(period) + "(THETA=" + to_string(theta) + ").txt";
}

double thetaArr[4] = {0.2, 0.4, 0.6, 0.8};

int main(int argc, char* argv[]) {
	int thetaIdx = atoi(argv[1]);		//THETA => 0:0.2 1:0.4 2:0.6 3:0.8
	int nChange = atoi(argv[2]);
	double theta = thetaArr[thetaIdx]; 	
	freopen(GetOutputFileName(nChange, theta).c_str(), "w+", stdout);

	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) curP[i - 1] = getP(thetaIdx, nChange - 1, i);
	
	double req = _requestRate[(nChange - 1)%4];	//6h마다 requestRate를 변화시킴 (24h 기준으로 반복)

	for (int j = 1; j <= NUMBER_OF_VIDEO; j++) R[j] = req * curP[j - 1];

	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) init_poisson(i);




	// // CHANGE(120) 6시간마다 바뀌니까 6 * 120 = 720hours => 30days
	// for (int period = 1; period <= 120; period++) {
	// 	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) curP[i - 1] = getP(period - 1, i);
		
    //     double req = _requestRate[(period - 1)%4];										//6h마다 requestRate를 변화시킴 (24h 기준으로 반복)
    //     for (int j = 1; j <= NUMBER_OF_VIDEO; j++) R[j] = req * curP[j - 1];

	// 	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) init_poisson(i, period);

	// 	for(int i=1; i<=NUMBER_OF_VIDEO; i++){
	// 		for(auto that : vec[i]){
	// 			printf("%d %d %d ", i, that.first, that.second);
	// 		}
	// 	}
		
	// 	// printf("%d %d %d ", 987654321, 987654321, 987654321); 						//회차 나누는 기준
	// }
	return 0;
}