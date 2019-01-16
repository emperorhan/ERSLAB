#include <cstdio>
#include <algorithm>
#include <vector>
#include <queue>
#include <string>
#include "ssd_pc.h"

#define HERE 3600

int maxSSDidx[2] = {1, 2}; //2GB, 4GB memSize 참고

//cur_req[c_i][c_j]에 대하여
void new_dram_cash(int c_i, int c_j)
{
	//disk 대기 큐에 넣어주는 역할
	if (c_j == 0 || MAX_DRAM == 0)
		return; //이전 요청이 없는 경우
	/*
	여기서 이제 이전요청이랑 차이로 캐쉬 할 수 있는지 보고 disk대기 큐에 넣어줌
	*/
	int cv = cur_req[c_i][c_j];
	if (request[cv].start - request[cur_req[c_i][c_j - 1]].start > request[cv].end - request[cv].start)
		return; //interval 불가상황
	double inter_s = B[c_i] * (double)(request[cv].start - request[cur_req[c_i][c_j - 1]].start);
	request[cv].inter_size = inter_s;
	disk_q.push({-inter_s, cv}); //disk큐에 넣어줌
}
void delete_finish(int cur_ti)
{
	for (int i = 1; i <= NUMBER_OF_VIDEO; i++)
	{
		for (int j = 0; j < cur_req[i].size(); j++)
		{
			int c_v = cur_req[i][j];
			if (request[c_v].end == cur_ti)
			{
				//끝난경우
				request[c_v].is_finish = true; //끝났다고 변경
				if (request[c_v].ssd_suport)
					ssd_use -= B[i]; //ssd사용중이였으면 ssd_bandwith에서 제거
				else if (request[c_v].dram_suport)
					dram_use -= request[c_v].inter_size;  //interval_caching중이였으면 dram에서 제거
				cur_req[i].erase(cur_req[i].begin() + j); //삭제
				if (j == cur_req[i].size())
					break;
				c_v = cur_req[i][j];
				if (request[c_v].dram_suport)
				{
					dram_use -= request[c_v].inter_size; //dram에서 캐쉬 제거
					request[c_v].dram_suport = false;
					new_dram_cash(i, j);
				}
				j--; //현재요청 다시 확인해야 하므로
			}
			else if (request[c_v].ssd_suport)
			{
				//ssd 지원 중
				if (cur_ti - request[c_v].start == prefix[i])
				{
					//prefix가 끝난경우
					request[c_v].ssd_suport = false;
					ssd_use -= B[i]; //ssd에서 제거
					new_dram_cash(i, j);
				}
			}
		}
	}
}
void add_cur_req(int cur_ti)
{
	for (int i = 0; i < st_req[cur_ti].size(); i++)
	{
		int cv = st_req[cur_ti][i];
		int ci = request[cv].vi_num;
		double inter_s;
		if (cur_req[ci].empty())
			inter_s = MAX_DRAM;
		else
		{
			int pres = request[cur_req[ci][cur_req[ci].size() - 1]].start;
			inter_s = B[ci] * (double)(request[cv].start - pres);
		}
		add_req.push({inter_s, cv});
	}
}
void add_new_req()
{
	while (add_req.size())
	{
		int cv = add_req.top().second;
		add_req.pop();
		int ci = request[cv].vi_num;
		if ((prefix[ci] != 0) && (ssd_use + B[ci] < MAX_SSD))
		{
			ssd_use += B[ci]; //ssd에 추가
			request[cv].ssd_suport = true;
			cur_req[ci].push_back(cv); //현재 요청목록에 추가
		}
		else
		{
			cur_req[ci].push_back(cv);				   //현재 요청목록에 추가
			new_dram_cash(ci, cur_req[ci].size() - 1); //new_dram_cash를 통해 interval이 일어날 수 있도록 함
		}
	}
}
double count_disk_use(int cur_ti)
{
	double ret = 0.0;
	for (int i = 1; i <= NUMBER_OF_VIDEO; i++)
	{
		for (int j = 0; j < cur_req[i].size(); j++)
		{
			int cv = cur_req[i][j];
			if (!request[cv].ssd_suport)
			{
				//disk에서 지원되는 경우
				if (request[cv].dram_suport)
				{
					//interval_cash가 이뤄지고 있는 경우 disk에서 보내는 부분이 있으면 추가
					if (request[cv].inter_size > B[i] * ((double)(cur_ti - request[cv].start)))
					{
						ret += B[i];
						cache_disk_req[cur_ti]++;
					}
				}
				else
				{
					ret += B[i];
					cache_disk_req[cur_ti]++;
				}
			}
		}
	}
	return ret;
}
void dram_caching()
{
	//disk_q에서 가능한 것들 dram caching 진행
	while (disk_q.size())
	{
		int cv = disk_q.top().second;
		double interval = -disk_q.top().first;
		if (request[cv].is_finish || request[cv].dram_suport)
		{
			disk_q.pop();
			continue;
		}
		int ci = request[cv].vi_num;
		int cj;
		for (cj = 0; cj < cur_req[ci].size(); cj++)
			if (cur_req[ci][cj] == cv)
				break;
		if (cj == 0)
		{
			disk_q.pop();
			continue;
		}
		if ((B[ci] * (double)(request[cv].start - request[cur_req[ci][cj - 1]].start)) != interval)
		{
			disk_q.pop();
			continue;
		}
		if (dram_use + interval > MAX_DRAM)
			return;
		dram_use += interval;
		request[cv].dram_suport = true;
		disk_q.pop();
	}
}
void simulation()
{
	for (int i = 0; i <= SIMULATION_TERM; i++)
	{
		delete_finish(i); //종료된 요청들을 삭제시키고 ssd에서 disk로 변환된 것 처리
		add_cur_req(i);   //현재 새롭게 추가되는 요청들을 add_req p_q에 추가한다
		add_new_req();	//새로운 요청을 알고리즘에 맞춰 추가하는 함수
		dram_caching();
		disk_use[i] = count_disk_use(i);
		ssd_b[i] = ssd_use;
	}
}

