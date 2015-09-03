#pragma once

#include <map>
#include <string>

class moFileReader {
public:
    virtual ~moFileReader() {};

    int readFile(const char* filename);

    const char* lookup(const char* id) const;
    int exportAsHTML(std::string infile, std::string& outfilename);
protected:
    typedef std::map<std::string, std::string> moLookupList;
    moLookupList m_lookup;
};
