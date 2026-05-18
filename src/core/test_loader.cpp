#include "test_loader.h"

QStringList TestLoader::loadCases(const QString& group,
                                  const QString& caseFilter,
                                  int repeat)
{
    QString fname = "C:/Danila/work/embedded_test_stand/configs/test_plan.json";
    QFile f(fname);
    QStringList result;

    if (!f.open(QIODevice::ReadOnly))
    {
        throw(ErrOpenFile("cannot open json file", fname));
    }

    QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();

    const QJsonArray arr = root["groups"].toObject()[group].toArray();

    if (arr.count() == 0)
    {
        throw(ErrOpenFile("invalid format in json file!", fname));
    }


    for (auto v : arr)
    {
        QString name = v.toString();

        // skip test
        if (name.startsWith("#"))
            continue;

        // filter
        if (!caseFilter.isEmpty())
        {
            if (!name.contains(caseFilter))
                continue;
        }

        for (int i = 0; i < repeat; ++i)
        { result << QString("C:/Danila/work/embedded_test_stand/configs/test_cases/%1").arg(name); }

    }

    return result;
}
