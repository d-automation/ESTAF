#ifndef EXCEPTIONS_HANDLE_H
#define EXCEPTIONS_HANDLE_H

#include "declarations.h"

using namespace std;

/*!
 * \brief the ErrOpenFile class for handling exceptions when opening file
 */
class ErrOpenFile: public exception
{
public:
    ErrOpenFile(QString message, QString filePath)
    {
        this->message = message;
        this->filePath = filePath;
    }

    QString getFilePath()
    { return filePath; }

    QString getMessage()
    { return message; }

    virtual ~ErrOpenFile() throw() {}

private:
    QString message;
    QString filePath;
};

class ErrInJsonSet: public exception
{
public:
    ErrInJsonSet(QString fname, QString jsonObjectsList, QString param, QString errMsg)
    {
        this->fname = fname;
        this->jsonObjectsList = jsonObjectsList;
        this->param = param;
        this->intro = "error reading json file";
        this->errMsg = errMsg;
        this->errFromJsonFlag = false;
    }
    ErrInJsonSet(QString fname, QString errFromJson)
    {
        this->fname = fname;
        this->errFromJsonFlag = true;
        this->intro = "error reading json file";
        this->errFromJson = errFromJson;
    }
    QString getFname() { return fname; }
    QString getJsonObj() { return jsonObjectsList; }
    QString getParam() { return param; }
    QString getIntro() { return intro; }
    QString getErrMsg() { return errMsg; }
    QString getErrFromJson() { return errFromJson; }
    bool checkErrFromJson() { return errFromJsonFlag; }
    virtual ~ErrInJsonSet() throw() {}

private:
    QString fname;
    QString intro;
    QString jsonObjectsList;
    QString param;
    QString errMsg;
    QString errFromJson;
    bool errFromJsonFlag = false;
};


#endif // EXCEPTIONS_HANDLE_H

