#include "test_loader.h"

QStringList TestLoader::loadCases(const QString& group,
                                  const QString& caseFilter,
                                  int repeat)
{
    JsonProcessor json;
    QJsonObject jsonObj;
    QString jsonPath = "Path";
    path_t path;

    json.openJsonFile("../config.json", jsonObj);
    QJsonObject objPath = jsonObj.value(jsonPath).toObject();
    json.jsonGetStrValue(objPath, "test_plan_path", path.test_plan_path, jsonPath);
    json.jsonGetStrValue(objPath, "test_cases_path", path.test_cases_path, jsonPath);

    QFile f(path.test_plan_path);
    QStringList result;

    if (!f.open(QIODevice::ReadOnly))
    {
        throw(ErrOpenFile("cannot open json file", path.test_plan_path));
    }

    QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();

    const QJsonArray arr = root["groups"].toObject()[group].toArray();

    if (arr.count() == 0)
    {
        throw(ErrOpenFile("invalid format in json file!", path.test_plan_path));
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
        { result << QString("%1%2").arg(path.test_cases_path, name); }

    }

    return result;
}
