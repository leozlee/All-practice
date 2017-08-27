//
// Created by leo on 8/2/17.
//

#include <iostream>
#include <algorithm>
#include <vector>

using std::vector;

bool isShorter(const std::string &s1,const std::string &s2)
{
    return s1.size() < s2.size();
}

void elimDups(std::vector<std::string> &words)
{
    //输出原字符序列
    std::cout<<"raw :";
    for(auto &it:words)
    {
        std::cout << it <<" ";
    }
    std::cout<<std::endl;

    //按字典排序words，以便查找查找重复单词
    std::cout<<"sort:";
    sort(words.begin(),words.end());
    for(auto &it:words)
    {
        std::cout << it <<" ";
    }
    std::cout<<std::endl;

    //unique重排输入范围
    std::cout<<"uniq:";
    auto end_unique = unique(words.begin(),words.end());
    for(auto &it:words)
    {
        std::cout << it <<" ";
    }
    std::cout<<std::endl;

    //erase
    std::cout<<"eras:";
    words.erase(end_unique,words.end());
    for(auto &it:words)
    {
        std::cout << it <<" ";
    }
    std::cout<<std::endl;
}


void biggies(std::vector<std::string> &words,std::vector<std::string>::size_type sz)
{

    elimDups(words);
    stable_sort(words.begin(),words.end(),isShorter);

    auto wc = find_if(words.begin(),words.end(),
         [sz](const std::string s1)
         {  return s1.size() >= sz; });
    for(auto &it:words)
    {
        std::cout << it <<" ";
    }
    std::cout<<std::endl;
    auto count = words.end() - wc;
    std::cout << *wc << std::endl;

    for_each(wc,words.end(),[](const std::string &s){std::cout << s << " ";});
    std::cout << std::endl;

}

int main(void)
{
    vector <std::string> word = {"the","quick","red","jumps","over","the","slow","red","turtle"};
    biggies(word,5);
    return 0;

}


