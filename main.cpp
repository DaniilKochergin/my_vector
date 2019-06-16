#include <iostream>
#include "vector.h"

using namespace std;



struct Node {
    Node() = delete;

    Node(int *b) {
        cout<<1<<endl;
        a = new int(*b);
    }

    ~Node(){
        delete a;
    }

    int *a;
};

vector<Node> a;

int main() {
    int c = 3;
    int *g = &c;
    a.push_back(g);
    int * d =a[0].a;
    cout<< *d <<endl;
    delete (g);
    return 0;
}
