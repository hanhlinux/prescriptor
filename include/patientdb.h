#ifndef PATIENTDB_H
#define PATIENTDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QSettings>

class PatientDb: public QObject
{
    Q_OBJECT
public:
    PatientDb();
    PatientDb(const QString &path);

    QSettings *prescriptorInfo;
    QString shopName, shopPhoneNum, shopAddress, doctorName;
    QSqlQuery *query;

    void init(const QString &path);
    QString getQuery(const QString &query = "");
    void savePrescriptorInfo();

private:
    QString dbPath;
    QSqlDatabase db;
    bool roErr;
};

#endif // PATIENTDB_H
