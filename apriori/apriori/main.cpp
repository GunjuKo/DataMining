#include <stdio.h>
#include <vector>
#include <string.h>
#include <algorithm>
#include <map>
#include <math.h>

using namespace std;

vector<vector<int>> transactions;					// Transaction Informations
vector<map<vector<int>,double>> frequentItemSets;	// Frequent Item Set

double min_support = 10.0;							// minimum support 
int TRANSACTION_SIZE;								// size of transactions

FILE *fRead;	//	"output.txt"
FILE *fWrite;	//	"input.txt"

// 트랜잭션 데이터를 input.txt에서 받고, 데이터를 transactions에 저장합니다.
void receiveTransactionData(FILE *fRead);
void transferStringToVectorInt(char *transaction);
void makeFrequentItemSetZero();

// 두개의 백터가 join 가능하다면 true를 반환하고 join한다.
bool joining(const vector<int>& l1, const vector<int>& l2, vector<int>& result);
// joining 함수에서 사용되는 두개의 벡터를 비교하는 함수
bool compareIntVector(const vector<int> &l1, const vector<int> &l2);
// itemSet의 support 값을 구하는 함수
double scanTransactionByItemSet(const vector<int> &itemSet);
// 이전 frequent itemset에서 다음 candidate item set을 만드는 함수
void candidateItemSet(map<vector<int>, double> &preFrequentItemSet, map<vector<int>, double> &candidateItemSet);
// checkSubset 함수를 이용해서 candidateItemSet에서 subset이 frequent itemset이 아닌 itemSet을 제거
void prone(map<vector<int>, double> &candidateItemSet);
// itemSet이 frequent itemSet이 아닌 경우에 false를 반환
bool checkSubset(vector<int> itemSet);
// candidate item set 중에서 min_support 값보다 작은 경우는 삭제한다.
void removeUnderMinSupport(map<vector<int>, double> &candidateItemSet);
void makeAssociationRule(map<vector<int>, double> &frequentItemSet);
void makeAssociationRuleItemSet(vector<int> &itemSet);
void makeSubsetOfItemSet(vector<int> &itemSet, vector<int> &Choosed, vector<int> &notChoosed, int position, int k);
void CalculateConfidence(vector<int> &Choosed, vector<int> &notChoosed, vector<int> &itemSet);

