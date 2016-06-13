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
	int		id;											// id ��

	double	x;											// x ��
	double	y;											// y ��

	bool	visited;									// �湮 ����
	bool	clustered;									// Ŭ�����͸� �� ��쿡 true, �׷��� ������ false
	
	int		clusterId;

	vector<pair<double, int> > neighbors;				// �ֺ� object���� �Ÿ��� ����(�Ÿ�, object�� id)

	data() : visited(false), clustered(false){}
};

int numberOfClusters;									// The number of clusters
int minPts		= MINPTS;								// the neighborhood density threshold
double radius	= DEFAULT_RADIOUS;
map<int, data> datas;									// set of input data objects
vector<data>   datasVector;								// This value is used for initialize the data object

void	initDatas();									// ������ �������� �ʱ�ȭ�ϴ� �Լ�, ������ ���� �̿� �����Ϳ��� �Ÿ��� ���
void	initData(struct data &d);						// ������ ���� �ʱ�ȭ
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

	vector<int> clusterSet;								// Ŭ�����Ϳ� ���Ե� object�� ID
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
	FILE* inputFile;									// input ���� ������	
	char  input[1000];									// input ������ �б� ���� ����ϴ� ����  

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
			input�� ���� minPts, epslion�� ����
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
		
		/* id�� */
		int pos = inputStr.find(' ');
		if(pos == -1)	break;
		newData.id	= stoi(inputStr.substr(0, pos));
		inputStr.erase(0, pos+1);
		
		/* x�� */
		pos = inputStr.find(' ');
		if(pos == -1)	break;
		newData.x	= stod(inputStr.substr(0, pos));
		inputStr.erase(0, pos+1);
		
		/* y�� */
		if(inputStr.size() == 0)	break;
		newData.y   = stod(inputStr);
		
		/* ���ο� ������ �߰� */
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
		for���� ���鼭 d�� ������ ��� object���� �Ÿ��� ����ϰ�
		�� ����� d�� neighbors�� �����Ѵ�.
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
	p�� radius �����ȿ� �������� ������ minPts ���� ���� ���
	radius �����ȿ� �����͵��� candidataSet�� �߰����ش�.
*/
bool	checkObjectIsCore(int minPts, double radius, data &p, vector<int>& candidataSet)
{
	int count = 0;
	vector<int> neighbors;
	/*
		p.neighbors�� �Ÿ������� ���ĵǾ� �ִ�. 
		p.neighbors[i].first�� i��° ����� neighbor object�� �Ÿ��̰�
		p.neighbors[i].second�� i��° ����� neighbor object�� id�̴�.
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
		radius ���� object�� ������ minPts���� ���� ��쿡��
		radius ���� object�� candidateSet�� �־��ְ� true�� ��ȯ�Ѵ�.
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
		radius���� object�� ������ count�Ѵ�
	*/
	for(int i=0; i<p.neighbors.size(); ++i){
		if(p.neighbors[i].first <= radius){
			count++;
		}else{
			break;
		}
	}
	/*
		count���� minPts���� ū ��쿡�� true�� ��ȯ�ϰ� �׷��� ����
		��쿣 false�� ��ȯ�Ѵ�.
	*/
	if(count >= minPts){
		return true;
	}
	return false;
}

/*
	������ ���					: 0
	Ŭ�������� ������ ���� ��� : 1
	Ŭ�������� ������ ���� ��� : -1
*/
int	makeCluster(double radius){
	int size = datas.size();
	
	/*
		������ �� �ϳ��� �����ϰ� �� �����͸� �߽����� Ŭ�����͸� �����.
	*/
	int c_id	= 0;
	for(int i=0; i<size; ++i){
		if(datas[i].visited == false){
			/*
				object�� core�� ��� �ش� object�� �߽����� �ϴ�
				cluster�� �����Ѵ�.
			*/
			if(checkObjectIsCore(minPts, radius, datas[i])){
				cluster c(datas[i], radius, minPts, c_id++);
				c.DBSCAN();
				clusters.push_back(c);
			}else{
				/*
				object�� core�� �ƴ� ���(outlier�� ���)
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
	double nowRadius	= radius;							//	���� Radius
	double increaseR	= 0.02;								//	Radius ������

	while(true){
		printf("%lf\n", nowRadius);
		int result = makeCluster(nowRadius);
		if(result == 1){
			// Ŭ����Ʈ�� ������ ���� ���(m > k), radius ���� ������Ų��.
			nowRadius   += increaseR;		
		}else if(result == -1){
			// Ŭ�������� ������ ���� ���(m < k), radius ���� ���ҽ�Ų��.
			nowRadius   = nowRadius/2.0;	
			increaseR	= increaseR/2.0;
		}else{
			// ������ ���
			return;
		}
	}
}
