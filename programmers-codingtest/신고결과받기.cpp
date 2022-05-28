#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>

using namespace std;

vector<int> solution(vector<string> id_list, vector<string> report, int k) {
    vector<int> answer;
    
    map<std::string, int> id_m;
    vector<pair<std::string, int>> id_v;
    
    map<std::string, vector<std::string>> repo_list;
    map<std::string, int> repo_counts;
    
    for (const auto& element : id_list)
    {
        id_m.insert(make_pair(element, 0));
        id_v.push_back(make_pair(element, 0));
    }
    
    // 중복 신고 제거
    sort(report.begin(), report.end());
    report.erase(unique(report.begin(), report.end()), report.end());
    
    for (const auto& element : report)
    {
        auto pos = element.find(" ");
        auto first_word = element.substr(0, pos);
        auto second_word = element.substr(pos+1);
        
        auto it = repo_list.find(first_word);
        if (it == repo_list.end())
        {
            vector<std::string> in_list;
            in_list.push_back(second_word);
            repo_list.insert(make_pair(first_word, in_list));
        }
        else
        {
            it->second.push_back(second_word);
        }
        
        auto item = repo_counts.find(second_word);
        if (item == repo_counts.end())
        {
            repo_counts.insert(make_pair(second_word, 1));
        }
        else 
        {
            item->second++;
        }
    }
    
    for (const auto& el : repo_list)
    {
        auto e_it = id_m.find(el.first);
        if (e_it != id_m.end())
        {
            e_it->second = el.second.size();
        }
        
        for (const auto& el_ : el.second)
        {
            auto it = repo_counts.find(el_);
            if (it != repo_counts.end())
            {
                if (it->second < k)
                    e_it->second--;                    
            }
        }
    }
   
    vector<pair<std::string, int>>::iterator iter;
    for (const auto& el : id_m)
    {
        for (iter=id_v.begin(); iter != id_v.end(); iter++)
        {
            if (iter->first.compare(el.first) == 0)
            {
                iter->second = el.second;
            }
        }
    }
    
    for (const auto& el : id_v)
        answer.push_back(el.second);
    
    return answer;
}