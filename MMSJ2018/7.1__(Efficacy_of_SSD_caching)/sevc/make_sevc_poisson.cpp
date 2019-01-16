#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "ssd_pc.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

int ssdSize;
int windowSize = 40;	//6h마다 shift될 window size

//i 번째에서 j 영화의 확률
double getP(int i, int j) {
	return P[(j - 1 + i * windowSize) % NUMBER_OF_VIDEO];
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

string GetOutputFileName(int ssdSize){
    return "SEVC_poisson___(ssdSize=" + to_string(ssdSize) + ").txt";
}

int main(int argc, char* argv[]) {
	ssdSizeIdx = atoi(argv[1]);			//32MB기준 0.5=16384 1TB=32768 1.5=49152 2TB=65536
	freopen(GetOutputFileName(ssdSizeArr[ssdSizeIdx]).c_str(), "w+", stdout);
	ssdSize = ssdSizeArr[ssdSizeIdx];
	// CHANGE(120) 6시간마다 바뀌니까 6 * 120 = 720hours => 30days
	for (int period = 1; period <= CHANGE; period++) {
		memset(prefix, 0, sizeof(prefix));
		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
			curP[i - 1] = getP(period, i);
		}
		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {	
			find_prob(cumu_segP[i], L[i], 0.8);										//영화 세그먼트 확률 1 ~ L[i]까지 zipf
			for (int j = 1; j < L[i]; j++) cumu_segP[i][j] += cumu_segP[i][j - 1];	//누적확률 설정
		}
		
        double req = _requestRate[(period - 1)%4];										//6h마다 requestRate를 변화시킴 (24h 기준으로 반복)
        for (int j = 1; j <= NUMBER_OF_VIDEO; j++) R[j] = req * curP[j - 1];

		priority_queue<pair<double, int> > pq;
		for(int i = 1; i <= NUMBER_OF_VIDEO; i++){
			pq.push({getP(period, i), i});
		}
		int popu = 0;
		while(pq.size()) {
			int here = pq.top().second;
			pq.pop();
			
			int temp = S[here] + 1;
			popu += temp;
			if (popu <= ssdSize) prefix[here] = L[here];
			else break;
		}

		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) printf("%d ", prefix[i]);			//////////////각 영화 0/1 prefix 출력//////////////////////

		for (int i = 1; i <= NUMBER_OF_VIDEO; i++) init_poisson(i);
		
		printf("%d %d %d ", 987654321, 987654321, 987654321); 						//회차 나누는 기준
	}
	return 0;
}