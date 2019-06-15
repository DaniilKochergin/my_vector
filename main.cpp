#include <iostream>
#include "vector.h"

using namespace std;

int main() {
    vector<int> v;
    v.reserve(6);
    v.shrink_to_fit();
    v.push_back(1);
    v.reserve(6);
    v.push_back(3);
    v.push_back(4);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);v.push_back(5);v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);
    v.push_back(5);

    v.resize(10);

    vector<int> a(v);
    std::cout << a[0] << std::endl;
    std::cout << v[1] << std::endl;
    cout << endl;
    v.pop_back();
    cout << v[1] << endl;
    cout << v[0] << endl;
    cout << a[2] << endl;
    v.clear();
    cout << v.size() << " " << a.size() << endl;
    v= a;
    cout << endl;

    v.clear();
    return 0;
}
