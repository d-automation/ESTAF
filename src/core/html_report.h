#ifndef HTML_REPORT_H
#define HTML_REPORT_H

#include <QString>

class HtmlReport
{
public:

    static void generate(const QString& logFile,
            const QString& htmlFile, QString curTime);
};

#endif // HTML_REPORT_H
