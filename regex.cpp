#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "Popen.cpp"
#include <vector>



#if 0
int main()
{
    std::string line;
    boost::regex pat( "^Subject: (Re: |Aw: )*(.*)" );

    while (std::cin)
    {
        std::getline(std::cin, line);
        boost::smatch matches;
        if (boost::regex_match(line, matches, pat))
            std::cout << matches[2] << std::endl;
    }
}
#endif


//测试匹配数字
//int main()
//{
//    // Match the whole string
//    // regex_match(str, regex)
//    boost::regex pat1("(\\d{4}-){3}\\d{4}");
//    std::string card_str("1234-5678-4321-8765");
//    std::cout << boost::regex_match(card_str, pat1) << std::endl;
//    std::cout << boost::regex_match(card_str, boost::regex("\\d{12}")) << std::endl;
//    std::cout << "----" << std::endl;
//    return 0;
//}


//测试匹配通用匹配和regex_match的使用
//int main(int argc ,char **argv)
//{
//    std::string str = "This exdivssion could match from A and beyond.";
//    boost::regex reg("(A.*)");
//    bool b = boost::regex_match(str,reg);
//    if(b)
//        std::cout << "match" << std::endl;
//    else
//        std::cout << "dismatch" << std::endl;
//    return 0;
//}


//测试\d和\w的使用
//规则："3个数字, 一个单词, 任意字符, 2个数字或字符串"N/A," 一个空格, 然后重复第一个单词 ."
//int main(int argc ,char **argv)
//{
//
//    std::cout<<"this is a test for regex"<<std::endl;
//    std::string correct   ="123Hello N/A Hello";
//    std::string incorrect ="123Hello 12 hello";
//
//    //boost::regex reg("\\d{3}([a-zA-Z]+).(\\d{2}|N/A)\\s\\1");
//    boost::regex reg("\\d{3}(\\w+).(\\d{2}|N/A)\\s\\1");
//
//    if(boost::regex_match(correct,reg))
//        std::cout<<"correct match"<<std::endl;
//    if(boost::regex_match(incorrect,reg))
//        std::cout<<"incorrect dismatch"<<std::endl;
//
//    return 0;
//}


int main(int argc,char **argv)
{
    std::string result;
    if(!Popen("ifconfig eth0",result))
        return -1;
    //std::cout<<result<<std::endl;
    std::string::const_iterator start = result.begin();
    std::string::const_iterator end = result.end();

    std::vector<boost::smatch> addr;
    boost::regex reg("\\d{3}\\.\\d{3}\\.\\d+\\.\\d+");
    boost::smatch m;
    while(boost::regex_search(start,end,m,reg)) {
        std::cout<< "result is "<< m[0]<<std::endl;
        start = m[0].second;
    }

    return 0;

}





//测试regex_search()的使用
//int main(int argc ,char **argv)
//{
//    boost::regex reg("(new)|(delete)");
//    boost::smatch m;//保存结果
//    std::string s= "Calls to new must be followed by delete.Calling simply new results in a leak!";
//    boost::rearch(s,m,reg);
//    if(m[0].matched)
//        std::cout<<"new is matched"<<std::endl;
//}


//测试regex_replace()的使用

