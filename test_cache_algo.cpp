#include "cache_algo.h"

using namespace std;

int main () {
	int test;
	cout << "first insert: " << endl;
	CacheAlgo<unsigned int> cachealgo;
	cachealgo.insert(0, 500);
	cachealgo.insert(700, 1000);
	cachealgo.insert(550, 600);
	cachealgo.print();
	cout << "test overlap insert" << endl;
	test = cachealgo.insert(250, 300);
	if (test < 0)
		cout << "overlap! insert failed" << endl;
	else
		cout << "failed to cacth the bug" << endl;
	test = cachealgo.insert(1300, 1500);
	if (test < 0)
		cout << "overlap! insert failed" << endl;
	else
		cout << "no overlap. insert suceed" << endl;
	test = cachealgo.remove(550, 600);
	if (test < 0)
		cout << "overlap! insert failed" << endl;
	else
		cout << "no overlap. remove suceeded" << endl;
	cachealgo.print();
	cout << "gonna search 0, 500" << endl;
	if (cachealgo.search(0, 500))
		cout << "find the range" << endl;
	else
		cout << "can't find the range" << endl;
	cout << "print as no change in order" << endl;
	cachealgo.print();
	if (cachealgo.search_update(0, 500) > 0)
		cout << "find the range" << endl;
	else
		cout << "can't find the range" << endl;
	cout << "print as we changed the order" << endl;
	cachealgo.print();
	if (cachealgo.search_update(0, 1200) > 0)
		cout << "find the range" << endl;
	else
		cout << "can't find the range" << endl;
	cout << "print as we changed the order" << endl;
	cachealgo.print();
	cout << "test modify!" << endl;
	if (cachealgo.modify(700, 1000, 700, 1200) > 0)
		cout << "found the range and modified" << endl;
	else
		cout << "can't find the range" << endl;
	cout << "print as we didn't change the order" << endl;
	cachealgo.print();
	if (cachealgo.modify(700, 1000, 700, 1200) > 0)
		cout << "found the range and modified" << endl;
	else
		cout << "can't find the range" << endl;
	cout << "print as we didn't change the order" << endl;
	if (cachealgo.modify_update(700, 1200, 700, 1100) > 0)
		cout << "found the range and modified" << endl;
	else
		cout << "can't find the range" << endl;
	cout << "print as we change the order" << endl;
	cachealgo.print();
	if (cachealgo.modify_update(700, 1200, 700, 1100) > 0)
		cout << "found the range and modified" << endl;
	else
		cout << "can't find the range" << endl;
	cout << "print as we change the order" << endl;
	cachealgo.print();
	
}
