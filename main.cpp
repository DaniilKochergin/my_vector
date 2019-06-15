#include <iostream>
#include "vector.h"
int main()
{
    vector<int> v;
    v.push_back(3);
    v.push_back(4);
    v.push_back(5);
    vector<int> a(v);
    std::cout<<v[2]<<std::endl;
    std::cout<<v[2]<<std::endl;
    return 0;
}
