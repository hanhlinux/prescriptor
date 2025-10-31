#include "patientdb.h"

PatientDb::PatientDb() {
    init("");
}

PatientDb::PatientDb(const QString &path) {
    init(path);
}

QString PatientDb::getQuery(const QString &query) {
    return "SELECT * FROM patients WHERE id > 0 " + query + " ORDER BY id DESC";
}

void PatientDb::init(const QString &path) {
    if (path == "") return;

    if (!QFile(path + "/settings.ini").exists()) {
        prescriptorInfo = new QSettings(path + "/settings.ini", QSettings::IniFormat);
        prescriptorInfo->setValue("info/shopname", "");
        prescriptorInfo->setValue("info/shopphonenum", "");
        prescriptorInfo->setValue("info/shopaddress", "");
        prescriptorInfo->setValue("info/doctorname", "");
        prescriptorInfo->sync();
    }
    else {
        prescriptorInfo = new QSettings(path + "/settings.ini", QSettings::IniFormat);
        shopName = prescriptorInfo->value("info/shopname").toString();
        shopAddress = prescriptorInfo->value("info/shopaddress").toString();
        shopPhoneNum = prescriptorInfo->value("info/shopphonenum").toString();
        doctorName = prescriptorInfo->value("info/doctorname").toString();
    }


    db = QSqlDatabase::addDatabase("QSQLITE");
    dbPath = path + "/prescriptions.db";
    db.setDatabaseName(dbPath);

    bool dbExist = QFile(dbPath).exists();
    bool isDBOpen = db.open();
    qDebug() << "Opening/creating database file";
    query = new QSqlQuery(db);
    if (isDBOpen == true && dbExist == false) {
        bool exec = query->exec("create table patients("
                   "id int NOT NULL primary key, "
                   "prescribeDate varchar(20), "
                   "phonenum varchar(10) , "
                   "name varchar(50), "
                   "dateofbirth varchar(10), "
                   "gender bool, "
                   "address varchar(100), "
                   "result text, "
                   "diagnosis text, "
                   "note text, "
                   "warnNoPhone bool, "
                   "drugs text, "
                   "revisitPeriod varchar(30), "
                   "rightNoGlass varchar(10),"
                   "rightDioptre varchar(10),"
                   "rightWithGlass varchar(10),"
                   "rightPressure varchar(10),"
                   "leftNoGlass varchar(10), "
                   "leftDioptre varchar(10),"
                   "leftWithGlass varchar(10),"
                   "leftPressure varchar(10))");
        if (exec == false) {
            QMessageBox::critical(nullptr, "Error", "Cannot create database file: " + query->lastError().text());
            QFile(dbPath).remove();
            exit(1);
        }
        qDebug() << query->lastError().text();


        query->exec("INSERT INTO patients(id)"
                    "VALUES(0)");
    }
    else if (isDBOpen == false) {
        QMessageBox::critical(nullptr , "Error", "Cannot connect to database");
        exit(1);
    }
    query->first();
}

void PatientDb::savePrescriptorInfo() {
    prescriptorInfo->setValue("info/shopname", shopName);
    prescriptorInfo->setValue("info/shopphonenum", shopPhoneNum);
    prescriptorInfo->setValue("info/shopaddress", shopAddress);
    prescriptorInfo->setValue("info/doctorname", doctorName);
    prescriptorInfo->sync();
}