void receiveTransactionData(FILE *fRead)
{
	const int MAX_TRANSACTION_LENGTH = 200;
	char transaction[MAX_TRANSACTION_LENGTH];
	while(!feof(fRead)){
		memset(transaction, 0, sizeof transaction);
		fgets(transaction, MAX_TRANSACTION_LENGTH, fRead);
		transferStringToVectorInt(transaction);
	}
	TRANSACTION_SIZE = transactions.size();
}
void transferStringToVectorInt(char *transaction){
	vector<int> trans;
	int len = strlen(transaction);
	for(int i=0; i<len;i++){
		if(transaction[i] >= '0' && transaction[i] <= '9'){
			int itemId = 0;
			int j;
			for(j=i; j<len; j++){
				if(transaction[j] >= '0' && transaction[j] <= '9'){
					itemId = (itemId * 10) + (transaction[j] - '0');
				}else{
					i = j;
					trans.push_back(itemId);
					break;
				}
			}
			if(j == len){
				trans.push_back(itemId);
				i = j;
			}
		}
	}
	sort(trans.begin(), trans.end());
	transactions.push_back(trans);
}
bool joining(const vector<int>& l1, const vector<int>& l2, vector<int>& result){
	if(l1.size() == l2.size()){
		int len = l1.size();
		if(len > 1){
			// size of itemsets is bigger than one.
			if(compareIntVector(l1, l2)){
				if(l1.back() < l2.back()){
					result.assign(l1.begin(), l1.end() -1);
					result.push_back(l1.back());
					result.push_back(l2.back());
				}
			}
		}else{
			// size of itemsets is one.
			if(l1.back() < l2.back()){
				result.push_back(l1.back());
				result.push_back(l2.back());
			}
		}
	}
	return !result.empty();
}
bool compareIntVector(const vector<int> &l1, const vector<int> &l2)
{
	bool result = true;
	if(l1.size() == l2.size()){
		int size = l1.size();
		for(int i=0; i<size-1; i++){
			if(l1[i] != l2[i]){
				result = false;
				break;
			}
		}
	}else{
		result = false;
	}
	return result;
}
double scanTransactionByItemSet(const vector<int> &itemSet)
{
	int support_count = 0;
	for(int i=0; i<TRANSACTION_SIZE; i++){
		int transactionIter = 0, itemSetIter = 0;
		for(;transactionIter < transactions[i].size() && itemSetIter < itemSet.size(); ){
			if(transactions[i][transactionIter] == itemSet[itemSetIter]){
				transactionIter++; itemSetIter++;
			}else if(transactions[i][transactionIter] < itemSet[itemSetIter]){
				transactionIter++;
			}else	break;		
		}
		if(itemSetIter == itemSet.size())
			support_count++;
	}
	return (support_count/(double)TRANSACTION_SIZE) * 100;
}
void makeFrequentItemSetZero()
{
	int itemCount[30];
	memset(itemCount, 0 , sizeof itemCount);
	// scanning the database to accumulate the count for each item
	for(int i=0; i<TRANSACTION_SIZE; i++){
		int size = transactions[i].size();
		for(int j=0; j<size; j++){
			itemCount[transactions[i][j]]++;
		}
	}
	// collecting those items that satisfy minimum support. 
	map<vector<int>, double> frequentItemSet;
	for(int i=0; i<30; i++){
		if(itemCount[i] != 0){
			double	itemSupport = (itemCount[i]/(double)TRANSACTION_SIZE) * 100;
			if(itemSupport > min_support){
				vector<int> item;
				item.push_back(i);
				frequentItemSet.insert(make_pair(item, itemSupport));
			}
		}
	}
	frequentItemSets.push_back(frequentItemSet);
}
void candidateItemSet(map<vector<int>, double> &preFrequentItemSet, 
	map<vector<int>, double> &candidateItemSet)
{
	map<vector<int>, double>::iterator i = preFrequentItemSet.begin();
	for(;i != preFrequentItemSet.end(); i++){
		for(map<vector<int>, double>::iterator j=i; j != preFrequentItemSet.end(); j++){
			vector<int> result;
			if(joining(i->first, j->first, result)){
				candidateItemSet.insert(make_pair(result, 0.0));
			}
		}
	}
}
void prone(map<vector<int>, double> &candidateItemSet)
{
	map<vector<int>, double>::iterator iter = candidateItemSet.begin();
	while(iter != candidateItemSet.end()){
		if(!checkSubset(iter->first)){
			// subset이 frequent itemset이 아닌 경우에 삭제
			iter = candidateItemSet.erase(iter);
		}else{
			iter++;
		}
	}
}
bool checkSubset(vector<int> itemSet)
{
	bool result = true;
	int k = itemSet.size() -1;
	for(int i=0; i<itemSet.size(); i++)
	{
		int tmp = itemSet[i];
		itemSet.erase(itemSet.begin() + i);
		map<vector<int>,double>::iterator findIter = frequentItemSets[k-1].find(itemSet);
		if(findIter == frequentItemSets[k-1].end()){
			// if subset of itemset is not in frequent itemsets, return false;
			result = false;
			break;
		}
		itemSet.insert(itemSet.begin() + i, tmp);
	}
	return result;
}
void removeUnderMinSupport(map<vector<int>, double> &candidateItemSet)
{
	map<vector<int>, double>::iterator iter = candidateItemSet.begin();
	while(iter != candidateItemSet.end()){
		double support = scanTransactionByItemSet(iter->first);
		if(support >= min_support){
			iter->second = support;
			iter++;
		}else{
			iter = candidateItemSet.erase(iter);
		}
	}
}
void makeAllfrequentItem()
{
	while(true)
	{
		map<vector<int>, double> newFrequentItemSet;
		candidateItemSet(frequentItemSets.back(), newFrequentItemSet);
		prone(newFrequentItemSet);
		removeUnderMinSupport(newFrequentItemSet);
		if(!newFrequentItemSet.empty()){
			frequentItemSets.push_back(newFrequentItemSet);
			makeAssociationRule(newFrequentItemSet);
		}else{
			break;
		}
	}
}
void makeAssociationRule(map<vector<int>, double> &frequentItemSet)
{
	map<vector<int>, double>::iterator iter= frequentItemSet.begin();
	for(;iter != frequentItemSet.end(); iter++){
		vector<int> itemSet = iter->first;
		makeAssociationRuleItemSet(itemSet);
	}
}

