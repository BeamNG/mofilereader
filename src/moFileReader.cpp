#include "moFileReader.h"
#include <iostream>
#include <cstdlib>

#include <deque>
#include <fstream>
#include <cstring> // this is for memset when compiling with gcc.
#include <sstream>

#ifdef _WIN32
#define MOFILEREADER_PATHSEP "\\"
#else // _WIN32
#define MOFILEREADER_PATHSEP "/"
#endif // _WIN32


/// \brief The Magic Number describes the endianess of bytes on the system.   
static const long MagicNumber = 0x950412DE;
/// \brief If the Magic Number is Reversed, we need to swap the bytes.     
static const long MagicReversed = 0xDE120495;


struct moTranslationPairInformation {
    moTranslationPairInformation() : m_orLength(0), m_orOffset(0), m_trLength(0), m_trOffset(0) {}
    int m_orLength; /// \brief Length of the Original String
    int m_orOffset; /// \brief Offset of the Original String (absolute)
    int m_trLength; /// \brief Length of the Translated String
    int m_trOffset; /// \brief Offset of the Translated String (absolute)
};

struct moFileInfo {
    typedef std::deque<moTranslationPairInformation> moTranslationPairList;
    moFileInfo() : m_magicNumber(0), m_fileVersion(0), m_numStrings(0), m_offsetOriginal(0), m_offsetTranslation(0), m_sizeHashtable(0), m_offsetHashtable(0) {}
    int m_magicNumber; /// \brief The Magic Number, compare it to g_MagicNumber.
    int m_fileVersion; /// \brief The File Version, 0 atm according to the manpage.
    int m_numStrings; /// \brief Number of Strings in the .mo-file.
    int m_offsetOriginal; /// \brief Offset of the Table of the Original Strings
    int m_offsetTranslation; /// \brief Offset of the Table of the Translated Strings
    int m_sizeHashtable; /// \brief Size of 1 Entry in the Hashtable.
    int m_offsetHashtable; /// \brief The Offset of the Hashtable.
    moTranslationPairList m_translationPairInformation; /// \brief A list containing offset and length of the strings in the file.
};

const char* moFileReader::lookup(const char* id) const {
    moLookupList::const_iterator it = m_lookup.find(id);
    if(it == m_lookup.end()) return id;
    return it->second.c_str();
}

int moFileReader::readFile(const char* filename) {
    moFileInfo moInfo;
    moFileInfo::moTranslationPairList& transPairInfo = moInfo.m_translationPairInformation;
    std::ifstream stream(filename, std::ios_base::binary | std::ios_base::in);
    if(!stream.is_open()) {
        return 1;
    }

    // Read in all the 4 bytes of fire-magic, offsets and stuff...
    stream.read((char*)&moInfo.m_magicNumber, 4);
    stream.read((char*)&moInfo.m_fileVersion, 4);
    stream.read((char*)&moInfo.m_numStrings, 4);
    stream.read((char*)&moInfo.m_offsetOriginal, 4);
    stream.read((char*)&moInfo.m_offsetTranslation, 4);
    stream.read((char*)&moInfo.m_sizeHashtable, 4);
    stream.read((char*)&moInfo.m_offsetHashtable, 4);

    if(stream.bad()) {
        stream.close();
        // "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
        return 2;
    }

    if(MagicNumber != moInfo.m_magicNumber) return 3;

    // Now we search all Length & Offsets of the original strings
    for(int i = 0; i < moInfo.m_numStrings; i++) {
        moTranslationPairInformation _str;
        stream.read((char*)&_str.m_orLength, 4);
        stream.read((char*)&_str.m_orOffset, 4);
        if(stream.bad()) {
            stream.close();
            // "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            return 3;
        }
        transPairInfo.push_back(_str);
    }

    // Get all Lengths & Offsets of the translated strings
    // Be aware: The Descriptors already exist in our list, so we just mod. refs from the deque.
    for(int i = 0; i < moInfo.m_numStrings; i++) {
        moTranslationPairInformation& _str = transPairInfo[i];
        stream.read((char*)&_str.m_trLength, 4);
        stream.read((char*)&_str.m_trOffset, 4);
        if(stream.bad()) {
            stream.close();
            // "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            return 3;
        }
    }

    // Normally you would read the hash-table here, but we don't use it. :)

    // Now to the interesting part, we read the strings-pairs now
    for(int i = 0; i < moInfo.m_numStrings; i++) {
        // We need a length of +1 to catch the trailing \0.
        int orLength = transPairInfo[i].m_orLength + 1;
        int trLength = transPairInfo[i].m_trLength + 1;

        int orOffset = transPairInfo[i].m_orOffset;
        int trOffset = transPairInfo[i].m_trOffset;

        // Original
        char* original = new char[orLength];
        memset(original, 0, sizeof(char)*orLength);

        stream.seekg(orOffset);
        stream.read(original, orLength);

        if(stream.bad()) {
            // "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            delete original;
            return 3;
        }

        // Translation
        char* translation = new char[trLength];
        memset(translation, 0, sizeof(char)*trLength);

        stream.seekg(trOffset);
        stream.read(translation, trLength);

        if(stream.bad()) {
            // "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            delete original;
            delete translation;
            return 3;
        }
        m_lookup[std::string(original)] = std::string(translation);
        delete original;
        delete translation;
    }
    stream.close();
    return 0;
}


