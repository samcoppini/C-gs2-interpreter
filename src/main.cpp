#include "block.hpp"
#include "gs2context.hpp"
#include "gs2exception.hpp"

#include <CLI/CLI.hpp>

#include <fstream>
#include <iostream>

#ifdef WIN32
    #include <io.h>
    #define isatty _isatty
#else
    #include <unistd.h>
#endif

gs2::List initialStack() {
    gs2::List input;

    if (!isatty(STDIN_FILENO)) {
        char c;
        while (std::cin.get(c)) {
            input.add(c);
        }
    }

    gs2::List stack;
    stack.add(input);
    return stack;
}

int main(int argc, char **argv) {
    std::string filename;
    bool printVersion;

    CLI::App app{"An interpreter for the gs2 programming language."};
    app.add_option("file", filename, "The gs2 file to interpret.");
    app.add_flag("-v,--version", printVersion, "Print the gs2 version and exit.");
    CLI11_PARSE(app, argc, argv);

    if (printVersion) {
        std::cout << "gs2-cpp " << GS2_VERSION << "\n";
        return 0;
    }

    if (filename.empty()) {
        std::cerr << "No input file provided!\n";
        return 1;
    }

    std::ifstream codeFile{filename, std::ios_base::in | std::ios_base::binary};
    if (!codeFile.is_open()) {
        std::cerr << "Unable to open '" << argv[1] << "'\n";
        return 2;
    }

    std::vector<uint8_t> code;
    char c;

    while (codeFile.get(c)) {
        code.push_back(c);
    }

    try {
        auto block = gs2::Block::parseBytes(code);
        auto stack = initialStack();

        gs2::GS2Context gs2{stack};
        block.execute(gs2);

        for (const auto &val: stack) {
            std::cout << val.str();
        }
    }
    catch (const gs2::GS2Exception &ex) {
        // If an exception is thrown, we output the error to stdout, but
        // it prints out a quine to stdout. This behavior of gs2 allows it
        // to be good at simple "print this string" problems, since most
        // random strings are unlikely to be valid gs2 programs.
        std::cerr << ex.what() << '\n';
        std::cout.write(reinterpret_cast<char *>(code.data()), code.size());
    }
}
