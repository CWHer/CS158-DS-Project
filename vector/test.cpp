#include<cstdio>
#include<cstdlib>
#include<iostream>
#include"vector.hpp"
using namespace sjtu;
//#include<vector>
//using namespace std;
//sjtu::vector<double> w;
void TestPush_Pop()
{
	std::cout << "Testing push_back and pop_back..." << std::endl;
	sjtu::vector<double> vd;
	for (double i = 0.0; i < 10.0; i += 1.0) {
		vd.push_back(i);
	}
//	std::cout << vd.back() << std::endl;
//	for (double i = 20.0; i < 23.0; i += 1.0) {
//		vd.push_back(i);
//	}
	std::cout << vd.back() << std::endl;
	vd.pop_back();
	std::cout << vd.back() << std::endl;
	vd.pop_back();
	std::cout << vd.back() << std::endl;
	for (int i = 0; i < 5; ++i) {
		vd.pop_back();
	}
	std::cout << vd.back() << std::endl;
}
int main()
{
	TestPush_Pop();
//    for(int i=0;i<100;i++)
//    {
//        w.push_back(i);
////        printf("%d\n",w[i]);
//    }
//    sjtu::vector<double> vd;
//    for (double i = 0.0; i < 10.0; i += 1.0) {
//		vd.push_back(i);
//	}
//    w.clear();
//    for(int i=0;i<10;i++)
//    {
//        w.push_back(i);
//        printf("%d\n",w[i]);
//    }
    return 0;
}
