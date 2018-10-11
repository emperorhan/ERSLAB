#include <cstdio>
#include <algorithm>
#include <queue>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "ssd_pc.h"

// #define GROUPSIZE	56
// #define N_LIMIT		1000

int windowSize = 16;	//6h마다 shift될 window size

double get30DaysSSDTotalChangeNum(int nlimit){
	return ((double)nlimit * MAX_SSD_SIZE / (12 * 5));
}
//i 번째에서 j 영화의 확률
double getP(int i, int j) {
	return P[(j - 1 + i * windowSize) % NUMBER_OF_VIDEO];
}
double hchange(int cnt, double th, int maxcnt) {
	priority_queue<pair<double, pair<int, int> > > minq;
	priority_queue<pair<double, pair<int, int> > > maxq;
	int ret = 0;
	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
		int last = hPrefix[cnt][i] = hPrefix[cnt - 1][i];
		
		if ((i - 1 + cnt*windowSize) % NUMBER_OF_VIDEO < windowSize) ret += last;	//새롭게 추가된 영화

		if (last != S[i])
			maxq.push({getP(cnt, i)*costTable[i][last], {i, last}});
		last--;
		if (last >= 0)
			minq.push({-getP(cnt, i)*costTable[i][last], {i, last}});
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
		if (minj != 0) minq.push({-getP(cnt, mini)*costTable[mini][minj - 1], {mini, minj - 1}});
		if ((maxj + 1) != S[maxi]) maxq.push({getP(cnt, maxi)*costTable[maxi][maxj + 1], {maxi, maxj + 1}});
	}
	return ret;
}
double getThreshold(double l, double r) {

	double mid = (l + r) / 2.0;

	if (l + 0.0005 >= r) {
		for (int i = 1; i <= CHANGE; i++) {
			hchange(i, mid, INF);
		}
		return mid;
	}
	
	int cnt = 0;
	for (int i = 1; i <= CHANGE; i++) {
		cnt += hchange(i, mid, totalChangeNum - cnt + 1);
		if (cnt > totalChangeNum) break;
	}

	if (cnt > totalChangeNum) return getThreshold(mid, r);
	else return getThreshold(l, mid);
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

string GetOutputFileName(int nlimit, int windowsize, int opt){
	string ret = opt == 0 ? "SPC_throttling_" : "SPC_";
    return ret + "prefix___(N_limit=" + to_string(nlimit) + ",windowSize=" + to_string(windowsize) + ").txt";
}

string GetThresholdOutputFileName(int nlimit, int windowsize, int opt){
	string ret = opt == 0 ? "SPC_throttling_" : "SPC_";
    return ret + "threshold___(N_limit=" + to_string(nlimit) + ",windowSize=" + to_string(windowsize) + ").txt";
}

int main(int argc, char* argv[]) {
	int n_limit = atoi(argv[1]);
	int groupsize = atoi(argv[2]);
	int opt = atoi(argv[3]);

	totalChangeNum = get30DaysSSDTotalChangeNum(n_limit);	//5년 쓴다 했을 때 30일 분 N_limit기준 N_limit{500, 750, 1000}

	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
		find_prob(costTable[i], S[i], 1 - THETA2);
		for(int j = 0; j < S[i]; j++) costTable[i][j] *= ALPHA;
	}

	for (int j = 1; j <= NUMBER_OF_VIDEO; j++) {
		for (int k = 0; k < S[j]; k++) {
			maxCost = max(maxCost, costTable[j][k]);
		}
	}

	priority_queue<pair<double, int> > pq;
	for (int i = 1; i <= NUMBER_OF_VIDEO; i++) {
		pq.push({getP(0, i)*costTable[i][0], i});
	}
	int temp = MAX_SSD_SIZE;
	while (temp--) {
		int cv = pq.top().second;
		pq.pop();
		hPrefix[0][cv]++;
		if (hPrefix[0][cv] != S[cv]) pq.push({getP(0, cv)*costTable[cv][hPrefix[0][cv]], cv});
	}
	

	//----------------------------------------------------�ϴ� getThreshold�� ������-----
	windowSize = groupsize;
	int oct;
	if(opt) oct = optcnt();
	else threshold = getThreshold(0.0, maxCost);
	
	freopen(GetThresholdOutputFileName(n_limit, groupsize, opt).c_str(), "w+", stdout);
	
	if(opt) printf("%d\n", oct);
	else printf("%lf\n", threshold);

	freopen(GetOutputFileName(n_limit, groupsize, opt).c_str(), "w+", stdout);
	
	for (int i = 1; i <= CHANGE; i++) {
		for (int j = 1; j <= NUMBER_OF_VIDEO; j++) {
			printf("%.8lf %d ", getP(i, j), hPrefix[i][j]);
		}
	}
	return 0;
}