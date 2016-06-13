#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;

string DEFAULT_CLASS_LABEL = "DEFAULT";

bool checkDatasInSameClass(vector<struct TrainingData>& datas);						// 모든 데이터가 같은 클래스에 속하는지를 검사
string findMajorityClass(vector<struct TrainingData>& datas);						// 현재 데이터에서 가장 많은 클래스를 반환
string selectionMethod(vector<struct TrainingData>& datas, vector<string>& attrs);	// 가장 좋은 Attribute를 선택하고 선택된 Attribute를 attrs에서 삭제
vector<pair<string, vector<struct TrainingData>>> partitionByAttribute(vector<struct TrainingData>& datas, string attr);
double getInfomation(vector<struct TrainingData>& datas);
double getSplitInfo(vector<pair<string, vector<struct TrainingData>>>& partitionResult, int size);
double getGainRatio(vector<struct TrainingData>& datas, string attribute, double infoD);
void testDataSet(vector<struct TestData>& testDatas);
void testData(struct TestData& data);

struct TrainingData{
	vector<pair<string, string>> columns;	// (속성, 속성 값)
	string classLabel;						// 클래스 라벨
};

struct TestData{
	vector<pair<string, string>> columns;	// (속성, 속성 값)
	string classLabel;						// 클래스 라벨
};

struct DecisionTree{
	string	attr;											// 속성
	vector<pair<string, struct DecisionTree *>> children;	// (속성 값, 자식 트리)

	bool	terminal;										// Leaf 노드인 경우에 True 아니면 False
	string	classLabel;										// Leaf 노드인 경우에 Class 
	string	defaultClassLabel;								// 현재 노드의 default class label

	void makeDecisionTree(vector<struct TrainingData>& datas, vector<string> attrs)
	{
		if(datas.empty()){
			// 데이터가 더이상 없는 경우
			terminal	= true;
			classLabel	= DEFAULT_CLASS_LABEL;
		}else if(checkDatasInSameClass(datas) == true){
			// 모든 datas가 같은 classLabel에 속하는 경우
			terminal	= true;
			classLabel	= datas[0].classLabel; 
		}
		else if(attrs.empty() == true){
			// 더 이상 사용할 수 있는 속성이 없는 경우
			terminal	= true;
			classLabel	= findMajorityClass(datas);
		}else{
			defaultClassLabel = findMajorityClass(datas);
			terminal	= false;
			attr = selectionMethod(datas, attrs);
			vector<pair<string, vector<struct TrainingData>>>& partitionResult = partitionByAttribute(datas, attr);
			for(int i=0; i<partitionResult.size(); ++i){
				struct DecisionTree *childTree = new struct DecisionTree;
				
				string attributeValue = partitionResult[i].first;
				childTree->makeDecisionTree(partitionResult[i].second, attrs);
				children.push_back(make_pair(attributeValue, childTree));
			}
		}
	}
};



vector<string> attrs;						// 데이터 속성들 
vector<string> testAttrs;					// 테스트 데이터 속성들	
string className;							// 데이터의 클래스
vector<struct TrainingData> datas;			// Training Data들
vector<struct TestData>		testDatas;		// Test Data들
struct DecisionTree dTree;					// Decision Tree

char inputStr[200];

char resultFileName[20] = "dt_resultn.txt";
FILE *trainingFile;
FILE *testFile;
FILE *resultFile;

