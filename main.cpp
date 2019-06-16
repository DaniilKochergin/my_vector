#include <iostream>
#include "vector.h"



int main() {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    auto it = v.begin();
    for(;it!=v.end();++it){
        std::cout<<*it<<"\n";
    }
    return 0;
}
