#include"bzy.h"

namespace task1_1 {
    const int N = 100000, M = 5;

    int main() { 
        srand( time(NULL) );
        std::vector<int>V(N), VP(M), V1(M), V2(M);
        for( int i = 1; i <= N; i ++ )
          { V.push_back(rand()); }
        for( int i = 0; i < M; i ++) VP[i] = rand(); 
        int x = rand();
        auto t1 = clock();
        for(auto c : VP) {
            int ans = 0;
            std::unordered_map<int, bool> MP;
            for(auto d : V) if( MP[c - d] ) {ans = 1; break; } else MP[d] = 1;
            V2.push_back(ans);
        }
        auto t2 = clock();
        for(auto c : VP) V2.push_back( bzy::sum_exsistance(V, c) );
        auto t3 = clock();
        for(int i = 0; i < M; i ++) assert( V1[i] == V2[i] );
        printf( "Accept!\n" );
        printf( "std::unorded_map cost %lf ms\n", 1.0 * (t2 - t1) / CLOCKS_PER_SEC * 1000 );
        printf( "tow points algorithm cost %lf ms\n", 1.0 * (t3 - t2) / CLOCKS_PER_SEC * 1000 );
        return 0;
    };
}

namespace task1_2 {
    const int N = 100000;

    int main() {
        std::priority_queue<int> Q1;
        bzy::binary_heap<int> Q2;
        srand( time(NULL) );
        std::vector<int>V(N), V1(N), V2(N);
        for( int i = 1; i <= N; i ++ )
          { V.push_back(rand()); }
        auto t1 = clock();
        for( auto c : V ) Q1.push(c);
        while( Q1.size() ) { V1.push_back( Q1.top() ); Q1.pop(); }
        auto t2 = clock();
        for( auto c : V ) Q2.insert(c);
        while( Q2.size() ) { V1.push_back( Q2.front() ); Q2.pop(); }
        auto t3 = clock();
        for(int i = 0; i < N; i ++) assert( V1[i] == V2[i] );
        printf( "Accept!\n" );
        printf( "std::priority_queue cost %lf ms\n", 1.0 * (t2 - t1) / CLOCKS_PER_SEC * 1000 );
        printf( "bzy::binary_heap cost %lf ms\n", 1.0 * (t3 - t2) / CLOCKS_PER_SEC * 1000 );
        return 0;
    }
}

namespace task1_3 {
    const int N = 100000;

    int main() {
        srand( time(NULL) );
        int c1[N + 5], c2[N + 5];
        for( int i = 1; i <= N; i ++ ) c1[i] = c2[i] = rand() % 100;
        auto t1 = clock();
        std::sort(c1 + 1, c1 + 1 + N);
        auto t2 = clock();
        bzy::quick_sort(c2 + 1, c2 + 1 + N);
        auto t3 = clock();
        for(int i = 1; i <= N; i ++) assert( c1[i] == c2[i] );
        printf( "Accept!\n" );
        printf( "std::sort cost %lf ms\n", 1.0 * (t2 - t1) / CLOCKS_PER_SEC * 1000 );
        printf( "bzy::QuickSort cost %lf ms\n", 1.0 * (t3 - t2) / CLOCKS_PER_SEC * 1000 );
        return 0;
    }
}

namespace task1_4 {
    const int N = 100000;

    int main() {
        srand(time(NULL));
        std::vector<int> V1(N), V2(N);
        for( int i = 0; i < N; i ++ ) V1.push_back(rand());
        for( int i = 0; i < N; i ++ ) V1.push_back(rand());
        int k = rand() % (N * 2) + 1;
        sort( V1.begin(), V1.end() );
        sort( V2.begin(), V2.end() );
        
        auto t1 = clock();
        int ans1 = 0;
        for( int p1 = 0, p2 = 0; k; k -- ) {
            if( p1 == N ) { ans1 = V2[p2 + k - 1]; break; }
            if( p2 == N ) { ans1 = V1[p1 + k - 1]; break; }
            int pre = 0;
            if( V1[p1] < V2[p2] ) pre = V1[p1 ++]; else V2[p2 ++];
            if( k == 1 ) { ans1 = pre; break;}
        }
        auto t2 = clock();
        int ans2 = bzy::kth_element_2(V1, V2, k);
        auto t3 = clock();
        assert( ans1 == ans2 );
        printf( "Accept!\n" );
        printf( "brute force cost %lf ms\n", 1.0 * (t2 - t1) / CLOCKS_PER_SEC * 1000 );
        printf( "bzy::kth_element2 cost %lf ms\n", 1.0 * (t3 - t2) / CLOCKS_PER_SEC * 1000 );
        return 0;
    }
}

namespace task2_1 {
    std::vector<int> A = {3, 5, 2, 1, 10};
    std::vector<int> B = {2, 7, 3, 6, 10};
    std::vector<int> C = {10, 3, 15, 12, 7, 2};
    std::vector<int> D = {7, 2, 4, 15, 20, 5};

    int main() {
        printf( "answer of A is %d\n", bzy::matrix_chain_multiplication(A) );
        printf( "answer of B is %d\n", bzy::matrix_chain_multiplication(B) );
        printf( "answer of C is %d\n", bzy::matrix_chain_multiplication(C) );
        printf( "answer of D is %d\n", bzy::matrix_chain_multiplication(D) );
        return 0;
    }
}