int main(int argc, char* argv[])
{
	if(argc == 3){
		trainingFile= fopen(argv[1], "r");
		testFile	= fopen(argv[2], "r");
		resultFileName[9] = argv[1][8];
		resultFile  = fopen(resultFileName, "w");
	}else{
		return 0;
	}

	string input;
	
	fgets(inputStr, 200, trainingFile);
	input = inputStr;
	input.erase(input.size()-1);

	/* 데이터의 속성과 클래스 파싱 */
	int pos;
	while((pos = input.find(9)) != string::npos){
		attrs.push_back(input.substr(0, pos));
		input.erase(0, pos+1);
	}
	className = input;

	int attrSize = attrs.size();			// 속성의 개수
	/* 데이터 파싱 */
	while(true){
		inputStr[0] = 0;
		fgets(inputStr, 200, trainingFile);
		if(inputStr[0] == 0)	break;
		input = inputStr;
		input.erase(input.size()-1);

		struct TrainingData data;
		for(int i=0; i<attrSize; ++i){
			int pos = input.find(9);

			string attribute = attrs[i];
			string value	 = input.substr(0, pos);
			input.erase(0, pos+1);

			data.columns.push_back(make_pair(attribute, value));
		}
		data.classLabel = input;
		datas.push_back(data);
	}
	DEFAULT_CLASS_LABEL = findMajorityClass(datas);
	
	dTree.makeDecisionTree(datas, attrs);

	/**** 테스트 데이터를 받는다 ****/
	fgets(inputStr, 200, testFile);
	input = inputStr;
	input.erase(input.size()-1);

	/* 데이터의 속성 파싱 */
	while((pos = input.find(9)) != string::npos){
		testAttrs.push_back(input.substr(0, pos));
		fprintf(resultFile, "%s\t", input.substr(0, pos).c_str());
		input.erase(0, pos+1);
	}
	fprintf(resultFile, "%s\t", input.c_str());
	fprintf(resultFile, "%s\n", className.c_str());
	testAttrs.push_back(input);

	attrSize = testAttrs.size();			// 속성의 개수
	/* 데이터 파싱 */
	int iter = 0;
	while(true){
		inputStr[0] = 0;
		fgets(inputStr, 200, testFile);
		if(inputStr[0] == 0)	break;
		input = inputStr;
		input.erase(input.size()-1);

		struct TestData data;
		for(int i=0; i<attrSize; ++i){
			int pos = input.find(9);

			string attribute = testAttrs[i];
			string value	 = input.substr(0, pos);
			input.erase(0, pos+1);

			data.columns.push_back(make_pair(attribute, value));
		}
		testDatas.push_back(data);
	}
	testDataSet(testDatas);
	
	fclose(trainingFile);
	fclose(testFile);
	fclose(resultFile);
	return 0;
}

