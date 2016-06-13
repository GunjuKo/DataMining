#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <algorithm>
#include <sstream>

using namespace std;

#define MINPTS 5
#define DEFAULT_RADIOUS 10.0

struct data{
	int		id;											// id 값

	double	x;											// x 값
	double	y;											// y 값

	bool	visited;									// 방문 여부
	bool	clustered;									// 클러스터링 된 경우에 true, 그렇지 않으면 false
	
	int		clusterId;

	vector<pair<double, int> > neighbors;				// 주변 object와의 거리를 저장(거리, object의 id)

	data() : visited(false), clustered(false){}
};

int numberOfClusters;									// The number of clusters
int minPts		= MINPTS;								// the neighborhood density threshold
double radius	= DEFAULT_RADIOUS;
map<int, data> datas;									// set of input data objects
vector<data>   datasVector;								// This value is used for initialize the data object

void	initDatas();									// 데이터 정보들을 초기화하는 함수, 데이터 별로 이웃 데이터와의 거리를 계산
void	initData(struct data &d);						// 데이터 정보 초기화
void	resetAllDatasUnvisited();
bool	checkObjectIsCore(int minPts, double radius, data &p, vector<int>& candidataSet);
bool	checkObjectIsCore(int minPts, double radius, data &p);
int		makeCluster(double radius);
void	makeClusterIteratively();
double	calcDistance(double x1, double y1, double x2, double y2);
	
struct cluster{
	int c_id;											// cluster ID
	double radius;										// the radius parameter
	int minPts;											// the neighborhood density

	vector<int> clusterSet;								// 클러스터에 포함된 object의 ID
	vector<int> candidateSet;							

	cluster(struct data p, double r, int m, int id) : radius(r), minPts(m), c_id(id){
		candidateSet.push_back(p.id);
	}

	void DBSCAN(){
		while(!candidateSet.empty()){
			// select an object in candidateSet
			int id = candidateSet.back();
			candidateSet.pop_back();

			// 
			map<int, data>::iterator findIter = datas.find(id);
			if(findIter == datas.end())	continue;
			/* if data is unvisited */
			if(findIter->second.visited == false){
				findIter->second.visited = true;
				checkObjectIsCore(minPts, radius, findIter->second, candidateSet);
			}
			/* if data is not yet a memeber of any cluster*/
			if(findIter->second.clustered == false){
				findIter->second.clustered = true;
				findIter->second.clusterId = c_id;
				/* add data to a cluster */
				clusterSet.push_back(findIter->second.id);
			}
		}
	}
};

vector<cluster> clusters;								// A set of cluster
string version;											// version of input file

string parsingArgument(char *fileName){
	string s = fileName;
	
	int pos1 = s.find_first_of("0123456789");
	int pos2 = s.find_first_of(".");
	
	if(pos1 == -1 || pos2 == -1 || pos1 > pos2){
		return "";
	}

	return s.substr(pos1, pos2-pos1);
}

string intToString(int n)
{
	stringstream s;
	s << n;
	return s.str();
}

int main(int argc, char* argv[])
{
	FILE* inputFile;									// input 파일 포인터	
	char  input[1000];									// input 파일을 읽기 위해 사용하는 변수  

	if(argc != 3){
		printf("wrong argument!\n");
		return 0;
	}else{
		inputFile = fopen(argv[1], "r");
		if(inputFile == 0){
			printf("can't open input file!\n");
			return 0;
		}
		version			 = parsingArgument(argv[1]);
		/*
			input에 따른 minPts, epslion값 설정
		*/
		if(version == "1"){
			minPts = 24;
			radius = 15.2;
		}else if(version == "2"){
			minPts = 10;
			radius = 2.2;
		}else if(version == "3"){
			minPts = 10;
			radius = 7.0;
		}
		numberOfClusters = atoi(argv[2]);
	}

	while(true){
		memset(input, 0, sizeof input);		
		fgets(input, 1000, inputFile);
		if(*input == 0)	break;
		
		string inputStr = input;
		struct data newData;
		
		/* id값 */
		int pos = inputStr.find(' ');
		if(pos == -1)	break;
		newData.id	= stoi(inputStr.substr(0, pos));
		inputStr.erase(0, pos+1);
		
		/* x값 */
		pos = inputStr.find(' ');
		if(pos == -1)	break;
		newData.x	= stod(inputStr.substr(0, pos));
		inputStr.erase(0, pos+1);
		
		/* y값 */
		if(inputStr.size() == 0)	break;
		newData.y   = stod(inputStr);
		
		/* 새로운 데이터 추가 */
		datas[newData.id] = newData;
		datasVector.push_back(newData);
	}
	
	initDatas();
	makeClusterIteratively();

	string outputFN = "output" + version + "_cluster_";
	for(int iter=0; iter<clusters.size(); ++iter){
		string outputFileName = outputFN + intToString(iter) + ".txt";
		FILE *outputFile = fopen(outputFileName.c_str(), "w");

		int size = clusters[iter].clusterSet.size();
		for(int i=0; i<size; ++i){
			fprintf(outputFile, "%d\n", clusters[iter].clusterSet[i]);
		}
	}
	return 0;
}

