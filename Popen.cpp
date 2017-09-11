//
// Created by leo on 7/29/17.
//

#include <iostream>
#include <cstdio>
#include <boost/regex.hpp>


bool Popen(const char* cmd,std::string &result)
{
    if(NULL == cmd)
        return false;
    FILE *fp;
    fp = popen(cmd,"r");
    if(NULL == fp) {
        perror("popen error\r\n");
        return false;
    }
    char tmp[1024] = {'\0'};
    if(!fread(tmp,1,sizeof(tmp),fp)){
        perror("read result error");
        fclose(fp);
        return false;
    }
    fclose(fp);
    result = tmp;
    return true;
}

void Split(const std::string &s, const std::string &delim, std::vector<std::string> &ret) {
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos) {
        if (index > last) {
            ret.push_back(s.substr(last, index - last));
        }
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (last < s.length()) {//获取最后一段的内容
        ret.push_back(s.substr(last, index - last));
    }
}



//
//int main(int argc,char **argv)
//{
//    std::string res;
//    if(Popen("ifconfig | awk '/eth[0-9]/ {print $1}' | xargs ifconfig | sed -nr 's/.*addr:(.*)Bcast.*/\\1/p' ",res));
//    {
//        std::cout << res;
//    }
//    //boost::regex pat( ".*addr:(.*)Bcast.*");
//    //boost::regex_match();
//    return 0;
//
//}