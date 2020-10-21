#include<cstdio>
#include<ctime>
#include<iostream>
#include<cstdlib>
#include <cassert>
#include<algorithm>
#include"map.hpp"
#include<map>
using namespace sjtu;
using namespace std;
// map<int,int> M;
long long now=1;
long long x;

class Integer {
public:
    static int counter;
    int val;

    Integer(int val) : val(val) {
        counter++;
    }

    Integer(const Integer &rhs) {
        val = rhs.val;
        counter++;
    }

    Integer& operator = (const Integer &rhs) {
        assert(false);
    }

    ~Integer() {
        counter--;
    }
};

int Integer::counter = 0;

class Compare {
public:
    bool operator () (const Integer &lhs, const Integer &rhs) const {
        return lhs.val < rhs.val;
    }
};
bool prime(int x)
{
	for(int i=2;i*i<=x;++i)
		if (x%i==0) return 0;
	return 1;
}

int Rand()
{
//    static long long now=1;
    now=(now*x+100019)%(int)(1e9+7);
    return now;
}
int cnt[20];
void solve()
{
	sjtu::map<Integer, std::string, Compare> map;
    for (int i = 0; i < 1000000; ++i) {
        std::string string = "";
        for (int number = i; number; number /= 10) {
            char digit = '0' + number % 10;
            string = digit + string;
        }
        if (i & 1) {
            map[Integer(i)] = string;
            auto result = map.insert(sjtu::pair<Integer, std::string>(Integer(i), string));
            assert(!result.second);
        } else {
            auto result = map.insert(sjtu::pair<Integer, std::string>(Integer(i), string));
            assert(result.second);
        }
    }
    std::cout << map.ss() << "\n";
    
}
int check12(){ // erase(it++)
    sjtu::map<int, int> Q;
    std::map<int, int> stdQ;
    int num[30001];
    num[0] = 0;
    for(int i = 1; i <= 30000; i++) num[i] = num[i - 1] + rand() % 325 + 1;
    for(int i = 1; i <= 60000; i++) swap(num[rand() % 30000 + 1], num[rand() % 30000 + 1]);
    for(int i = 1; i <= 30000; i++){
        int t = rand();
        stdQ[num[i]] = t; Q[num[i]] = t;
    }

    sjtu::map<int, int>::iterator it;
    std::map<int, int>::iterator stdit;
    for(int i = 1; i <= 60000; i++) swap(num[rand() % 30000 + 1], num[rand() % 30000 + 1]);
    for(int i = 1; i <= 10325; i++){
        // cout<<num[i];
        it = Q.find(num[i]); 
        Q.erase(it++);
        stdit = stdQ.find(num[i]); stdQ.erase(stdit++);
        if(it == Q.end()){
            if(stdit != stdQ.end()) return 1;
        }
        else{
            if(it -> first != stdit -> first) return 2;
        }
    }
    if(Q.size() != stdQ.size()) return 3;
    it = Q.begin();
    for(stdit = stdQ.begin(); stdit != stdQ.end(); stdit++){
        if(stdit -> first != it -> first) return 4;
        if(stdit -> second != (*it).second) return 5;
        it++;
    }
    stdit = --stdQ.end();
    for(it = --Q.end(); it != Q.begin(); it--){
        if(stdit -> first != it -> first) return 6;
        if(stdit -> second != (*it).second) return 7;
        stdit--;
    }
    return 8;
}
int main()
{
//	freopen("out","w",stdout);
//	for(int i=12001;i<20000;++i)
    // int x=check12();
    // cout<<x;
	// while (x==8)
    // {
    //     x=check12();
    //     cout<<x<<endl;
    // }
    sjtu::map<int,int> map;
    for(int i=0;i<5;++i) map[i]=i;
    // std::cout<<map.ss()<<endl;
    for(int i=0;i<5;++i) 
    {
        cout<<i<<endl;
        map.erase(map.find(i));
        for(int j=i+1;j<5;j++) cout<<map[j]<< ' ';
    }
    // std::cout<<map.ss()<<endl;
    // for(int i=50;i<100;++i) 
    // {
    //     if (i==56)
    //     {
    //         putchar('\n');
    //     }
    //     cout<<i<<endl;
    //     map.erase(map.find(i));
    //     std::cout<<map.ss()<<endl;
    // }
    // while (map.begin()!=map.end())
    //     map.erase(map.begin());
    // std::cout<<map.ss()<<std::endl;
    // std::cout<<map.ss()<<endl;
    // for(int i=2;i<=10;++i)
        // std::cout<<map[i]<<std::endl;
	return 0;
}
