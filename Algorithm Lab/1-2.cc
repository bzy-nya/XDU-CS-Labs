#include"bzy.h"

namespace bzy {
    template<>
    void binary_heap<int>::insert(int val) {
        auto &V = this -> V;
        V.push_back(val);
        for(int x = V.size(); x != 1; x /= 2)
          { if(V[x - 1] < V[x / 2 - 1]) std::swap(V[x - 1], V[x / 2 - 1]); else break; }
    }

    template<>
    void binary_heap<int>::pop() {
        auto &V = this -> V;
        std::swap( V[0], V[V.size() - 1] );
        V.erase( V.end() - 1 );
        for(int x = 1; ; ) {
            if( x * 2 + 1 >= V.size() and V[x * 2] < std::min(V[x * 2 - 1], V[x - 1]) )
              { std::swap( V[x * 2], V[x - 1] ); x = x * 2 + 1; continue; }
            if( x * 2 >= V.size() and V[x * 2 - 1] < std::min(V[x * 2 ], V[x - 1]) )
              { std::swap( V[x * 2 - 1], V[x - 1] ); x = x * 2; continue; }
            break;
        }
    }
    
    template<>
    int binary_heap<int>::front() {
        return this -> V[0];
    }
}
