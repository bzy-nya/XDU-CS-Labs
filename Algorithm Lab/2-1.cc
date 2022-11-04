#include"bzy.h"

namespace bzy {
    int matrix_chain_multiplication(std::vector<int> &V) {
        int N = V.size();
        std::vector< std::vector<int> > dp = std::vector( N, std::vector(N, INT_MAX) );
        for( int i = 0; i < N; i ++ ) dp[i][i] = 0;
        for( int i = 1; i < N; i ++ ) for(int j = i - 1; j >= 0; j -- ) for( int k = j; k < i; k ++ )
          { dp[j][i] = std::min( dp[j][i], dp[j][k] + dp[k + 1][i] + V[i] * V[k] * V[j] ); }
        return dp[0][N - 1];
    }
}