/* datas의 Class Label이 모두 같은지 확인하는 함수 */
bool checkDatasInSameClass(vector<struct TrainingData>& datas)
{
	int size = datas.size();
	for(int i=1; i < size; ++i){
		if(datas[i].classLabel == datas[i-1].classLabel);
		else	return false;
	}
	return true;
}
/* 현재 데이터에서 가장 많이 차지하고 있는 ClassLabel을 반환하는 함수 */
string findMajorityClass(vector<struct TrainingData>& datas)
{
	vector<pair<int, string>> partition;

	int size = datas.size();
	for(int i=0; i<size; ++i){
		string classLabel = datas[i].classLabel;
		bool updated = false;
		for(int iter = 0; iter < partition.size(); ++iter){
			if(partition[iter].second == classLabel){
				partition[iter].first++;
				updated = true;
				break;
			}
		}
		if(!updated){
			partition.push_back(make_pair(1, classLabel));
		}
	}
	sort(partition.begin(), partition.end(), greater<pair<int, string>>());
	return partition[0].second;
}
/* attr를 기준으로 해서 datas를 partition하는 함수 */
vector<pair<string, vector<struct TrainingData>>> partitionByAttribute
	(vector<struct TrainingData>& datas, string attr)
{
	// (속성 값, 해당 속성 값을 가진 데이터들)
	vector<pair<string, vector<struct TrainingData>>> result;		
	int size = datas.size();

	for(int i = 0; i < size; ++i){
		struct TrainingData data = datas[i];	// 분류할 데이터
		string dataAttributeValue;				// 분류할 데이터의 속성 값
		for(int j=0; j<data.columns.size(); ++j){
			if(data.columns[j].first == attr){
				dataAttributeValue = data.columns[j].second;
				break;
			}
		}
		bool added = false;
		
		for(int j=0; j < result.size(); j++){
			if(result[j].first == dataAttributeValue){
				result[j].second.push_back(data);
				added = true;
				break;
			}
		}
		if(!added){
			vector<struct TrainingData> addData;
			addData.push_back(data);
			result.push_back(make_pair(dataAttributeValue, addData));
		}
	}
	return result;
}
/* 가장 좋은 Attribute를 선택하고 선택된 Attribute를 attrs에서 삭제 */
string selectionMethod(vector<struct TrainingData>& datas, vector<string>& attrs)
{
	int		max		 = 0;
	double	maxGain	 = -100.0;
	double	infoD = getInfomation(datas); 
	for(int i=0; i < attrs.size(); ++i){
		double infoGain = getGainRatio(datas, attrs[i], infoD);
		if(maxGain < infoGain){
			max		= i;
			maxGain = infoGain;
		}
	}
	string spitAttribute = attrs[max];
	attrs.erase(attrs.begin() + max);
	return spitAttribute;
}
double getInfomation(vector<struct TrainingData>& datas)
{
	vector<pair<int, string>> partition;

	int size = datas.size();
	for(int i=0; i<size; ++i){
		string classLabel = datas[i].classLabel;
		int iter;
		for(iter = 0; iter < partition.size(); ++iter){
			if(partition[iter].second == classLabel){
				partition[iter].first++;
				break;
			}
		}
		if(iter == partition.size()){
			partition.push_back(make_pair(1, classLabel));
		}
	}

	double infoD = 0;
	for(int i=0; i<partition.size(); ++i){
		double p = partition[i].first / (double)size;
		infoD	 += (p *  (log(p) / log(2.0)));
	}

	return -infoD;
}

double getSplitInfo(vector<pair<string, vector<struct TrainingData>>>& partitionResult, int size)
{
	double result = 0.0;
	for(int i=0; i<partitionResult.size(); ++i){
		double p = partitionResult[i].second.size()/(double)size;
		result += p * (log(p)/log(2.0));
	}
	return -result;
}

double getGainRatio(vector<struct TrainingData>& datas, string attribute, double infoD)
{
	vector<pair<string, vector<struct TrainingData>>>& partitionResult = partitionByAttribute(datas, attribute);
	double	infoA = 0.0;
	int		size  = datas.size();

	for(int i=0; i < partitionResult.size(); ++i){
		double info = getInfomation(partitionResult[i].second);
		infoA += ((partitionResult[i].second.size())/(double)size) * info;
	}

	double splitInfo = getSplitInfo(partitionResult, size);
	return (infoD - infoA)/splitInfo;
}


void testDataSet(vector<struct TestData>& testDatas)
{
	for(int i=0; i < testDatas.size(); i++){
		testData(testDatas[i]);
		for(int j=0; j<testDatas[i].columns.size(); ++j){
			fprintf(resultFile, "%s\t", testDatas[i].columns[j].second.c_str());
		}
		fprintf(resultFile, "%s\n", testDatas[i].classLabel.c_str());
	}
	return;
}
void testData(struct TestData& data)
{
	struct DecisionTree *p = &dTree;
	while((p->terminal) != true){
		string attr = p->attr;
		string attrValue;
		for(int i=0; i<data.columns.size(); ++i){
			if(attr == data.columns[i].first)
				attrValue = data.columns[i].second;
		}

		bool updated = false;
		for(int i=0; i < p->children.size(); ++i){
			if(attrValue == p->children[i].first){
				p = p->children[i].second;
				updated = true;
				break;
			}
		}
		if(!updated){
			data.classLabel = p->defaultClassLabel;
			return;
		}
	}
	data.classLabel = p->classLabel;
	return;
}
