#pragma once
#include <vector>
#include <string>
class Command
{
public:
    std::string executable;
    std::vector<std::string> arguments = std::vector<std::string>();
    int compare(std::string s);
};
class CommandExtractor {
public:
    Command extract(std::string s);
};

