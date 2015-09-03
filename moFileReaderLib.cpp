#include "moFileReaderLib.h"

#define MOFR_MAX_BUF_LEN 9046

#ifdef _WIN32
#define MOFR_STRNCPY(W,X,Y,Z) strncpy_s(W, X, Y, Z);
#else
#define MOFR_STRNCPY(W,X,Y,Z) strncpy(W, Y, Z);
#endif // _WIN32

typedef struct mo_header_t {
    int magic;
    int file_format;
    int string_count;
    int offset_original;
    int offset_translated;
    int size_hashtable;
    int offset_hashtable;
} mo_header_t;

typedef struct mo_entry_t {
    int length;
    int offset;
} mo_entry_t;

const char* moFileReader::lookup(const char* id) const {
    translationMap_t::const_iterator it = translations.find(id);
    if(it == translations.end()) return id;
    return it->second.c_str();
}

int moFileReader::readMemory(const char* memory, size_t memorySize) {
    mo_header_t* header = (mo_header_t*)memory;
    if(header->magic != 0x950412DE) return 1; // 0xDE120495 for reversed

    mo_entry_t* entry = (mo_entry_t*)(memory + sizeof(mo_header_t));

    char buf[MOFR_MAX_BUF_LEN] = "";
    std::map<int, std::string> tmp;
    
    // read originals
    for(int i = 0; i < header->string_count; i++, entry++) {
        buf[0] = 0;
        MOFR_STRNCPY(buf, sizeof(buf), (memory + entry->offset), entry->length);
        tmp[i] = std::string(buf);
    }

    // read translations
    for(int i = 0; i < header->string_count; i++, entry++) {
        buf[0] = 0;
        MOFR_STRNCPY(buf, sizeof(buf), (memory + entry->offset), entry->length);
        translations[tmp[i]] = std::string(buf);;
    }
    return 0;
}
#undef MOFR_MAX_BUF_LEN
#undef MOFR_STRNCPY
