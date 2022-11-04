#include"bzy.h"

namespace bzy {
    double knapsack_fractional(std::vector< std::pair<int, int> > P, int V) {
        sort( P.begin(), P.end(), [](std::pair<int, int> x, std::pair<int, int> y) {
            return x.first * y.second > x.second * y.first;
        } );
        double ans = 0;
        for( auto [v, m] : P ) {
            int q = std::min( V, m );
            ans += 1.0 * q * v / m;
            V -= q;
            if( !V ) break;
        }
        return ans;
    }
    
    int knapsack_01(std::vector< std::pair<int, int> > P, int V) {
        std::vector<int> dp(V + 1);
        for( auto [v, m] : P ) for( int i = V; i >= m; i --) 
          { dp[i] = std::max( dp[i], dp[i - m] + v ); }
        return dp[V];
    }
}