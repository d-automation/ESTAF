#include "html_report.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

static QMap<QString,int> extractSummary(
        const QString& content)
{
    QMap<QString,int> s;

    QStringList lines =
        content.split("\n");

    for (QString line : lines)
    {
        if (line.startsWith("TOTAL:"))
            s["total"] =
                line.split(":")[1].trimmed().toInt();

        if (line.startsWith("PASSED:"))
            s["passed"] =
                line.split(":")[1].trimmed().toInt();

        if (line.startsWith("FAILED:"))
            s["failed"] =
                line.split(":")[1].trimmed().toInt();
    }

    return s;
}

void HtmlReport::generate(
        const QString& logFile,
        const QString& htmlFile,
        QString curTime)
{
    QFile f(logFile);

    f.open(QIODevice::ReadOnly);

    QString content =
        f.readAll();

    f.close();

    auto summary =
        extractSummary(content);

    QString html;

    html += R"(
    <html>
    <head>
    <style>
    body {
        font-family: monospace;
        background:#111;
        color:#eee;
    }

    .pass {
        color:lightgreen;
    }

    .fail {
        color:red;
    }

    details {
        margin-bottom:10px;
    }

    summary {
        cursor:pointer;
        font-weight:bold;
    }

    pre {
        white-space:pre-wrap;
    }
    </style>
    </head>
    <body>
    )";

    html += QString("<h1>Tests Report (%1)</h1>").arg(curTime);

    html += QString(R"(
    <h2>SUMMARY</h2>

    <div>
    <b>Total:</b> %1<br>
    <b style='color:lightgreen;'>Passed:</b> %2<br>
    <b style='color:red;'>Failed:</b> %3<br>
    </div>
    )")
    .arg(summary["total"])
    .arg(summary["passed"])
    .arg(summary["failed"]);

    QStringList tests =
        content.split("<<<TEST_START>>>");

    for (QString t : tests)
    {
        if (!t.contains("TEST:"))
            continue;

        QString cls =
            t.contains("STATUS: FAIL")
            ? "fail"
            : "pass";

        QString title =
            t.split("\n")[1];

        html += QString(R"(
        <details>
        <summary class="%1">%2</summary>
        <pre>%3</pre>
        </details>
        )")
        .arg(cls)
        .arg(title)
        .arg(t.toHtmlEscaped());
    }

    int passRate = 0;

    if (summary["total"] > 0)
    {
        passRate =
            (100 * summary["passed"])
            / summary["total"];
    }

    html += QString(R"(
    <div style="background:#333;width:300px;">
        <div style="
            background:green;
            width:%1%;
            color:white;">
            %1%
        </div>
    </div>
    )").arg(passRate);

    html += "</body></html>";

    QFile out(htmlFile);

    out.open(QIODevice::WriteOnly);

    out.write(html.toUtf8());

    out.close();
}
