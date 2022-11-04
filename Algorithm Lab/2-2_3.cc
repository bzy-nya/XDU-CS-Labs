#include"bzy.h"

namespace bzy {
    int longest_common_subsequence(std::string A, std::string B) {
        int N = A.size(), M = B.size();
        std::vector< std::vector<int> > dp = std::vector( N + 1, std::vector(M + 1, 0) );
        for( int i = 1; i <= N; i ++ ) for( int j = 0; j <= M; j ++ ) 
          { if(A[i - 1] == B[j - 1]) dp[i][j] = dp[i - 1][j - 1] + 1;
            dp[i][j] = std::max(dp[i][j], dp[i - 1][j]);
            dp[i][j] = std::max(dp[i][j], dp[i][j - 1]); }
        return dp[N][M];
    }
    
    int longest_common_substring(std::string A, std::string B) {
        int N = A.size(), M = B.size(), ans = 0;
        std::vector< std::vector<int> > dp = std::vector( N + 1, std::vector(M + 1, 0) );
        for( int i = 1; i <= N; i ++ ) for( int j = 0; j <= M; j ++ ) 
          { if(A[i - 1] == B[j - 1]) dp[i][j] = dp[i - 1][j - 1] + 1;
            ans = std::max(ans, dp[i][j]); }
        return ans;
    } 
}
