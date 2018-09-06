#ifndef EXTERNALPROGRAMOPENER_H
#define EXTERNALPROGRAMOPENER_H

#include <QString>
#include <QSettings>
#include <my_organprint_views_sidepanel_Export.h>

class MY_ORGANPRINT_VIEWS_SIDEPANEL_EXPORT ExternalProgramOpener
{
public:

    //ExternalProgramOpener(QString programName, QString executableName, QString filePath);
    ExternalProgramOpener(const char * programName, const char * executableName);
    // Should stay untouched
    void run();

    void addArgument(const char *);

    void addArgument(QString &);

protected:
    bool runSystemCommand();

    bool hasProgramPath();

    QString askForProgramPath();

    QString loadProgramPath();


    bool askForUsingSystemDefaultProgram();

    bool useSystemDefaultProgram();

    void saveProgramPath(QString & path);



private:
    static const QString & NO_PATH;
    static const QString & GROUP_ID;
    static const QString & INDICATE_PROGRAM_QUESTION;
    static const QString & LET_SYSTEM_CHOOSE_QUESTION;
    QString programName;
    QString executableName;
    QString filePath;
    QString * executablePath;
    QStringList arguments;
};



#endif // EXTERNALPROGRAMOPENER_H
