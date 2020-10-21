#include<cstdio>
#include"BTree.hpp"
#include<ctime>
#include<cstdlib>
using namespace std;
// #include"BTree-yuri.hpp"
sjtu::BTree<int,long long> t;
int main()
{
    // int cnt=0;
    // srand(time(0));
    int cnt=0;
    t.clear();
    // int n=500000;
    int n=5000;
    for(int i=0;i<=n;++i)
        t.insert(i,i);
    for(int i=0;i<=n;i+=2)
    // for(int i=n;i>=0;i-=rand()%20+1)
    {
        // printf("%d\n",i);
        t.erase(i);
    }
        
        // t.erase(rand());
    // t.clear();
    // t.insert(1,1);
    // printf("%d\n",t.begin().getValue());
    // // t.insert(2,2);
    // printf("%d\n",t.begin().getValue());
    // // t.insert(-1,999);
    // printf("%d\n",t.begin().getValue());
    // // t.insert(-2,2);
    // printf("%d\n",t.begin().getValue());
    // printf("\n");
    // printf("%d\n",t.at(1));
    // printf("%d\n",t.at(-2));
    // printf("%d\n",t.at(2));
    // printf("%d\n",t.at(-1));
    // printf("%d\n",t.begin().getKey());
    // printf("%d\n",(--t.end()).getKey());
    sjtu::BTree<int,long long>::iterator it;
    it=t.begin();
    while (it!=t.end())
    {
        printf("%d\n",it.getKey());
        ++it,cnt++;
    }
    printf("%d\n",cnt);
    // it=t.end();
    // do
    // {
    //     --it;
    //     printf("%d\n",it.getKey());
        
        
    // } while (it!=t.begin());
    return 0;
}