namespace task2_23 {
    std::string A1 = "xzyzzyx";
    std::string A2 = "zxyyzxz";
    std::string B1 = "MAEEEVAKLEKHLMLLRQEYVKLQKKLAETEKRCALLAAQANKESSSESFISRLLAIVAD";
    std::string B2 = "MAEEEVAKLEKHLMLLRQEYVKLQKKLAETEKRCTLLAAQANKENSNESFISRLLAIVAG";

    int main() {
        printf( "A: lcs = %d(sequence), %d(string)\n", bzy::longest_common_subsequence(A1, A2), bzy::longest_common_substring(A1, A2) );
        printf( "B: lcs = %d(sequence), %d(string)\n", bzy::longest_common_subsequence(B1, B2), bzy::longest_common_substring(B1, B2) );
        return 0;
    }
}

namespace task2_4 {
    std::vector<int> V = {-2, 11, -4, 13, -5, -2};

    int main() {
        printf( "max interval sum is %d\n", bzy::max_interval_sum(V) );
        return 0;
    }
}

namespace task2_5 {
    #define mp std::make_pair
    std::vector< std::vector< std::pair<int, int> > > G = {
        { mp(1, 5), mp(2, 3) },
        { mp(3, 1), mp(4, 3), mp(5, 6) },
        { mp(5, 7), mp(6, 6) },
        { mp(7, 6), mp(8, 8) },
        { mp(7, 3), mp(8, 5) },
        { mp(8, 3), mp(9, 3) },
        { mp(8, 8), mp(9, 4) },
        { mp(10, 2), mp(11, 2) },
        { mp(11, 1), mp(12, 2) },
        { mp(11, 3), mp(12, 3) },
        { mp(13, 3), mp(14, 5) },
        { mp(13, 5), mp(14, 2) },
        { mp(13, 6), mp(14, 5) },
        { mp(15, 4) },
        { mp(15, 3) },
        {} 
    };
    #undef mp

    int main() {
        printf( "shortest path on the dag is %d\n", bzy::shortest_path_dag(G) );
        return 0;
    }
}

namespace task3_1 {
    #define mp std::make_pair
    std::vector< std::pair<int, int> > V = {
        mp(20, 10), mp(30, 20), mp(65, 30), mp(40, 40), mp(60, 50)
    };
    #undef mp

    int main() {
        printf( "the answer of fractional knapsack is %lf\n", bzy::knapsack_fractional( V, 100 ) );
        printf( "the answer of 0/1 knapsack is %d\n", bzy::knapsack_01( V, 100 ) );
        return 0;
    }
}

namespace task3_2 {
    std::vector<int> V = {15, 8, 3, 10};
    int main() {
        printf( "average completion time of best scheduling is %lf\n", bzy::task_scheduling(V) );
        return 0;
    }
}

namespace task3_34 {
    typedef std::vector< std::vector<int> > Graph;

    Graph G = {
        {INT_MAX, -1, 3, INT_MAX, INT_MAX},
        {INT_MAX, 3, 2, 2, INT_MAX},
        {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
        {INT_MAX, 1, 5, INT_MAX, INT_MAX},
        {INT_MAX, INT_MAX, -3, INT_MAX, INT_MAX, INT_MAX}
    };

    void print_vector(std::vector<int>V) {
        printf("[");
        for( auto c : V ) printf("%d ", c );
        printf("]\n");
    }

    int main() {
        printf( "Single source shortest path from A:\n");
        print_vector( bzy::SPFA(G, 0) );
        printf( "All pair shortest path:\n");
        for(auto c : bzy::floyd(G)) print_vector(c);
        return 0;
    }

}

namespace task4_1 {
    #define mp std::make_pair
    std::vector< std::pair<int, int> > V = {
        mp(20, 10), mp(30, 20), mp(65, 30), mp(40, 40), mp(60, 50)
    };
    #undef mp

    int main() {
        printf( "the answer of 0/1 knapsack is %d\n", bzy::knapsack_01_backtracking( V, 100 ) );
        return 0; 
    }
}

namespace task4_2 {
    int main() {
        printf( "answer of 8-queens problem is %d\n", bzy::eight_queen() );
        return 0;
    };
}

int main() {
    printf( "-------------------------task1-1-----------------------\n" ); task1_1::main(); printf("\n");
    printf( "-------------------------task1-2-----------------------\n" ); task1_2::main(); printf("\n");
    printf( "-------------------------task1-3-----------------------\n" ); task1_3::main(); printf("\n");
    printf( "-------------------------task1-4-----------------------\n" ); task1_4::main(); printf("\n");
    printf( "-------------------------task2-1-----------------------\n" ); task2_1::main(); printf("\n");
    printf( "--------------------task2-2 & task2-3------------------\n" ); task2_23::main(); printf("\n");
    printf( "-------------------------task2-4-----------------------\n" ); task2_4::main(); printf("\n");
    printf( "-------------------------task2-5-----------------------\n" ); task2_5::main(); printf("\n");
    printf( "-------------------------task3-1-----------------------\n" ); task3_1::main(); printf("\n");
    printf( "-------------------------task3-2-----------------------\n" ); task3_2::main(); printf("\n");
    printf( "--------------------task3-3 & task3-4------------------\n" ); task3_34::main(); printf("\n");
    printf( "-------------------------task4-1-----------------------\n" ); task4_1::main(); printf("\n");
    printf( "-------------------------task4-2-----------------------\n" ); task4_2::main(); printf("\n");
    return 0;
}
