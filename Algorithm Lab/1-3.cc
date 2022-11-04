#include"bzy.h"

namespace bzy {
    template<>
        void quick_sort<int>(int* begin, int* end) {
            if( begin + 1 >= end ) return ;
            auto mid = begin[rand() % (end - begin)];
            auto itl = begin, itr = end - 1;
            for( ; itl <= itr; ) {
                if(*itl > mid) { std::swap(*itl, *itr); itr --; }
                else itl ++; 
            }
            bool flag = 0;
            for( auto it = begin + 1; it != end; it ++ )
              { if( *it < *(it - 1)) {flag = 1; break;} }
            if(!flag) return;
            quick_sort(begin, itl);
            quick_sort(itr, end);
        }
}
