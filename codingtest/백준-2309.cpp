// 백준 2309번 일곱 난쟁이 문제
#include<iostream>
#include<vector>
#include<algorithm>

auto solution (auto& v)
{
    std::vector<int> comi;
    int sum {};

    do {
        auto it = begin(v);
        for (auto i = 0; i < 7; ++i) {
            sum += *it;
            comi.push_back(*it);
            ++it;
        }
        if (sum == 100) break;
        else {comi.clear(); sum = 0;}
    } while(next_permutation(begin(v), end(v)));

    return comi;
}

int main()
{
    std::vector<int> v;
    std::vector<int> final_v;
    int n {};

    for (int i=0; i<9; i++) {
        std::cin>>n;
        v.push_back(n);
    }

    sort(begin(v), end(v));
    final_v = solution(v);

    for (const auto& i : final_v) {
        std::cout<<i<<std::endl;
    }

}