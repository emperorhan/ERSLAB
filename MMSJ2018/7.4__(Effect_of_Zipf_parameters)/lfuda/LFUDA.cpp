#include <cstdio>
#include <algorithm>
#include <vector>
#include <queue>
#include "ssd_pc.h"
using namespace std;

#define Simu_time 21600		//시뮬레이션 돌릴 시간

int maxSSDidx[2] = {1, 2}; //2GB, 4GB memSize 참고
int ssdSize;


int segcnt;	//세그먼트 찬 기준

struct node{
	int data;
	int vi;
	int vj;
	node() :data(0), vi(0), vj(0){}
};
struct priority_q{
	node heap[MAX_SSD_SIZE + 10];
	int idx[NUMBER_OF_VIDEO + 3][3010];
	int size;
	priority_q() :size(0){}

	node top(){
		return heap[1];
	}
	void swap(int idx1, int idx2) {
		int vi1 = heap[idx1].vi;
		int vj1 = heap[idx1].vj;
		int val1 = heap[idx1].data;
		int vi2 = heap[idx2].vi;
		int vj2 = heap[idx2].vj;
		idx[vi1][vj1] = idx2;
		idx[vi2][vj2] = idx1;
		heap[idx1].data = heap[idx2].data;
		heap[idx1].vi = vi2;
		heap[idx1].vj = vj2;
		heap[idx2].data = val1;
		heap[idx2].vi = vi1;
		heap[idx2].vj = vj1;
	}
	void upheap(int cidx) {
		if (cidx == 1) return;
		int pidx = cidx / 2;
		if (heap[pidx].data <= heap[cidx].data) return;
		swap(cidx, pidx);
		upheap(pidx);
	}
	void downheap(int cidx) {
		int lidx = cidx * 2;
		if (lidx > size) return;
		if (lidx + 1 <= size) {
			if (heap[lidx].data > heap[lidx + 1].data) lidx++;
		}
		if (heap[cidx].data <= heap[lidx].data) return;
		swap(cidx, lidx);
		downheap(lidx);
	}
	void pop(){
		if (size == 0) return;
		heap[1].data = heap[size].data;
		heap[1].vi = heap[size].vi;
		heap[1].vj = heap[size].vj;
		idx[heap[1].vi][heap[1].vj] = 1;
		size--;
		downheap(1);
	}
	void push(int val, int ci, int cj) {
		size++;
		heap[size].data = val;
		heap[size].vi = ci;
		heap[size].vj = cj;
		idx[ci][cj] = size;
		upheap(size);
	}
	void update(int val, int ci, int cj) {
		int hidx = idx[ci][cj];
		heap[hidx].data = val;
		downheap(hidx);
	}
};

bool lfu[NUMBER_OF_VIDEO + 3][7010];

int timelfu[NUMBER_OF_VIDEO + 3][7010];
int Flfu[NUMBER_OF_VIDEO + 3][7010];
ll C;

double lfusum, lfuBandwidth;

vector<vector<pair<int,pair<int,int> > > > req;	//i초 요청 pair-> first번째 영화 second 현재 구간,끝나는 시간
priority_q lfupq;

string GetInputFileName(int period, double theta){
    return "LFUDA_zipf_poisson_" + to_string(period) + "(THETA=" + to_string(theta) + ").txt";
}

string GetOutputFileName(int thetaIdx, int ssdBandwidthIdx, int period){
	string ret = "./";
	if(thetaIdx == 0){
		if(ssdBandwidthIdx == 0) 
			ret += "0.2_2GB/";
		else 
			ret += "0.2_4GB/";
	}
	else if(thetaIdx == 1){
		if(ssdBandwidthIdx == 0) 
			ret += "0.4_2GB/";
		else 
			ret += "0.4_4GB/";
	}
	else if(thetaIdx == 2){
		if(ssdBandwidthIdx == 0) 
			ret += "0.6_2GB/";
		else 
			ret += "0.6_4GB/";
	}
	else if(thetaIdx == 3){
		if(ssdBandwidthIdx == 0) 
			ret += "0.8_2GB/";
		else 
			ret += "0.8_4GB/";
	}
	return ret + "LFUDA_zipf_Disk_bandwidth_saved___(N_CHANGE=" + to_string(period) + ").txt";
}

double thetaArr[4] = {0.2, 0.4, 0.6, 0.8};
int dramArr[2] = {_0GB, _16GB};