string GetInputFileName(double theta){
    return "SPC_zipf_poisson___(THETA=" + to_string(theta) + ").txt";
}

string GetOutputFileName(double theta, int ssdBand, int dramFlag){
	string ret = dramFlag == 0 ? "" : "_DRAM";
	return "SPC_zipf" + ret + "_Disk_bandwidth_saved___(THETA=" + to_string(theta) + "_ssdBandwidth=" + to_string(ssdBand) + "KB).txt";
}

double thetaArr[4] = {0.2, 0.4, 0.6, 0.8};
int dramArr[2] = {_0GB, _16GB};

int main(int argc, char* argv[]) {
	int thetaIdx = atoi(argv[1]);		//THETA => 0:0.2 1:0.4 2:0.6 3:0.8
	double theta = thetaArr[thetaIdx]; 	
	int ssdBandIdx = atoi(argv[2]);		//ssdBandwidth => 0:2GB/s 1:4GB/s
	int dramIdx = dramArr[atoi(argv[3])];	//0:SPC 1:DRAM(16GB)

	MAX_DRAM = (double)memSize[dramIdx];
	MAX_SSD = (double)memSize[maxSSDidx[ssdBandIdx]]; //0:2GB, 1:4GB

	freopen(GetInputFileName(theta).c_str(), "r", stdin);
	freopen(GetOutputFileName(theta, MAX_SSD, dramIdx).c_str(), "w+", stdout);
	
	

	for (int period = 1; period <= CHANGE; period++)
	{
		int ti, ts, te, idx = 0;
		for (int i = 1; i <= NUMBER_OF_VIDEO; i++)
		{
			scanf("%d", &prefix[i]);
			prefix[i] *= 32;
			prefix[i] /= B[i]; //초 단위로 변경
		}
		request.clear();
		for (int i = 0; i <= SIMULATION_TERM; i++)
			st_req[i].clear();
		for (int i = 0; i <= NUMBER_OF_VIDEO; i++)
			cur_req[i].clear();
		while (pQueue.size())
			pQueue.pop();
		while (add_req.size())
			add_req.pop();
		while (disk_q.size())
			disk_q.pop();

		ssd_use = 0;
		dram_use = 0;
		for (int i = 0; i <= SIMULATION_TERM + 3600; i++) //이부분 HERE 원래 10000이었음
		{
			only_disk_use[i] = 0;
			disk_use[i] = 0;
			disk_req[i] = 0;
			cache_disk_req[i] = 0;
			disk_s[i] = 0;
			cache_s[i] = 0;
			ssd_b[i] = 0;
		}
		disk_sum = 0;

		while (~scanf("%d%d%d", &ti, &ts, &te))
		{
			if (ti == 987654321 && ts == 987654321)
				break;
			request.push_back(node1(ti, ts, te));
			st_req[request[idx].start].push_back(idx);
			idx++;
			only_disk_use[ts] += B[ti];
			only_disk_use[te] -= B[ti];
			disk_req[ts]++;
			disk_req[te]--;
		}
		double sum = 0.0;
		double disk_min = 987654321.0;
		double disk_max = 0.0;
		for (int i = 2; i <= SIMULATION_TERM; i++)
		{
			only_disk_use[i] += only_disk_use[i - 1];
			disk_req[i] += disk_req[i - 1];
			if (i >= HERE)
				sum += only_disk_use[i];
			if ((i >= HERE) && (i <= SIMULATION_TERM))
			{
				disk_min = min(disk_min, only_disk_use[i]);
				disk_max = max(disk_max, only_disk_use[i]);
			}
		}
		/*
		printf("%.0lf\n", sum); //disk만 사용했을 때,
		printf("%lf %lf\n", disk_min, disk_max);
		*/
		disk_sum = sum;

		/*
		prefix_temp[  0,1,2 순으로 순서, 0/1, segment 방식
		memSize의 값으로 용량 설정 _0GB, _4GB, _8GB, _16GB       old => 0,256(1),512(2),3072(3),1024(4),2048(5),4096(6),8192(7)
		*/

		simulation();
		sum = 0.0;
		for (int t = HERE; t <= SIMULATION_TERM; t++)
			sum += disk_use[t];
		printf("%.0lf %0.4lf\n", sum, 1.0 - (sum / disk_sum));

		total_sum += sum;
		total_disk_sum += disk_sum;
/*
		double disk_w_sum = 0.0;
		double cache_w_sum = 0.0;
		//전력을 위해 대역폭 : only_disk_use , disk_use 요청수 : disk_req , cache_disk_req 처리필요시간: disk_s, cache_s
		for (int t = HERE; t <= (END_SIMULATION - HERE); t++)
		{
			disk_s[t] = ((only_disk_use[t] * 2.0) / 226.0 + ((double)disk_req[t]) * 0.00416) / 36.0;
			cache_s[t] = ((disk_use[t] * 2.0) / 226.0 + ((double)cache_disk_req[t]) * 0.00416) / 36.0;
			disk_w_sum += disk_s[t] * 11.1 + (2.0 - disk_s[t]) * 6.9;
			cache_w_sum += cache_s[t] * 11.1 + (2.0 - cache_s[t]) * 6.9;
			double ssd_per = (ssd_b[t] / MAX_SSD);
			cache_w_sum += ((ssd_per * 2.3) * 1.5 + ((1.0 - ssd_per) * 0.05) * 1.5) / 36.0;
		}
		printf("%lf %lf %lf", disk_w_sum, cache_w_sum, (1.0 - cache_w_sum / disk_w_sum) * 100);
		*/
	}
	printf("\n\n%.0lf %lf\n\n", total_sum, 1.0 - (total_sum / total_disk_sum));
	return 0;
}