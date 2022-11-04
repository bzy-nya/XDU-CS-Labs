#include"bzy.h"

typedef std::vector< std::vector<int> > Graph;

namespace bzy {
    Graph floyd(Graph G) {
        int N = G.size();
        for( int i = 0; i < N; i ++ ) for( int j = 0; j < N; j ++ ) for( int k = 0; k < N; k ++ )
          { if( G[i][k] != INT_MAX and G[k][j] != INT_MAX ) G[i][j] = std::min( G[i][j], G[i][k] + G[k][j] ); }
        return G; 
    }

    std::vector<int> SPFA(Graph G, int x) {
        int N = G.size();
        std::queue<int> Q; Q.push(x);
        std::vector<int> dis(N, INT_MAX); dis[x] = 0;
        while( Q.size() ) {
            int x = Q.front(); Q.pop();
            for(int i = 0; i < N; i ++) if(G[x][i] != INT_MAX) {
                if( dis[x] + G[x][i] < dis[i] ) dis[i] = dis[x] + G[x][i], Q.push(i);
            }
        }
        return dis;
    }
}