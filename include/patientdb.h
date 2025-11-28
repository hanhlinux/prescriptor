#ifndef PATIENTDB_H
#define PATIENTDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QDirIterator>

class PatientDb: public QObject
{
    Q_OBJECT
public:
    PatientDb();
    ~PatientDb();
    PatientDb(const QString &path);

    enum ErrType {Success, CreationErr, ConnectionErr, CorruptedBackup, RWError};
    QSettings *prescriptorInfo;
    QString shopName, shopPhoneNum, shopAddress, doctorName;
    QSqlQuery *query;
    QStringList drugsTableHeader, searchTableHeader, completionList;

    int init(const QString &path);
    QString getQuery(const QString &inp = "");
    bool removePatient(const QString &inp);
    int getNextID();
    void savePrescriptorInfo();
    bool backupData(QString dest);
    int restoreData(QString source);

private:
    QString dbDirPath;
    QString dbPath;
    QStringList dbGenCommand;
    QSqlDatabase db;
    bool roErr;

    void prepareVariables(const QString &path);
    void prepareSettings();
};

#endif // PATIENTDB_H
