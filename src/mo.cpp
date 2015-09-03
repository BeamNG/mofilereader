#include "moFileReader.h"

#include <iostream>
#include <cstdlib>

void printHelp() {
    printf("moFileReader version 2.0\n");
    printf("Usage: <option> <params>\n");
    printf("Possible Options: \n");
    printf("  lookup <mofile> <msgid>      - Outputs the given ID from the file\n");
    printf("  export <mofile> <exportfile> - Exports the whole .mo-file as .po text file\n");
    printf("\n");
    printf("Example: export my18n.mo exportfile.txt\n");
    printf("Example: lookup my18n.mo \"lookupstring\"\n");
}

int readFile(const char* filename, moFileReader &r) {
    FILE* f = NULL;
#ifdef _WIN32
    fopen_s(&f, filename, "rb");
#else 
    f = fopen(filename, "rb");
#endif // _WIN32
    if(!f) return 1;
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc(fsize + 1);
    size_t read_bytes = fread(buf, 1, fsize, f);
    fclose(f);
    if(read_bytes != fsize) {
        free(buf);
        return 2;
    }
    buf[fsize] = 0;
    int res = r.readMemory(buf, fsize);
    free(buf);
    return res;
}


int main(int argc, char** argv) {
    if(argc < 1 || !argv[1]) {
        printHelp();
        return 0;
    }

    std::string arg1 = std::string(argv[1]);
    if(arg1 == "export" && argc > 1 && argv[2] && argv[3]) {
        moFileReader r;
        int res = readFile(argv[2], r);
        if(res) {
            printf("error reading file: %s\n", argv[1]);
            return 1;
        }
        FILE* fo = NULL;
#ifdef _WIN32
        fopen_s(&fo, argv[3], "w");
#else 
        fo = fopen(argv[3], "w");
#endif // _WIN32
        if(!fo) return 2;

        for(moFileReader::translationMap_t::const_iterator it = r.translations.begin(); it != r.translations.end(); it++) {
            fprintf(fo, "msgid  \"%s\"\nmsgstr \"%s\"\n\n", it->first.c_str(), it->second.c_str());
        }
        fclose(fo);
        return 0;

    } else if(std::string(argv[1]) == "lookup" && argv[2] && argv[3]) {
        moFileReader r;
        int res = readFile(argv[2], r);
        if(res) {
            printf("error reading file: %s\n", argv[1]);
            return 1;
        }

        printf("-----------------------------------------\n");
        printf("Lookup: '%s'\n", argv[3]);
        printf("Result: '%s'\n", r.lookup(argv[3]));
        printf("-----------------------------------------\n");
        return 0;
    }

    printHelp();
    return 1;
}