bool changelife[NUMBER_OF_VIDEO + 3];
int lifecnt;
int main(int argc, char* argv[]) {
	int thetaIdx = atoi(argv[1]);		//THETA => 0:0.2 1:0.4 2:0.6 3:0.8
	double theta = thetaArr[thetaIdx]; 	
	int ssdBandIdx = atoi(argv[2]);		//ssdBandwidth => 0:2GB/s 1:4GB/s
	ssdSizeIdx = 3;			//32MB기준 0.5=16384 1TB=32768 1.5=49152 2TB=65536
    int nChange = atoi(argv[3]);

	freopen(GetInputFileName(nChange, theta).c_str(), "r", stdin);
	freopen(GetOutputFileName(thetaIdx, ssdBandIdx, nChange).c_str(), "w+", stdout);

	ssdSize = ssdSizeArr[ssdSizeIdx];
	MAX_SSD = (double)memSize[maxSSDidx[ssdBandIdx]]; //0:2GB, 1:4GB

	req.resize(Simu_time + 10);
	int videoNum, start, end = 0;
	

	while (~scanf("%d%d%d", &videoNum, &start, &end)) {
		req[start].push_back({videoNum, {0, end}});
		only_disk_use[start] += B[videoNum];
		only_disk_use[end]   -= B[videoNum];
	}

	double sum = 0.0;
	for (int sec = 2; sec <= Simu_time; sec++) {
		if (C<0) {
			printf("C가 너무 커짐 %lld %d\n", C, sec);
			break;
		}
		only_disk_use[sec] += only_disk_use[sec - 1];
		lfuBandwidth = 0;
		if (sec == 21601) {
			for (int fi = 961; fi <= 1000; fi++) {
				for (int fj = 0; fj < 7000; fj++) {
					Flfu[fi][fj] = 0;
				}
			}
		}
		for (auto here : req[sec]) {
			int reqVideoIdx = here.first;
			int reqSec      = here.second.first;
			int reqEndTime  = here.second.second;
			if (sec > 21600) {
				if (reqVideoIdx <= 32) reqVideoIdx += 968;
				else reqVideoIdx -= 32;
			}
			if (reqEndTime > sec + 1) {
				req[sec + 1].push_back({reqVideoIdx, {reqSec + 1, reqEndTime}});
			}
			int reqCurSize = (B[reqVideoIdx] * reqSec) / _U;
			Flfu[reqVideoIdx][reqCurSize]++;

			if (segcnt < ssdSize) {
				if (lfu[reqVideoIdx][reqCurSize]) {
					lfupq.update(Flfu[reqVideoIdx][reqCurSize] + C, reqVideoIdx, reqCurSize);
				}
				else {
					segcnt++;
					lfu[reqVideoIdx][reqCurSize] = true;
					lfupq.push(Flfu[reqVideoIdx][reqCurSize] + C, reqVideoIdx, reqCurSize);
				}
			}
			else {
				if (!lfu[reqVideoIdx][reqCurSize]) {
					lfusum += B[reqVideoIdx];
					if (Flfu[reqVideoIdx][reqCurSize] + C > lfupq.top().data) {
						C = lfupq.top().data;
						lfu[lfupq.top().vi][lfupq.top().vj] = false;
						lfupq.pop();
						lfu[reqVideoIdx][reqCurSize] = true;
						lfupq.push(Flfu[reqVideoIdx][reqCurSize] + C, reqVideoIdx, reqCurSize);
						timelfu[reqVideoIdx][reqCurSize] = sec;
						if (sec > 21600) lifecnt++;
					}
				}
				else {
					if (sec > 21600 && reqVideoIdx > 961) {
						if (!changelife[reqVideoIdx]) {
							changelife[reqVideoIdx] = true;
							lifecnt++;
						}
					}
					if (lfuBandwidth < MAX_SSD) {
						lfuBandwidth += B[reqVideoIdx];
						if (timelfu[reqVideoIdx][reqCurSize] == sec - 1) {
							timelfu[reqVideoIdx][reqCurSize] += 1;
							lfusum += B[reqVideoIdx];
						}
					}
					else {
						lfusum += B[reqVideoIdx];
					}
					lfupq.update(Flfu[reqVideoIdx][reqCurSize] + C, reqVideoIdx, reqCurSize);
				}
			}
		}
		if (segcnt >= ssdSize)
			sum += only_disk_use[sec];
		if (sec == 43200)break;
	}
	// printf("%.0lf %.0lf\n", sum, lfusum); //disk, lfu, lru
	printf("%.1lf\n", ((sum - lfusum)*100) / sum);
	return 0;
}