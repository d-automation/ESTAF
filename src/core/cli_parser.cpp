#include "cli_parser.h"

#include <QStringList>

TestOptions CLIParser::parse(int argc, char* argv[])
{
    TestOptions opt;

    QStringList args;

    for (int i = 0; i < argc; ++i)
    {
        args << argv[i];
    }

    for (int i = 0; i < args.size(); ++i)
    {
        QString a = args[i];

        if (a == "--group" && i + 1 < args.size())
        {
            opt.group = args[i + 1];
        }

        else if (a == "--case" && i + 1 < args.size())
        {
            opt.caseFilter = args[i + 1];
        }

        else if (a == "--repeat" && i + 1 < args.size())
        {
            opt.repeat = args[i + 1].toInt();
        }

        else if (a == "--delay" && i + 1 < args.size())
        {
            opt.delayMs = args[i + 1].toInt();
        }
    }

    return opt;
}
