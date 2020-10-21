#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "BTree.hpp"
// #include"BTree-yuri.hpp"
#include <vector>
#include <map>

#define MAXN 500000
int random_seed = 19260817;

std::map<int, long long> _map;
sjtu::BTree <int, long long> btree;
std::vector<int> keys;

int rand(){
  random_seed = abs(55762 * random_seed * random_seed + 10744 * random_seed + 233 * random_seed * random_seed * random_seed + 103421);
  return random_seed;
}

void init_data(){
  for(register int i=0; i<MAXN; ++i){
    int i1=rand();
    long long i2=rand();
    keys.push_back(i1);
    _map.insert(std::pair<int, long long>(i1, i2));
    btree.insert(i1, i2);
  }
}

void test_query() {
  printf("Test Insert.\n");
  for (int i = 0; i < MAXN; ++i) {
    if (btree.at(i) != _map[i]) {
      std::cerr << "WA at insert/query." << std::endl;
      system("pause");
    }
  }
  printf("Test Insert Pass!\n");
}

void test_erase() {
  printf("Test Erase.\n");
  for (int i = 0; i < MAXN; ++i) {
    int i1=rand()%MAXN;
    int key=keys[i1];
    if(!_map.count(key)){
      if(btree.erase(key)){
        std::cerr << "WA at erase/query." << std::endl;
        system("pause");
      }
    }else{
      if(_map[key]!=btree.at(key)){
        std::cerr << "WA at query." << std::endl;
        system("pause");
      }
      _map.erase(key);
      btree.erase(key);
    }
  }
  printf("Test Erase Pass!\n");
}

int main(){
  btree.clear();
  init_data();
  test_query();
  // test_erase();
  return 0;
}