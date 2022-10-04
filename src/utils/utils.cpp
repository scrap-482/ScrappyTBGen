#include "utils.h"

#include <string>
#include <regex>

std::stringstream dlogStream;

std::vector<std::string> split(const std::string str, const std::string regex_str) {
    std::regex regexz(regex_str);
    std::sregex_token_iterator token_iter(str.begin(), str.end(), regexz, -1);
    std::sregex_token_iterator end;
    std::vector<std::string> list;
    while (token_iter != end) {
        list.emplace_back(*token_iter++);
    }
    return list;
}
