#include <iostream>
#include <cstring>
#include <cstdio>
#include <map>
#include "BTree.hpp"
// #include"BTree-yuri.hpp"
#include <vector>

using std::cin;
using std::cout;
using std::string;
using std::endl;
using std::cerr;

sjtu::BTree<int, long long> tree;
std::map<int, int> mp;
std::vector<int> v1;
std::vector<int> v2;
// 如果块大小是4096，200000不能保证正确性，所以建议改小块大小
const int n = 100000;

long long aa = 13131, bb = 5353, MOD = (long long) (1e9 + 7), now = 1;

int rand() {
    for (int i = 1; i < 3; i++)
        now = (now * aa + bb) % MOD;
    return now;
}

void make_vector() {
    for (int i = 1; i <= n + 1; ++i) {
        v1.push_back(rand());
        v2.push_back(rand());
    }
    for (int i = 0; i <= n; ++i) {
        mp[v1[i]] = v2[i];
    }
}

void test_insert() {
    puts("Test: insert");
    for (int i = 1; i <= n; ++i) {
    //    cout << "insert:" << i << endl;
        tree.insert(i, -i);
//        tree.view_root();
    }

    puts("Test insert passed!");
}

void test_insert_random() {
    puts("Test: insert randomly");
    for (int i = n; i >= 1; --i) {
        // if (i==41501)
        // {
        //     puts("");
        // }
        // cout << i << endl;
        if (i==4134)
        {
            puts(" ");
            cout<<tree.at(v1[i])<<endl;
        }
        if (tree.insert(v1[i], v2[i])==0) cout<<i<<endl;
//        tree.view_root();
    }
    puts("Test insert passed!");
}


void test_find() {
    puts("Test: find");
    for (int i = 1; i <= n; ++i) {
        // if (i==41501)
        // {
        //     puts("");
        // }
        // cout << i << endl;
        if (tree.at(i) != -i) {
            cerr << "find error in " << tree.at(i) << endl;
            return;
        }
    }
    puts("Test find passed!");
}

void test_find_random() {
    puts("Test: find random");
    for (int i = 1; i <= n; ++i) {
        if (i==4134)
        {
            puts("");
            cout<<tree.at(v1[i])<<endl;
            cout<<mp[v1[i]]<<endl;
        }
        if (tree.at(v1[i]) != mp[v1[i]]) {
            puts("find error!");
            return;
        }
    }
    puts("Test find passed!");
}

void test_erase() {
    puts("Test erase");
    bool flag;
    for (int i = 1; i <= n; ++i) {
        flag = tree.erase(i);
        if (!flag) {
            cerr << "error in erase " << i << endl;
            return;
        }
    }

    puts("Test erase passed!");
}

void test_erase_random() {
    puts("Test erase random");
    bool flag;
    for (int i = 1; i <= n; ++i) {
        flag = tree.erase(v1[i]);
        if (!flag) {
            cerr << "error in erase " << i << endl;
            return;
        }
    }

    puts("Test erase passed!");
}


void test1() {
    test_insert();
//    tree.view_root();
//    tree.traverse();
    test_find();

    // test_erase();

    puts("simple test over");
}

void test2() {
    make_vector();
    test_insert_random();
//    tree.traverse();
    test_find_random();
    test_erase_random();
//

    puts("Random test over");
}

int keys[n + 1];
long long vals[n + 1];

void make_array() {
    for (int i = 1; i <= n; ++i) {
        keys[i] = i;
        vals[i] = -i;
    }
}



int main() {
    puts("hello");
    tree.clear();
    puts("clear over");

    test1();

   test2();

//    tree.traverse();
}