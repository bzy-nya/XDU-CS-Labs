#ifndef BZY_H
#define BZY_H

#include<bits/stdc++.h>

namespace bzy {
    bool sum_exsistance( std::vector<int> V, int x );

template<typename T>
    class priority_queue {
    public:
        virtual void insert(T val) = 0;
        virtual void pop() = 0;
        virtual T front() = 0;
        virtual size_t size() = 0;
    };
    
template<typename T>
    class binary_heap : priority_queue<T> {
    public:
        void insert(T val);
        void pop();
        T front();
        size_t size() { return V.size(); }
    private:
        std::vector<T> V;
    };
    
template<typename T>
    void quick_sort(T* begin, T* end);
    
    int kth_element_2(std::vector<int> &V1, std::vector<int> &V2, int k);
    
    int matrix_chain_multiplication(std::vector<int> &V);
    
    int longest_common_subsequence(std::string A, std::string B);
    
    int longest_common_substring(std::string A, std::string B);
    
    int max_interval_sum(std::vector<int> &V);
    
    int shortest_path_dag(std::vector< std::vector< std::pair<int, int> > > &G);
    
    double knapsack_fractional(std::vector< std::pair<int, int> > P, int V);
    
    int knapsack_01(std::vector< std::pair<int, int> > P, int V);

    int knapsack_01_backtracking(std::vector< std::pair<int, int> > P, int V);
    
    double task_scheduling(std::vector<int> V);

    typedef std::vector< std::vector<int> > Graph;

    Graph floyd(Graph G);

    std::vector<int> SPFA(Graph G, int x);

    int eight_queen();
}

#endif
