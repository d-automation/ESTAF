#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include "test_options.h"

class CLIParser
{
public:
    static TestOptions parse(int argc, char* argv[]);
};

#endif // CLI_PARSER_H
