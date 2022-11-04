#include"bzy.h"

namespace bzy {
    int max_interval_sum(std::vector<int> &V) {
        int ans = 0, pre = 0;
        for(auto c : V) { pre += c; if(pre < 0) pre = 0; ans = std::max(ans, pre); }
        return ans;
    }
}
