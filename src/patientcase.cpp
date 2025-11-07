#include "patientcase.h"

PatientCase::PatientCase(QObject *parent)
    : QObject{parent}
{
    warnNoPhone = false;
    gender = Male;
    id = 0;
}

void PatientCase::clear() {
    id = 0;
    gender = Male;
    prescribeDate.clear();
    phoneNumber.clear();
    name.clear();
    dateOfBirth.clear();
    address.clear();
    result.clear();
    diagnosis.clear();
    note.clear();
    warnNoPhone = false;
    drugsList.clear();
    revisitPeriod.clear();
    leftNoGlass.clear();
    leftDioptre.clear();
    leftWithGlass.clear();
    leftPressure.clear();
    rightNoGlass.clear();
    rightDioptre.clear();
    rightWithGlass.clear();
    rightPressure.clear();
}

bool PatientCase::saveToDb(const PatientDb &patientDb) {
    QString listOfInfo = "id, prescribeDate, phonenum, name, dateofbirth, gender, address, ";
    listOfInfo += "result, diagnosis, note, warnNoPhone, drugs, revisitPeriod, ";
    listOfInfo += "rightNoGlass, rightDioptre, rightWithGlass, rightPressure, ";
    listOfInfo += "leftNoGlass, leftDioptre, leftWithGlass, leftPressure";

    QString drugs;
    for (qsizetype i = 0; i < drugsList.count(); i++) {
        drugs += drugsList[i].join("::") + "\n";
    }

    patientDb.query->prepare(
        "INSERT INTO patients("+ listOfInfo +") "
        "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    patientDb.query->addBindValue(id);
    patientDb.query->addBindValue(prescribeDate);
    patientDb.query->addBindValue(phoneNumber);
    patientDb.query->addBindValue(name);
    patientDb.query->addBindValue(dateOfBirth);
    patientDb.query->addBindValue(gender);
    patientDb.query->addBindValue(address);
    patientDb.query->addBindValue(result);
    patientDb.query->addBindValue(diagnosis);
    patientDb.query->addBindValue(note);
    patientDb.query->addBindValue(warnNoPhone);
    patientDb.query->addBindValue(drugs);
    patientDb.query->addBindValue(revisitPeriod);
    patientDb.query->addBindValue(rightNoGlass);
    patientDb.query->addBindValue(rightDioptre);
    patientDb.query->addBindValue(rightWithGlass);
    patientDb.query->addBindValue(rightPressure);
    patientDb.query->addBindValue(leftNoGlass);
    patientDb.query->addBindValue(leftDioptre);
    patientDb.query->addBindValue(leftWithGlass);
    patientDb.query->addBindValue(leftPressure);

    return patientDb.query->exec();
}
