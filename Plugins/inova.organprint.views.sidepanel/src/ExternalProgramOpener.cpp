#include "ExternalProgramOpener.h"
#include <iostream>

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QStringList>


const QString & ExternalProgramOpener::NO_PATH = QString("");
const QString & ExternalProgramOpener::GROUP_ID = QString("ExternalSoftware");
const QString & ExternalProgramOpener::INDICATE_PROGRAM_QUESTION = QString("We could not find %1 on your system, could you indicate us where to find the software ?");
const QString & ExternalProgramOpener::LET_SYSTEM_CHOOSE_QUESTION = QString("Would you like to open the file with the default system program ?");

ExternalProgramOpener::ExternalProgramOpener(const char * programName
        , const char * executableName)
    : programName(programName)
    , executableName(executableName)
    , executablePath(nullptr)
{
    // it tries to load the program path.
    // if it exists, it sets it as the executable path
    QString loadedExecutablePath = loadProgramPath();
    if(loadedExecutablePath == NO_PATH) {
        executablePath = &this->executableName;
    }
    else {
        executablePath = new QString(loadedExecutablePath);
    }
}


/**
 * @brief ExternalProgramOpener::run
 * This method defines the whole logic of the process. Platform specific
 * methods should be implemented
 * inside the other methods
 */
void ExternalProgramOpener::run() {

    if(arguments.size() == 0) {
        return;
    }

    if(runSystemCommand()== false) {
        QString path = askForProgramPath();
        if(path == NO_PATH) {
            bool systemOpening = askForUsingSystemDefaultProgram();
            if(systemOpening) useSystemDefaultProgram();
        }
        else {
            executablePath = &path;
            saveProgramPath(path);

            run();
        }
    }
}


/**
 * @brief ExternalProgramOpener::runSystemCommand
 * @return True if the command succeeded
 *
 * This use the name of the executable to run. The executable has be
 * in the PATH variable in order for it to work. If not, this class
 * should ask for the user to specify the path of the executable
 *
 */
bool ExternalProgramOpener::runSystemCommand() {

    QProcess * process = new QProcess(nullptr);

    const QString program(executablePath->toStdString().c_str());



    process->start(program,arguments);
    process->waitForStarted();

    if(process->error() == QProcess::ProcessError::FailedToStart) {
        return false;
    }
    else {
        return true;
    }

    /*
    QString cmd = executablePath + QString(" ") + filePath;
    int r = std::system(cmd.toStdString().c_str());
    return r == 0;
    */
}


/**
 * @brief ExternalProgramOpener::useSystemDefaultProgram
 * @return true if the program started correctly
 *
 * This function call the process which is responsible of letting
 * the system choose which program should be used for this file.
 */
bool ExternalProgramOpener::useSystemDefaultProgram() {

    // !!!
    // Code that should be platform specific

    QString xdgOpen("xdg-open");
    saveProgramPath(xdgOpen);
    QString cmd("xdg-open " + filePath);
    int r = std::system(cmd.toStdString().c_str());
    return r == 0;



}


/**
 * @brief ExternalProgramOpener::hasProgramPath
 * @return True if path for a program was already saved
 */
bool ExternalProgramOpener::hasProgramPath() {
    return loadProgramPath() != NO_PATH;
}

/**
 * @brief ExternalProgramOpener::askForProgramPath
 * @return NO_PATH if the users cancels or the path indicated by the user
 *
 * This function will pop up a dialog that will ask the user if he wants
 * to indicate where the program is. If the user says yes, a file dialog
 * should appear
 */
QString ExternalProgramOpener::askForProgramPath() {

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, programName,INDICATE_PROGRAM_QUESTION.arg(programName),
                                  QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes) {
        QString result;
        result = QFileDialog::getOpenFileName(nullptr,
                                              QString("Find %1 (%2)")
                                              .arg(programName)
                                              .arg(executableName), "", executableName
                                             );
        if(result == "") {
            return NO_PATH;
        }
        return result;
    }
    else {
        return NO_PATH;
    }
}

/**
 * @brief ExternalProgramOpener::askForUsingSystemDefaultProgram
 * @return Ask the user if he wants to use the default system program
 * to open the file
 */
bool ExternalProgramOpener::askForUsingSystemDefaultProgram() {

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, programName,LET_SYSTEM_CHOOSE_QUESTION,
                                  QMessageBox::Yes|QMessageBox::No);

    return reply == QMessageBox::Yes;
}

/**
 * @brief ExternalProgramOpener::saveProgramPath
 * @param path
 *
 * Saves the path of executable inside the INI file
 *
 */
void ExternalProgramOpener::saveProgramPath(QString & path) {


    QSettings settings("OrganPrint.ini",QSettings::IniFormat);
    settings.beginGroup(GROUP_ID);
    settings.setValue(executableName,path);
    settings.endGroup();
}


/**
 * @brief ExternalProgramOpener::loadProgramPath
 * @return
 * Loads and return the path of the executable
 */
QString ExternalProgramOpener::loadProgramPath() {
    QSettings settings("OrganPrint.ini",QSettings::IniFormat);
    settings.beginGroup(GROUP_ID);
    return settings
    .value(executableName,QVariant(NO_PATH))
    .toString();

}

void ExternalProgramOpener::addArgument(const char * str) {
    arguments << QString(str);
}

void ExternalProgramOpener::addArgument(QString & str) {
    arguments << str;
}
