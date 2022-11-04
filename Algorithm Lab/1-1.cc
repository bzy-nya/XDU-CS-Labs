#include"bzy.h"

bool bzy::sum_exsistance( std::vector<int> V, int x ) {
    sort( V.begin(), V.end() );
    for( int l = 0, r = V.size() - 1; l < r; l ++) {
        while( r > l and V[l] + V[r] > x ) r --;
        if( r > l and V[l] + V[r] == x ) return true;
    }
    return false;
}