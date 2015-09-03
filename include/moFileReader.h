#pragma once

#include <map>
#include <string>

class moFileReader {
public:
    virtual ~moFileReader() {};
    int readMemory(const char* memory, size_t memorySize);
    const char* lookup(const char* id) const;

    typedef std::map<std::string, std::string> translationMap_t;
    translationMap_t translations;
};
