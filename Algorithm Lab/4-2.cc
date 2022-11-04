#include"bzy.h"

int dfs(std::vector< std::vector<int> > V, int f) {
    if( f == 8 ) return 1;
    int ans = 0;
    for( int i = 0; i < 8; i ++ ) if( !V[f][i] ) {
        for( int u = 0; u < 8; u ++ ) for( int v = 0; v < 8; v ++ ) 
          { if( u == f or v == i or u + v == f + i or u - v == f - i ) V[u][v] ++; }
        ans += dfs( V, f + 1 );
        for( int u = 0; u < 8; u ++ ) for( int v = 0; v < 8; v ++ ) 
          { if( u == f or v == i or u + v == f + i or u - v == f - i ) V[u][v] --; }
    }
    return ans;
}

int bzy::eight_queen() {
    std::vector< std::vector<int> > V(8, std::vector<int>(8, 0) );
    return dfs( V, 0 );
}