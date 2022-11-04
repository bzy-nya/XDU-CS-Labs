#include<bzy.h>

namespace bzy {
    int kth_element_2(std::vector<int> &V1, std::vector<int> &V2, int k) {
        int l1 = std::max(0, k - (int)V2.size()), r1 = std::min(k - 1, (int)V1.size());
        int l2 = std::max(0, k - (int)V1.size()), r2 = std::min(k - 1, (int)V2.size());
        
        while( l1 ^ r1 ) {
            int mid = (l1 + r1) >> 1;
            if( V1[mid] >= V2[k - mid - 2] and (k - mid - 1 == V2.size() or V1[mid] <= V2[k - mid - 1]) ) return V1[mid];
            if( V1[mid] < V2[k - mid - 2] ) l1 = mid + 1;
            else r1 = mid;
        }
        if( V1[l1] >= V2[k - l1 - 2] and (k - l1 - 1 == V2.size() or V1[l1] <= V2[k - l1 - 1])  ) return V1[l1];
        
        while( l2 ^ r2 ) {
            int mid = (l2 + r2) >> 1;
            if( V2[mid] >= V1[k - mid - 2] and (k - mid - 1 == V2.size() or V1[mid] <= V2[k - mid - 1])  ) return V2[mid];
            if( V2[mid] < V1[k - mid - 2] ) l2 = mid + 1;
            else r2 = mid;
        }
        if( V2[l2] >= V1[k - l2 - 2] and (k - l2 - 1 == V2.size() or V1[l2] <= V2[k - l2 - 1])  ) return V2[l2];
        return -1;
    }
}
