#include"bzy.h"

int dfs(std::vector< std::pair<int, int> > &P, int V, int mask, int f, int val) {
    for(int i = 1; i < f; i ++ ) if( mask & (1 << (i - 1)) ) printf("│ "); else printf("  ");
    if( f == 0 ) {printf("[");}
    else if( !(mask & (1 << (f - 1))) ) printf("└[");
    else printf("├[");
    for(int i = 0; i < P.size(); i ++) if( mask & (1 << i) ) printf("%d", i);
    printf("] -> (weight: %d, value: %d)\n", 100 - V, val);
    if( f == P.size() ) return val;
    int ans = 0;
    if( V >= P[f].second ) ans = std::max( ans, dfs( P, V - P[f].second, mask | (1 << f), f + 1, val + P[f].first) );
    ans = std::max( ans, dfs(P, V, mask, f + 1, val) );
    return ans;
}

int bzy::knapsack_01_backtracking(std::vector< std::pair<int, int> > P, int V) {
    return dfs(P, V, 0, 0, 0);
}
