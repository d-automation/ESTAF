#pragma once

#include "exceptions_handle.h"
#include "json_processor.h"

class TestLoader
{
public:

    static QStringList loadCases(const QString& group,
                                 const QString& caseFilter = QString(),
                                 int repeat = 1);

};
