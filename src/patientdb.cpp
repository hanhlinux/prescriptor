#include "patientdb.h"

PatientDb::PatientDb() {
    init("");
}

PatientDb::PatientDb(const QString &path) {
    init(path);
}

PatientDb::~PatientDb() {
    db.close();
}

QString PatientDb::getQuery(const QString &inp) {
    return "SELECT * FROM patients WHERE id > 0 " + inp + " ORDER BY id DESC";
}

void PatientDb::prepareVariables(const QString &path) {
    dbDirPath = path;
    drugsTableHeader = {"Xoá bỏ", "Tên thuốc", "Số lượng", "Cách sử dụng"};
    searchTableHeader = {"ID", "Ngày khám", "Số điện thoại", "Họ và tên", "Ngày sinh/Tuổi", "Giới tính",
                         "Địa chỉ", "Kết quả khám", "Chẩn đoán", "Ghi chú", "Hạn chế ĐT",
                         "Danh sách thuốc", "Tái khám", "TLMP không kính", "Số đo MP", "TLMP có kính",
                         "Áp suất MP", "TLMT không kính", "Số đo MT", "TLMT có kính", "Áp suất mắt trái"};
    completionList = {"diagnosis", "result", "drugs", "usage"};;
    dbPath = dbDirPath + "/prescriptions.db";

    dbGenCommand << "create table patients(";
    dbGenCommand << "id int NOT NULL primary key, ";
    dbGenCommand << "prescribeDate varchar(20), ";
    dbGenCommand << "phonenum varchar(10) , ";
    dbGenCommand << "name varchar(50), ";
    dbGenCommand << "dateofbirth varchar(10), ";
    dbGenCommand << "gender bool, ";
    dbGenCommand << "address varchar(100), ";
    dbGenCommand << "result text, ";
    dbGenCommand << "diagnosis text, ";
    dbGenCommand << "note text, ";
    dbGenCommand << "warnNoPhone bool, ";
    dbGenCommand << "drugs text, ";
    dbGenCommand << "revisitPeriod varchar(30), ";
    dbGenCommand << "rightNoGlass varchar(10),";
    dbGenCommand << "rightDioptre varchar(10),";
    dbGenCommand << "rightWithGlass varchar(10),";
    dbGenCommand << "rightPressure varchar(10),";
    dbGenCommand << "leftNoGlass varchar(10), ";
    dbGenCommand << "leftDioptre varchar(10),";
    dbGenCommand << "leftWithGlass varchar(10),";
    dbGenCommand << "leftPressure varchar(10))";

    prescriptorInfo = new QSettings(dbDirPath + "/settings.ini", QSettings::IniFormat);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
}

void PatientDb::prepareSettings() {
    if (!QFile(dbDirPath + "/settings.ini").exists()) {
        prescriptorInfo->setValue("info/shopname", "");
        prescriptorInfo->setValue("info/shopphonenum", "");
        prescriptorInfo->setValue("info/shopaddress", "");
        prescriptorInfo->setValue("info/doctorname", "");
        prescriptorInfo->sync();
    }
    else {
        shopName = prescriptorInfo->value("info/shopname").toString();
        shopAddress = prescriptorInfo->value("info/shopaddress").toString();
        shopPhoneNum = prescriptorInfo->value("info/shopphonenum").toString();
        doctorName = prescriptorInfo->value("info/doctorname").toString();
    }
}

int PatientDb::init(const QString &path) {
    if (path == "") return 0; // do nothing

    prepareVariables(path);
    prepareSettings();


    bool dbExist = QFile(dbPath).exists();
    bool isDBOpen = db.open();
    query = new QSqlQuery(db);
    if (isDBOpen == true && dbExist == false) {
        bool exec = query->exec(dbGenCommand.join(""));
        if (exec == false) {
            QFile(dbPath).remove();
            return CreationErr;
        }

        query->exec("INSERT INTO patients(id)"
                    "VALUES(0)");
    }
    else if (isDBOpen == false) {
        return ConnectionErr;
    }
    query->first();

    return Success;
}

void PatientDb::savePrescriptorInfo() {
    prescriptorInfo->setValue("info/shopname", shopName);
    prescriptorInfo->setValue("info/shopphonenum", shopPhoneNum);
    prescriptorInfo->setValue("info/shopaddress", shopAddress);
    prescriptorInfo->setValue("info/doctorname", doctorName);
    prescriptorInfo->sync();
}

bool PatientDb::backupData(QString dest) {
    QDirIterator it(dbDirPath, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);

    if (!QDir().mkpath(dest))
        return false;

    while (it.hasNext()) {
        QString sourcePath = it.next();
        QStringList fileName = sourcePath.split("/");

        if (!QFile(sourcePath).copy(dest + "/" + fileName[fileName.count() - 1]))
            return false;
    }

    return true;
}

int PatientDb::restoreData(QString source) {
    if (!QFile(source + "/prescriptions.db").exists() || !QFile(source + "/template.html").exists()) {
        return CorruptedBackup;
    }

    db.close();

    QDir dir(dbDirPath);
    dir.removeRecursively();
    dir.mkpath(dbDirPath);

    QDirIterator it(source, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString sourcePath = it.next();
        QStringList fileName = sourcePath.split("/");

        if (!QFile(sourcePath).copy(dbDirPath + "/" + fileName[fileName.count() - 1]))
            return RWError;
    }

    db.open();
    return Success;
}

bool PatientDb::removePatient(const QString &inp) {
     return query->exec("DELETE FROM patients "
                "WHERE id = '" + inp + "'");
}
int PatientDb::getNextID() {
    bool success = query->exec("SELECT id "
                "FROM patients "
                "ORDER BY id DESC "
                "LIMIT 1");
    if (success == false) return -1;
    int id;
    while (query->next()) {
        id = query->value(0).toInt() + 1;
        break;
    }
    return id;
}