void makeAssociationRuleItemSet(vector<int> &itemSet)
{
	int size = itemSet.size();
	for(int i=1; i<size; i++){
		vector<int> Choosed;
		vector<int> notChoosed;
		makeSubsetOfItemSet(itemSet, Choosed, notChoosed, 0, i);
	}
}

void makeSubsetOfItemSet(vector<int> &itemSet, vector<int> &Choosed, vector<int> &notChoosed, int position, int k)
{
	if(k == 0){
		for(int i=position; i<itemSet.size(); i++){
			notChoosed.push_back(itemSet[i]);
		}
		CalculateConfidence(Choosed, notChoosed, itemSet);
		for(int i=0; i<itemSet.size() - position; i++){
			notChoosed.pop_back();
		}
	}else{
		if(itemSet.size() - position == k){
			// 현재 position에서 k만큼 선택
			for(int i=position; i<itemSet.size(); i++){
				Choosed.push_back(itemSet[i]);
			}
			makeSubsetOfItemSet(itemSet, Choosed, notChoosed, position+k, 0);
			for(int i=0; i<k; i++){
				Choosed.pop_back();
			}
		}else{
			// choosed의 position를 선택
			Choosed.push_back(itemSet[position]);
			makeSubsetOfItemSet(itemSet, Choosed, notChoosed, position+1, k-1);
			Choosed.pop_back();
			// choosed의 position를 선택안함
			notChoosed.push_back(itemSet[position]);
			makeSubsetOfItemSet(itemSet, Choosed, notChoosed, position+1, k);
			notChoosed.pop_back();
		}
	}
}

void CalculateConfidence(vector<int> &Choosed, vector<int> &notChoosed, vector<int> &itemSet)
{
	int k = Choosed.size() - 1;
	double choosedProbability, itemSetProbability;
	map<vector<int>, double>::iterator findIter = frequentItemSets[k].find(Choosed);
	if(findIter != frequentItemSets[k].end()){
		choosedProbability = findIter->second;
	}else{
		return;
	}

	k = itemSet.size() - 1;
	findIter = frequentItemSets[k].find(itemSet);
	if(findIter != frequentItemSets[k].end()){
		itemSetProbability = findIter->second;
	}else{
		return;
	}

	fprintf(fWrite, "{");
	for(int i=0; i<Choosed.size(); i++){
		if(i != Choosed.size() -1)
			fprintf(fWrite, "%d,", Choosed[i]);
		else
			fprintf(fWrite, "%d", Choosed[i]);
	}
	fprintf(fWrite, "}\t");

	
	fprintf(fWrite, "{");
	for(int i=0; i<notChoosed.size(); i++){
		if(i != notChoosed.size() -1)
			fprintf(fWrite, "%d,", notChoosed[i]);
		else
			fprintf(fWrite, "%d", notChoosed[i]);
	}
	fprintf(fWrite, "}\t");

	fprintf(fWrite, "%0.2f\t", itemSetProbability);
	fprintf(fWrite, "%0.2f\n", (itemSetProbability/choosedProbability)*100);
}

int main(int argc, char* argv[])
{

	if(argc == 4){
		// receive argument
		min_support = atof(argv[1]);
		fRead  = fopen(argv[2], "r");
		fWrite = fopen(argv[3], "w");
		if(fRead == NULL || fWrite == NULL){
			printf("Open file Error : Can't open file.");
			return -1;
		}
	}else{
		printf("Wrong Argument : You should pass 3 arguments.");
		return -1;
	} 
	receiveTransactionData(fRead);
	makeFrequentItemSetZero();
	makeAllfrequentItem();
	fclose(fRead);
	fclose(fWrite);
	return 0;
}