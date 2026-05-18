#ifndef TEST_OPTIONS_H
#define TEST_OPTIONS_H

#include <QString>

struct TestOptions
{
    QString group = "full";

    QString caseFilter;

    int repeat = 1;

    int delayMs = 0;
};

#endif // TEST_OPTIONS_H
