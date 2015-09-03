#include "moFileReader.h"

#include <iostream>
#include <cstdlib>

void printHelp() {
    std::cout << "moFileReader version 2.0" << std::endl;
    std::cout << "Usage: " << std::endl;
    std::cout << "   <option> <params>" << std::endl;
    std::cout << "Possible Options: " << std::endl;
    std::cout << "lookup <mofile> <msgid>        - Outputs the given ID from the file." << std::endl;
    std::cout << "export <mofile> [<exportfile>] - Exports the whole .mo-file as HTML." << std::endl;
    std::cout << std::endl;
    std::cout << "Example: export my18n.mo exportfile.html" << std::endl;
    std::cout << "Example: lookup my18n.mo lookupstring" << std::endl;
}

int main(int argc, char** argv) {
    if(argc < 1 || !argv[1]) {
        printHelp();
        return 0;
    }

    std::string arg1 = std::string(argv[1]);
    if(arg1 == "export" && argc > 1 && argv[2]) {

        std::string outfile;
        if(argc > 2 && argv[3]) {
            outfile = argv[3];
        }

        moFileReader r;
        int res = r.readFile(argv[2]);
        if(res) {
            std::cout << "error reading file" << argv[1] << std::endl;
            return 1;
        }
        res = r.exportAsHTML(argv[2], outfile);
        if(res) {
            std::cout << "error exporting file" << argv[1] << std::endl;
            return 1;
        }
        std::cout << "successfully exported to file" << outfile << std::endl;
    
    } else if(std::string(argv[1]) == "lookup" && argv[2] && argv[3]) {
        moFileReader r;
        int res = r.readFile(argv[2]);
        if(res) {
            std::cout << "error reading file" << argv[1] << std::endl;
            return 1;
        }

        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Lookup: '" << argv[3] << "'" << std::endl;
        std::cout << "Result: '" << r.lookup(argv[3]) << "'" << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        return 0;
    }

    printHelp();
    return 1;
}
