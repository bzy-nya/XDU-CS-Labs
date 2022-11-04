#include"bzy.h"

namespace bzy {
    double task_scheduling(std::vector<int> V) {
        sort( V.begin(), V.end() );
        int ans = 0, pre = 0;
        for( auto c : V ) pre += c, ans += pre;
        return 1.0 * ans / V.size();
    }
}