#include <cstdio>
#include <algorithm>
#include <queue>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "ssd_pc.h"

int windowSize = 16;	//6h마다 shift될 window size
int thetaIdx;

//i 번째에서 j 영화의 확률
double getP(int thetaIdx, int i, int j) {
	return P[thetaIdx][(j - 1 + i * windowSize) % NUMBER_OF_VIDEO];
}
double hchange(int cnt, double th, int maxcnt) {
	priority_queue<pair<double, pair<int, int> > > minq;
	priority_queue<pair<double, pair<int, int> > > maxq;
	int ret = 0;
	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
		int last = hPrefix[cnt][i] = hPrefix[cnt - 1][i];
		
		if ((i - 1 + cnt*windowSize) % NUMBER_OF_VIDEO < windowSize) ret += last;	//새롭게 추가된 영화

		if (last != S[i])
			maxq.push({getP(thetaIdx, cnt, i)*costTable[i][last], {i, last}});
		last--;
		if (last >= 0)
			minq.push({-getP(thetaIdx, cnt, i)*costTable[i][last], {i, last}});
	}
	while (maxcnt--) {
		if (minq.empty() || maxq.empty()) break;
		double cost = minq.top().first + maxq.top().first;
		if (cost < th) break;
		if (cost == 0) break;
		ret++;

		int mini = minq.top().second.first;
		int minj = minq.top().second.second;
		minq.pop();
		int maxi = maxq.top().second.first;
		int maxj = maxq.top().second.second;
		maxq.pop();
		hPrefix[cnt][mini]--;
		hPrefix[cnt][maxi]++;
		if (minj != 0) minq.push({-getP(thetaIdx, cnt, mini)*costTable[mini][minj - 1], {mini, minj - 1}});
		if ((maxj + 1) != S[maxi]) maxq.push({getP(thetaIdx, cnt, maxi)*costTable[maxi][maxj + 1], {maxi, maxj + 1}});
	}
	return ret;
}

//optimal 버전에서 기간동안 변경횟수 카운트
int optcnt() {
	int ret = 0;
	for (int i = 1; i <= CHANGE; i++) {
		int temp = hchange(i, 0, INF);
		ret += temp;
	}
	return ret;
}

string GetOutputFileName(double theta){
    return "SPC_zipf_prefix___(THETA=" + to_string(theta) + ").txt";
}

double thetaArr[4] = {0.2, 0.4, 0.6, 0.8};

int main(int argc, char* argv[]) {
	ssdSizeIdx = 3; //32MB기준 2TB=65536
	thetaIdx = atoi(argv[1]);		//THETA => 0:0.2 1:0.4 2:0.6 3:0.8
	double theta = thetaArr[thetaIdx]; 	
	freopen(GetOutputFileName(theta).c_str(), "w+", stdout);
	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
		find_prob(costTable[i], S[i], 1 - THETA2);
		for(int j = 0; j < S[i]; j++) costTable[i][j] *= ALPHA;
	}
	priority_queue<pair<double, int> > pq;
	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
		pq.push({getP(thetaIdx, 0, i)*costTable[i][0], i});
	}
	int temp = ssdSizeArr[ssdSizeIdx];
	while (temp--) {
		int cv = pq.top().second;
		pq.pop();
		hPrefix[0][cv]++;
		if (hPrefix[0][cv] != S[cv]) pq.push({getP(thetaIdx, 0, cv)*costTable[cv][hPrefix[0][cv]], cv});
	}
	
	//----------------------------------------------------�ϴ� getThreshold�� ������-----
	windowSize = 40;
	optcnt();
	
	for (int i = 1; i <= CHANGE; i++) {
		for (int j = 1; j <= NUMBER_OF_VIDEO; j++) {
			printf("%.8lf %d ", getP(thetaIdx, i, j), hPrefix[i][j]);
		}
	}
	return 0;
}