void initDatas(){
	int size = datasVector.size();
	for(int i=0;i < size; ++i){
		initData(datasVector[i]);
	}
}

void initData(struct data &d){	
	int size = datasVector.size();
	map<int, data>::iterator findIter = datas.find(d.id);
	if(findIter == datas.end())	return;
	/*
		for문을 돌면서 d을 제외한 모든 object와의 거리를 계산하고
		그 결과를 d의 neighbors에 저장한다.
	*/
	for(int i=0; i < size; ++i){
		if(d.id != datasVector[i].id){
			double distance = calcDistance(d.x, d.y, datasVector[i].x, datasVector[i].y);
			findIter->second.neighbors.push_back(make_pair(distance, datasVector[i].id));
		}
	}
	sort(findIter->second.neighbors.begin(), findIter->second.neighbors.end());
}

void	resetAllDatasUnvisited()
{
	map<int, data>::iterator iter = datas.begin();
	for(;iter != datas.end(); ++iter){
		iter->second.clustered	= false;
		iter->second.visited	= false;
	}
}

double	calcDistance(double x1, double y1, double x2, double y2)
{
	double ret = ((x2-x1)*(x2-x1)) + ((y2-y1)*(y2-y1));
	return sqrt(ret);
}

/* 
	p의 radius 영역안에 데이터의 개수가 minPts 보다 많은 경우
	radius 영역안에 데이터들을 candidataSet에 추가해준다.
*/
bool	checkObjectIsCore(int minPts, double radius, data &p, vector<int>& candidataSet)
{
	int count = 0;
	vector<int> neighbors;
	/*
		p.neighbors는 거리순으로 정렬되어 있다. 
		p.neighbors[i].first는 i번째 가까운 neighbor object의 거리이고
		p.neighbors[i].second는 i번째 가까운 neighbor object의 id이다.
	*/
	for(int i=0; i<p.neighbors.size(); ++i){
		if(p.neighbors[i].first <= radius){
			count++;
			neighbors.push_back(p.neighbors[i].second);
		}else{
			break;
		}
	}
	/*
		radius 안의 object의 개수가 minPts보다 많은 경우에는
		radius 안의 object를 candidateSet에 넣어주고 true를 반환한다.
	*/
	if(count >= minPts){
		for(int i=0; i<neighbors.size(); ++i){
			candidataSet.push_back(neighbors[i]);
		}
		return true;
	}
	return false;
}

bool	checkObjectIsCore(int minPts, double radius, data &p)
{
	int count = 0;
	/*
		radius안의 object의 개수를 count한다
	*/
	for(int i=0; i<p.neighbors.size(); ++i){
		if(p.neighbors[i].first <= radius){
			count++;
		}else{
			break;
		}
	}
	/*
		count값이 minPts보다 큰 경우에는 true를 반환하고 그렇지 않은
		경우엔 false를 반환한다.
	*/
	if(count >= minPts){
		return true;
	}
	return false;
}

/*
	성공한 경우					: 0
	클러스터의 개수가 많은 경우 : 1
	클러스터의 개수가 적은 경우 : -1
*/
int	makeCluster(double radius){
	int size = datas.size();
	
	/*
		데이터 중 하나를 선택하고 그 데이터를 중심으로 클러스터를 만든다.
	*/
	int c_id	= 0;
	for(int i=0; i<size; ++i){
		if(datas[i].visited == false){
			/*
				object가 core인 경우 해당 object를 중심으로 하는
				cluster를 생성한다.
			*/
			if(checkObjectIsCore(minPts, radius, datas[i])){
				cluster c(datas[i], radius, minPts, c_id++);
				c.DBSCAN();
				clusters.push_back(c);
			}else{
				/*
				object가 core가 아닌 경우(outlier인 경우)
				*/
				datas[i].visited = true;
			}
		}
	}

	if(clusters.size() > numberOfClusters){
		resetAllDatasUnvisited();
		clusters.clear();
		return 1;
	}else if(clusters.size() < numberOfClusters){
		resetAllDatasUnvisited();
		clusters.clear();
		return -1;
	}
	return 0;
}

void	makeClusterIteratively()
{
	double nowRadius	= radius;							//	현재 Radius
	double increaseR	= 0.02;								//	Radius 증가량

	while(true){
		printf("%lf\n", nowRadius);
		int result = makeCluster(nowRadius);
		if(result == 1){
			// 클러스트의 개수가 많은 경우(m > k), radius 값을 증가시킨다.
			nowRadius   += increaseR;		
		}else if(result == -1){
			// 클러스터의 개수가 적은 경우(m < k), radius 값을 감소시킨다.
			nowRadius   = nowRadius/2.0;	
			increaseR	= increaseR/2.0;
		}else{
			// 성공한 경우
			return;
		}
	}
}
