#include<bzy.h>

namespace bzy {
    int shortest_path_dag(std::vector< std::vector< std::pair<int, int> > > &G) {
        int N = G.size();
        std::vector<int> dp(N, INT_MAX), deg(N);
        std::queue<int> Q; Q.push(0); dp[0] = 0;
        for(int i = 0; i < N; i ++ ) for( auto [N, v] : G[i] ) deg[N] ++;
        while( Q.size() ) {
            int x = Q.front(); Q.pop();
            for( auto [N, v] : G[x] ) {
                dp[N] = std::min(dp[x] + v, dp[N]);
                if( ! -- deg[N] ) Q.push(N);
            }
        }
        return dp[N - 1];
    }
}
