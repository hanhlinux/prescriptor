#ifndef PATIENTCASE_H
#define PATIENTCASE_H

#include <QObject>
#include "patientdb.h"

class PatientCase : public QObject
{
    Q_OBJECT
public:
    enum Gender {Male = false, Female = true};
    enum Info {ID, PrescribeDate, PhoneNumber, Name, BOD, Sex, Address,
                Result, Diagnosis, Note, WarnNoPhone, Drugs, RevisitPeriod,
                RNG, RD, RWG, RP, LNG, LD, LWG, LP};

    PatientCase(QObject *parent = nullptr);

    int id;
    QString name, address, phoneNumber, dateOfBirth, result,
        diagnosis, note, revisitPeriod, prescribeDate;
    QString leftNoGlass, leftDioptre, leftWithGlass, leftPressure;
    QString rightNoGlass, rightDioptre, rightWithGlass, rightPressure;
    bool warnNoPhone, gender;
    QVector<QStringList> drugsList;

    bool saveToDb(const PatientDb& patientDb);
    void clear();

signals:
};

#endif // PATIENTCASE_H
