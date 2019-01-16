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

bool lru[NUMBER_OF_VIDEO + 3][7010];

int timelru[NUMBER_OF_VIDEO + 3][7010];

double lrusum, lruBandwidth;

vector<vector<pair<int,pair<int,int> > > > req;	//i초 요청 pair-> first번째 영화 second 현재 구간,끝나는 시간
priority_q lrupq;

string GetInputFileName(int period){
    return "LRU_poisson_" + to_string(period) + ".txt";
}

string GetOutputFileName(int ssdSize, int ssdBandwidthIdx, int period){
	return "LRU_Disk_bandwidth_saved___(N_CHANGE=" + to_string(period) + "_SSD=" + to_string(ssdSize) + "_ssdBandwidth=" + to_string(memSize[maxSSDidx[ssdBandwidthIdx]]) + "KB).txt";
}


int main(int argc, char* argv[]) {
	ssdSizeIdx = atoi(argv[1]);			//32MB기준 0.5=16384 1TB=32768 1.5=49152 2TB=65536
	int ssdBandwidthIdx = atoi(argv[2]);
    int nChange = atoi(argv[3]);
	freopen(GetInputFileName(nChange).c_str(), "r", stdin);
	freopen(GetOutputFileName(ssdSizeArr[ssdSizeIdx], ssdBandwidthIdx, nChange).c_str(), "w+", stdout);

	ssdSize = ssdSizeArr[ssdSizeIdx];
	MAX_SSD = (double)memSize[maxSSDidx[ssdBandwidthIdx]]; //0:2GB, 1:4GB

	req.resize(Simu_time + 10);
	int videoNum, start, end = 0;
	

	while (~scanf("%d%d%d", &videoNum, &start, &end)) {
		req[start].push_back({videoNum, {0, end}});
		only_disk_use[start] += B[videoNum];
		only_disk_use[end]   -= B[videoNum];
	}

	double sum = 0.0;
	for (int sec = 2; sec <= Simu_time; sec++) {
		only_disk_use[sec] += only_disk_use[sec - 1];
		lruBandwidth = 0;
		for (auto here : req[sec]) {
			int reqVideoIdx = here.first;
			int reqSec      = here.second.first;
			int reqEndTime  = here.second.second;
			if (reqEndTime > sec + 1) {
				req[sec + 1].push_back({reqVideoIdx, {reqSec + 1, reqEndTime}});
			}
			int reqCurSize = (B[reqVideoIdx] * reqSec) / _U;
			if (segcnt < ssdSize) {
				if (lru[reqVideoIdx][reqCurSize]) {
					lrupq.update(sec, reqVideoIdx, reqCurSize);
				}
				else {
					segcnt++;
					lru[reqVideoIdx][reqCurSize] = true;
					lrupq.push(sec, reqVideoIdx, reqCurSize);
				}
			}
			else {
				if (!lru[reqVideoIdx][reqCurSize]) {
					lrusum += B[reqVideoIdx];
					lru[lrupq.top().vi][lrupq.top().vj] = false;
					lrupq.pop();
					lru[reqVideoIdx][reqCurSize] = true;
					lrupq.push(sec, reqVideoIdx, reqCurSize);
					timelru[reqVideoIdx][reqCurSize] = sec;
				}
				else {
					if (lruBandwidth < MAX_SSD) {
						lruBandwidth += B[reqVideoIdx];
						if (timelru[reqVideoIdx][reqCurSize] == sec - 1) {
							timelru[reqVideoIdx][reqCurSize] += 1;
							lrusum += B[reqVideoIdx];
						}
					}
					else {
						lrusum += B[reqVideoIdx];
					}
					lrupq.update(sec, reqVideoIdx, reqCurSize);
				}
			}
		}
		if (segcnt >= ssdSize)
			sum += only_disk_use[sec];
		if (sec == 43200)break;
	}
	// printf("%.0lf %.0lf\n", sum, lrusum); //disk, lfu, lru
	printf("%.1lf\n", ((sum - lrusum)*100) / sum);
	return 0;
}