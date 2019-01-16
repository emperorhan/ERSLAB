#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "ssd_pc.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

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


//segment의 populality에 따른 가중치
double segWeight(int len, int i) {
	if (len == 0) return 0;
	return cumu_segP[i][len - 1];//  / (((double)len) / ((double)D[i])) 생략 해봤음
}

string GetInputFileName(int nlimit, int windowsize, int opt){
	string ret = opt == 0 ? "SPC_throttling_" : "SPC_";
    return ret + "prefix___(N_limit=" + to_string(nlimit) + ",windowSize=" + to_string(windowsize) + ").txt";
}

string GetOutputFileName(int nlimit, int windowsize, int opt){
	string ret = opt == 0 ? "SPC_throttling_" : "SPC_";
    return ret + "poisson___(N_limit=" + to_string(nlimit) + ",windowSize=" + to_string(windowsize) + ").txt";
}

int main(int argc, char* argv[]) {
	int n_limit = atoi(argv[1]);
	int groupsize = atoi(argv[2]);
	int opt = atoi(argv[3]);
	freopen(GetInputFileName(n_limit, groupsize, opt).c_str(), "r", stdin);
	freopen(GetOutputFileName(n_limit, groupsize, opt).c_str(), "w+", stdout);

	double tem = 0;
	// CHANGE(120) 6시간마다 바뀌니까 6 * 120 = 720hours => 30days
	for (int period = 1; period <= CHANGE; period++) {
		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
			scanf("%lf %d ", &curP[i - 1], &prefix[i]);
		}
		
		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {	
			find_prob(cumu_segP[i], L[i], 0.8);										//영화 세그먼트 확률 1 ~ D[i]까지 zipf
			for (int j = 1; j < L[i]; j++) cumu_segP[i][j] += cumu_segP[i][j - 1];	//누적확률 설정
		}
		
        double req = _requestRate[(period - 1)%4];										//6h마다 requestRate를 변화시킴 (24h 기준으로 반복)
        for (int j = 1; j <= NUMBER_OF_VIDEO; j++) R[j] = req * curP[j - 1];
    
		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) printf("%d ", prefix[i]);			//////////////각 영화 0/1 prefix 출력//////////////////////

		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) init_poisson(i);
		
		printf("%d %d %d ", 987654321, 987654321, 987654321); 						//회차 나누는 기준
	}
	return 0;
}