// the functions below are non-essential and are only used for exportAsHTML()

// Removes spaces from front and end. 
void mofileReader_trim(std::string& in) {
    while(in[0] == ' ') {
        in = in.substr(1, in.length());
    }
    while(in[in.length()] == ' ') {
        in = in.substr(0, in.length() - 1);
    }
}

// Replaces < with ( to satisfy html-rules.
void mofileReader_makeHtmlConform(std::string& inout) {
    std::string temp = inout;
    for(unsigned int i = 0; i < temp.length(); i++) {
        if(temp[i] == '>') {
            inout.replace(i, 1, ")");
        }
        if(temp[i] == '<') {
            inout.replace(i, 1, "(");
        }
    }
}


// Extracts a value-pair from the po-edit-information
bool mofileReader_getPoEditorString(const char* buffer, std::string& name, std::string& value) {
    std::string line(buffer);
    size_t first = line.find_first_of(":");

    if(first == std::string::npos) return false;

    name = line.substr(0, first);
    value = line.substr(first + 1, line.length());

    // Replace <> with () for Html-Conformity.
    mofileReader_makeHtmlConform(value);
    mofileReader_makeHtmlConform(name);

    // Remove spaces from front and end.
    mofileReader_trim(value);
    mofileReader_trim(name);
    return true;
}

int moFileReader::exportAsHTML(std::string infile, std::string& outfilename) {
    const char* css = "body { background-color: black; color: silver; } table { width: 80%;} th { background-color: orange; color: black; } hr { color: red;width: 80%; size: 5px; } a:link{color: gold;} a:visited{color: grey;} a:hover{color:blue;}";
    // Read the file
    moFileReader reader;
    int res = reader.readFile(infile.c_str());
    if(res) return res;

    // Beautify Output
    std::string fname;
    size_t pos = infile.find_last_of(MOFILEREADER_PATHSEP);
    if(pos != std::string::npos) {
        fname = infile.substr(pos + 1, infile.length());
    } else {
        fname = infile;
    }

    // if there is no filename given, we set it to the .mo + html, e.g. test.mo.html
    if(outfilename.empty()) {
        outfilename = infile + std::string(".html");
    }

    std::ofstream stream(outfilename.c_str());
    if(!stream.is_open())  return 1;
    stream << "<!DOCTYPE HTML PUBLIC \"- //W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" << std::endl;
    stream << "<html><head><style type=\"text/css\">\n" << std::endl;
    stream << css << std::endl;
    stream << "</style>" << std::endl;
    stream << "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" << std::endl;
    stream << "<title>Dump of " << fname << "</title></head>" << std::endl;
    stream << "<body>" << std::endl;
    stream << "<center>" << std::endl;
    stream << "<h1>" << fname << "</h1>" << std::endl;
    stream << "<table border=\"1\"><th colspan=\"2\">Project Info</th>" << std::endl;

    std::stringstream parsee;
    parsee << reader.lookup("");

    while(!parsee.eof()) {
        char buffer[1024];
        parsee.getline(buffer, 1024);
        std::string name;
        std::string value;

        mofileReader_getPoEditorString(buffer, name, value);
        if(!(name.empty() || value.empty())) {
            stream << "<tr><td>" << name << "</td><td>" << value << "</td></tr>" << std::endl;
        }
    }
    stream << "</table>" << std::endl;
    stream << "<hr noshade/>" << std::endl;

    // Now output the content
    stream << "<table border=\"1\"><th colspan=\"2\">Content</th>" << std::endl;
    for(moLookupList::const_iterator it = reader.m_lookup.begin();
        it != reader.m_lookup.end(); it++) {
        if(it->first != "") // Skip the empty msgid, its the table we handled above.
        {
            stream << "<tr><td>" << it->first << "</td><td>" << it->second << "</td></tr>" << std::endl;
        }
    }
    stream << "</table><br/>" << std::endl;
    stream << "</center>" << std::endl;
    stream << "</body></html>" << std::endl;
    stream.close();
    return 0;
}
