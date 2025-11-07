#include "prescriptormainwin.h"
#include "ui_prescriptormainwin.h"

PrescriptorMainWin::PrescriptorMainWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PrescriptorMainWin)
{
    QString warningMessage;
    QStringList drugsTableHeader = {"Xoá bỏ", "Tên thuốc", "Số lượng", "Cách sử dụng"};
    searchTableHeader = {"ID", "Ngày khám", "Số điện thoại", "Họ và tên", "Ngày sinh", "Giới tính",
                        "Địa chỉ", "Kết quả khám", "Chẩn đoán", "Ghi chú", "Hạn chế ĐT",
                        "Danh sách thuốc", "Tái khám", "TLMP không kính", "Số đo MP", "TLMP có kính",
                         "Áp suất MP", "TLMT không kính", "Số đo MT", "TLMT có kính", "Áp suất mắt trái"};
    QTextStream input;
    searchCurrentRow = -1;

    dbDirPath = QDir::homePath() + "/.prescriptor/";

    ui->setupUi(this);

    dianogsisFile.setFileName(dbDirPath + "dianogsis.txt");
    drugsFile.setFileName(dbDirPath + "drugs.txt");
    drugsUseFile.setFileName(dbDirPath + "usage.txt");
    resultFile.setFileName(dbDirPath + "result.txt");

    if (!QDir(dbDirPath).exists()) {
        QDir().mkpath(dbDirPath);
    }

    patientDb.init(dbDirPath);

    if(!dianogsisFile.open(QIODevice::ReadWrite))
        warningMessage += "+> dianogsis \n";
    else {
        input.setDevice(&dianogsisFile);
        while (!input.atEnd()) dianogsisList << input.readLine();
        dianogsisFile.close();

        diagnosisListModel = new QStringListModel(dianogsisList);
        diagnosisCompleter = new QCompleter(diagnosisListModel);
        diagnosisCompleter->setCompletionMode(QCompleter::PopupCompletion);
        diagnosisCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        ui->pre_diagnosisInput->setCompleter(diagnosisCompleter);
    }

    if (!drugsFile.open(QIODevice::ReadWrite))
        warningMessage += "+> List of drugs\n";
    else {
        input.setDevice(&drugsFile);
        while (!input.atEnd()) drugsList << input.readLine();
        drugsFile.close();

        drugsListModel = new QStringListModel(drugsList);
        drugsCompleter = new QCompleter(drugsListModel);
        drugsCompleter->setCompletionMode(QCompleter::PopupCompletion);
        drugsCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        ui->pre_drugsInput->setCompleter(drugsCompleter);
    }

    if (!drugsUseFile.open(QIODevice::ReadWrite))
        warningMessage += "+> Drugs usage\n";
    else {
        input.setDevice(&drugsUseFile);
        while (!input.atEnd()) drugsUseList << input.readLine();
        drugsUseFile.close();

        drugsUseListModel = new QStringListModel(drugsUseList);
        drugsUseCompleter = new QCompleter(drugsUseListModel);
        drugsUseCompleter->setCompletionMode(QCompleter::PopupCompletion);
        drugsUseCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        ui->pre_drugUsageInput->setCompleter(drugsUseCompleter);
    }

    if (!resultFile.open(QIODevice::ReadWrite))
        warningMessage += "+> Result\n";
    else {
        input.setDevice(&resultFile);
        while (!input.atEnd()) resultList << input.readLine();
        resultFile.close();

        resultListModel = new QStringListModel(resultList);
        resultCompleter = new QCompleter(resultListModel);
        resultCompleter->setCompletionMode(QCompleter::PopupCompletion);
        resultCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        ui->pre_resultInput->setCompleter(resultCompleter);
    }

    if (warningMessage != "") {
        warningMessage = "Failed to get these autocompletion file: \n";
        QMessageBox::warning(ui->Central, "Warning", warningMessage);
    }

    printer.prepareTemplate(dbDirPath);

    ui->pre_drugsView->setColumnCount(4);
    ui->pre_drugsView->setHorizontalHeaderLabels(drugsTableHeader);
    ui->pre_drugsView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->pre_revisitperiodInput->setEnabled(false);

    patientInfoTableModel = new QSqlQueryModel();
    patientInfoTableModel->setQuery(patientDb.getQuery());
    ui->search_patientTableView->setModel(patientInfoTableModel);
    ui->search_patientTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < searchTableHeader.count(); i++) {
        if ((i > 0 && i < 9 && i != 5) || i == 11) {
            patientInfoTableModel->setHeaderData(i, Qt::Horizontal, searchTableHeader[i]);
        }
        else {
            ui->search_patientTableView->hideColumn(i);
        }
    }

    ui->prop_addrInput->setText(patientDb.shopAddress);
    ui->prop_doctorInput->setText(patientDb.doctorName);
    ui->prop_nameInput->setText(patientDb.shopName);
    ui->prop_phoneNumInput->setText(patientDb.shopPhoneNum);

    connect(ui->pre_removePreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::clearAll);
    connect(ui->pre_diagnosisInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::diagnosisEnter);
    connect(ui->pre_drugsInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::drugsNameEnter);
    connect(ui->pre_drugHowmanyInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::drugsNumEnter);
    connect(ui->pre_drugUsageInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::drugsUseEnter);
    connect(ui->pre_savePreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::savePre);
    connect(ui->pre_revisitCheckbox, &QCheckBox::checkStateChanged, this, &PrescriptorMainWin::revisitCheck);
    connect(ui->pre_resultInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::resultEnter);

    // Switch cell
    connect(ui->pre_nameInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterName);
    connect(ui->pre_addressInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterAddress);
    connect(ui->pre_birthInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterBOD);
    connect(ui->pre_phoneNumberInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterPhoneNum);
    connect(ui->pre_rightNoGlassInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterRightNoGlass);
    connect(ui->pre_rightGlassDioptreInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterRightDioptre);
    connect(ui->pre_rightGlassVisionInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterRightWithGlass);
    connect(ui->pre_rightEyePressureInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterRightPressure);
    connect(ui->pre_leftNoGlassInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterLeftNoGlass);
    connect(ui->pre_leftGlassDioptreInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterLeftDioptre);
    connect(ui->pre_leftGlassVisionInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterLeftWithGlass);
    connect(ui->pre_leftEyePressureInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::enterLeftPressure);

    connect(ui->search_searchBtn, &QPushButton::clicked, this, &PrescriptorMainWin::searchPatient);
    connect(ui->search_nameOrPhoneNumInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::searchPatient);
    connect(ui->search_patientTableView, &QTableView::clicked, this, &PrescriptorMainWin::showPatientInfo);
    connect(ui->search_editPreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::editPatientInfo);
    connect(ui->search_inheritOldPreBtn, &QPushButton::clicked, this, [this]() {inheritPatientInfo(false);});
    connect(ui->search_removePreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::deletePatientInfo);
    connect(ui->pre_printPreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::printPatientInfo);
    connect(ui->search_printPreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::printPatientInfo);
    connect(ui->prop_saveInfo, &QPushButton::clicked, this, &PrescriptorMainWin::savePrescriptorInfo);

    connect(ui->prop_backupBtn, &QPushButton::clicked, this, &PrescriptorMainWin::backupData);
    connect(ui->prop_recoverBtn, &QPushButton::clicked, this, &PrescriptorMainWin::restoreData);
}

PrescriptorMainWin::~PrescriptorMainWin()   {
    QTextStream output;
    if (dianogsisFile.open(QIODevice::WriteOnly)) {
        output.setDevice(&dianogsisFile);
        for (int i = 0; i < int(dianogsisList.count()); i++) {
            output << dianogsisList[i] << "\n";
        }
        dianogsisFile.close();
    }

    if (drugsFile.open(QIODevice::WriteOnly)) {
        output.setDevice(&drugsFile);
        for (int i = 0; i < int(drugsList.count()); i++) {
            output << drugsList[i] << "\n";
        }
        drugsFile.close();
    }

    if (drugsUseFile.open(QIODevice::WriteOnly)) {
        output.setDevice(&drugsUseFile);
        for (int i = 0; i < int(drugsUseList.count()); i++) {
            output << drugsUseList[i] << "\n";
        }
        drugsUseFile.close();
    }

    if (resultFile.open(QIODevice::WriteOnly)) {
        output.setDevice(&resultFile);
        for (int i = 0; i < int(drugsUseList.count()); i++) {
            output << drugsUseList[i] << "\n";
        }
        resultFile.close();
    }

    delete ui;
}

void PrescriptorMainWin::resizeEvent(QResizeEvent *event)   {
    ui->pre_drugsView->setColumnWidth(0, this->width() / 100 * 10);
    ui->pre_drugsView->setColumnWidth(1, this->width() / 100 * 35);
    ui->pre_drugsView->setColumnWidth(2, this->width() / 100 * 10);
    ui->pre_drugsView->horizontalHeader()->setStretchLastSection(true);
    QMainWindow::resizeEvent(event);
}

void PrescriptorMainWin::resultEnter() {
    if (ui->pre_resultInput->text() == "") return;

    if (!dianogsisList.contains(ui->pre_resultInput->text())) {
        dianogsisList << ui->pre_resultInput->text();
        dianogsisList.sort();
    }
    diagnosisListModel->setStringList(resultList);

    QString br = "";
    if (ui->pre_resultListInput->toPlainText() != "") {
        br = "\n";
    }
    ui->pre_resultListInput->setText(ui->pre_resultListInput->toPlainText()
                                        + br + ui->pre_resultInput->text());
    ui->pre_resultInput->clear();
}

void PrescriptorMainWin::diagnosisEnter() {
    if (ui->pre_diagnosisInput->text() == "") return;

    if (!dianogsisList.contains(ui->pre_diagnosisInput->text())) {
        dianogsisList << ui->pre_diagnosisInput->text();
        dianogsisList.sort();
    }
    diagnosisListModel->setStringList(dianogsisList);

    QString br = "";
    if (ui->pre_diagnosisListInput->toPlainText() != "") {
        br = "\n";
    }
    ui->pre_diagnosisListInput->setText(ui->pre_diagnosisListInput->toPlainText()
                                        + br + ui->pre_diagnosisInput->text());
    ui->pre_diagnosisInput->clear();
}

void PrescriptorMainWin::drugsNameEnter() {
    if (ui->pre_drugsInput->text() == "") return;

    if (!drugsList.contains(ui->pre_drugsInput->text())) {
        drugsList << ui->pre_drugsInput->text();
        drugsList.sort();
    }
    drugsListModel->setStringList(drugsList);
    ui->pre_drugHowmanyInput->setFocus();
}

void PrescriptorMainWin::drugsNumEnter() {
    if (ui->pre_drugHowmanyInput->text() == "") return;
    ui->pre_drugUsageInput->setFocus();
}

void PrescriptorMainWin::drugsUseEnter() {
    int rowPos = ui->pre_drugsView->rowCount();
    QPushButton *delDrugbtn = new QPushButton("Xoá bỏ");

    connect(delDrugbtn, &QPushButton::clicked, this, &PrescriptorMainWin::deleteDrug);
    ui->pre_drugsView->insertRow(rowPos);
    ui->pre_drugsView->setCellWidget(rowPos, 0, delDrugbtn);
    ui->pre_drugsView->setItem(rowPos, 1, new QTableWidgetItem(ui->pre_drugsInput->text()));
    ui->pre_drugsView->setItem(rowPos, 2, new QTableWidgetItem(ui->pre_drugHowmanyInput->text()));
    ui->pre_drugsView->setItem(rowPos, 3, new QTableWidgetItem(ui->pre_drugUsageInput->text()));

    if (!drugsUseList.contains(ui->pre_drugUsageInput->text())) {
        drugsUseList << ui->pre_drugUsageInput->text();
        drugsUseList.sort();
    }
    drugsUseListModel->setStringList(drugsUseList);

    ui->pre_drugsInput->clear();
    ui->pre_drugHowmanyInput->clear();
    ui->pre_drugUsageInput->clear();
    ui->pre_drugsInput->setFocus();
}

void PrescriptorMainWin::deleteDrug() {
    ui->pre_drugsView->removeRow(ui->pre_drugsView->currentRow());
}

void PrescriptorMainWin::revisitCheck() {
    ui->pre_revisitperiodInput->setEnabled(ui->pre_revisitCheckbox->isChecked());
}

void PrescriptorMainWin::showPatientInfo() {
    setSearchCurrentRow();
    ui->search_printPreBtn->setEnabled(true);
    QString allInfo;
    for (int i = 1; i < searchTableHeader.count(); i++) {
        if (i == 5) {
            if (getCellData(searchCurrentRow, i).toBool() == PatientCase::Male) {
                allInfo += searchTableHeader[i] + ": " + "Nam" + "\n";
            }
            else {
                allInfo += searchTableHeader[i] + ": " + "Nữ" + "\n";
            }
        }
        else {
            allInfo += searchTableHeader[i] + ": " + getCellData(searchCurrentRow, i).toString() + "\n";
        }
    }
    ui->search_patientInfoView->setText(allInfo);

}

void PrescriptorMainWin::editPatientInfo() {
    inheritPatientInfo(true);

    ui->pre_resultListInput->setText(patientCase.result);
    ui->pre_diagnosisListInput->setText(patientCase.diagnosis);
    ui->pre_noteInput->setText(patientCase.note);
    ui->pre_warnNoPhoneCheckbox->setChecked(patientCase.warnNoPhone);
    ui->pre_revisitperiodInput->setText(patientCase.revisitPeriod);
    if (patientCase.revisitPeriod != "") ui->pre_revisitCheckbox->setChecked(true);
    ui->pre_rightNoGlassInput->setText(patientCase.rightNoGlass);
    ui->pre_rightGlassDioptreInput->setText(patientCase.rightDioptre);
    ui->pre_rightGlassVisionInput->setText(patientCase.rightWithGlass);
    ui->pre_rightEyePressureInput->setText(patientCase.rightPressure);
    ui->pre_leftNoGlassInput->setText(patientCase.leftNoGlass);
    ui->pre_leftGlassDioptreInput->setText(patientCase.leftDioptre);
    ui->pre_leftGlassVisionInput->setText(patientCase.leftWithGlass);
    ui->pre_leftEyePressureInput->setText(patientCase.leftPressure);

    for (int i = 0; i < patientCase.drugsList.count(); i++) {
        int row = ui->pre_drugsView->rowCount();
        QPushButton *delDrugbtn = new QPushButton("Xoá bỏ");

        ui->pre_drugsView->insertRow(row);
        ui->pre_drugsView->setCellWidget(row, 0, delDrugbtn);
        ui->pre_drugsView->setItem(row, 1, new QTableWidgetItem(patientCase.drugsList[i][0]));
        ui->pre_drugsView->setItem(row, 2, new QTableWidgetItem(patientCase.drugsList[i][1]));
        ui->pre_drugsView->setItem(row, 3, new QTableWidgetItem(patientCase.drugsList[i][2]));
        connect(delDrugbtn, &QPushButton::clicked, this, &PrescriptorMainWin::deleteDrug);
    }
}

void PrescriptorMainWin::inheritPatientInfo(bool copyFull) {
    copyPatientInfo(copyFull);

    ui->pre_nameInput->setText(patientCase.name);
    ui->pre_birthInput->setText(patientCase.dateOfBirth);
    ui->pre_phoneNumberInput->setText(patientCase.phoneNumber);
    ui->pre_ifFemale->setChecked(patientCase.gender);
    ui->pre_addressInput->setText(patientCase.address);

    ui->tabWidget->setCurrentIndex(0);
}

void PrescriptorMainWin::copyPatientInfo(bool copyFull) {
    clearAll();
    setSearchCurrentRow();

    patientCase.prescribeDate = getCellData(searchCurrentRow, 1).toString();
    patientCase.phoneNumber = getCellData(searchCurrentRow, 2).toString();
    patientCase.name = getCellData(searchCurrentRow, 3).toString();
    patientCase.dateOfBirth = getCellData(searchCurrentRow, 4).toString();
    patientCase.gender = getCellData(searchCurrentRow, 5).toBool();
    patientCase.address = getCellData(searchCurrentRow, 6).toString();

    if (copyFull == true) {
        patientCase.id = getCellData(searchCurrentRow, 0).toInt();
        patientCase.result = getCellData(searchCurrentRow, 7).toString();
        patientCase.diagnosis = getCellData(searchCurrentRow, 8).toString();
        patientCase.note = getCellData(searchCurrentRow, 9).toString();
        patientCase.warnNoPhone = getCellData(searchCurrentRow, 10).toBool();
        patientCase.revisitPeriod = getCellData(searchCurrentRow, 12).toString();
        patientCase.rightNoGlass = getCellData(searchCurrentRow, 13).toString();
        patientCase.rightDioptre = getCellData(searchCurrentRow, 14).toString();
        patientCase.rightWithGlass = getCellData(searchCurrentRow, 15).toString();
        patientCase.rightPressure = getCellData(searchCurrentRow, 16).toString();
        patientCase.leftNoGlass = getCellData(searchCurrentRow, 17).toString();
        patientCase.leftDioptre = getCellData(searchCurrentRow, 18).toString();
        patientCase.leftWithGlass = getCellData(searchCurrentRow, 19).toString();
        patientCase.leftPressure = getCellData(searchCurrentRow, 20).toString();
        QStringList temp = getCellData(searchCurrentRow, 11).toString().split("\n", Qt::SkipEmptyParts);
        for (int i = 0; i < temp.count(); i++) {
            qDebug() << temp[i];
            patientCase.drugsList.append(temp[i].split("::"));
        }
    }
}

QVariant PrescriptorMainWin::getCellData(int row, int column) {
    return patientInfoTableModel->data(patientInfoTableModel->index(row, column));
}

void PrescriptorMainWin::searchPatient() {
    searchCurrentRow = -1;
    ui->search_printPreBtn->setEnabled(false);
    bool converted;
    QString info = ui->search_nameOrPhoneNumInput->text();
    info.toInt(&converted, 10);

    if (converted == true)
        patientInfoTableModel->setQuery(patientDb.getQuery("AND (phonenum LIKE '%" + info + "%')"));
    else
        patientInfoTableModel->setQuery(patientDb.getQuery("AND (name LIKE '%" + info + "%')"));
}

void PrescriptorMainWin::setSearchCurrentRow() {
        searchCurrentRow = ui->search_patientTableView->selectionModel()->selectedIndexes()[0].row();
}

void PrescriptorMainWin::deletePatientInfo() {
    setSearchCurrentRow();
    patientDb.query->exec("DELETE FROM patients "
                          "WHERE id = '" + getCellData(searchCurrentRow, 0).toString() + "'");
    patientInfoTableModel->setQuery(patientDb.getQuery());
}

void PrescriptorMainWin::savePre() {
    if (ui->pre_nameInput->text() == "") {
        QMessageBox::critical(nullptr, "Lỗi", "Tên bệnh nhân không được để trống!");
        return;
    }

    if (patientCase.id != 0 && ui->pre_nameInput->text() == patientCase.name) {
        patientDb.query->exec("DELETE FROM patients "
                            "WHERE id = '" + QString::number(patientCase.id) + "'");
    }

    patientDb.query->exec("SELECT id "
                          "FROM patients "
                          "ORDER BY id DESC "
                          "LIMIT 1");
    while (patientDb.query->next()) {
        patientCase.id = patientDb.query->value(0).toInt() + 1;
    }

    patientCase.prescribeDate = QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss");
    patientCase.name = ui->pre_nameInput->text();
    patientCase.dateOfBirth = ui->pre_birthInput->text();
    patientCase.gender = ui->pre_ifFemale->isChecked();
    patientCase.address = ui->pre_addressInput->text();
    patientCase.phoneNumber = ui->pre_phoneNumberInput->text();
    patientCase.result = ui->pre_resultListInput->toPlainText();
    patientCase.diagnosis = ui->pre_diagnosisListInput->toPlainText();
    patientCase.note = ui->pre_noteInput->text();
    patientCase.warnNoPhone = ui->pre_warnNoPhoneCheckbox->isChecked();
    patientCase.revisitPeriod = ui->pre_revisitperiodInput->text();
    patientCase.rightNoGlass = ui->pre_rightNoGlassInput->text();
    patientCase.rightWithGlass = ui->pre_rightGlassVisionInput->text();
    patientCase.rightDioptre = ui->pre_rightGlassDioptreInput->text();
    patientCase.rightPressure = ui->pre_rightEyePressureInput->text();
    patientCase.leftNoGlass = ui->pre_leftNoGlassInput->text();
    patientCase.leftWithGlass = ui->pre_leftGlassVisionInput->text();
    patientCase.leftDioptre = ui->pre_leftGlassDioptreInput->text();
    patientCase.leftPressure = ui->pre_leftEyePressureInput->text();

    patientCase.drugsList.clear();
    for (int i = 0; i < ui->pre_drugsView->rowCount(); i++) {
        QStringList temp;
        temp << ui->pre_drugsView->item(i, 1)->text()
             << ui->pre_drugsView->item(i, 2)->text()
             << ui->pre_drugsView->item(i, 3)->text();
        patientCase.drugsList.append(temp);
    }

    bool res = patientCase.saveToDb(patientDb);
    if (res == false) {
        QMessageBox::critical(nullptr, "Error", "Cannot insert patient information into database: " + patientDb.query->lastError().text());
        ui->pre_stateLabel->setText("Không lưu được thông tin bệnh nhân!");
    }

    if (res == true) {
        ui->pre_stateLabel->setText("Đã lưu thành công!");
    }

    patientInfoTableModel->setQuery(patientDb.getQuery());
}

void PrescriptorMainWin::printPatientInfo() {
    if (ui->tabWidget->currentIndex() == 0) {
        savePre();
    }
    else if (ui->tabWidget->currentIndex() == 1 && searchCurrentRow != -1) {
        copyPatientInfo(true);
    }
    printer.exportAndShowPrescription(patientCase, patientDb);
}

void PrescriptorMainWin::savePrescriptorInfo() {
    patientDb.shopName = ui->prop_nameInput->text();
    patientDb.shopPhoneNum = ui->prop_phoneNumInput->text();
    patientDb.shopAddress = ui->prop_addrInput->text();
    patientDb.doctorName = ui->prop_doctorInput->text();
    patientDb.savePrescriptorInfo();
    ui->prop_currentState->setText("Đã lưu thông tin thành công!");
}

void PrescriptorMainWin::clearAll()
{
    ui->pre_addressInput->clear();
    ui->pre_birthInput->setText("01/01/2000");
    ui->pre_diagnosisInput->clear();
    ui->pre_diagnosisListInput->clear();
    ui->pre_drugsInput->clear();
    ui->pre_drugHowmanyInput->clear();
    ui->pre_drugUsageInput->clear();
    ui->pre_drugsView->model()->removeRows(0, ui->pre_drugsView->model()->rowCount());
    ui->pre_leftEyePressureInput->clear();
    ui->pre_leftGlassDioptreInput->clear();
    ui->pre_leftGlassVisionInput->clear();
    ui->pre_leftNoGlassInput->clear();
    ui->pre_nameInput->clear();
    ui->pre_noteInput->clear();
    ui->pre_phoneNumberInput->clear();
    ui->pre_revisitCheckbox->setChecked(false);
    ui->pre_revisitperiodInput->clear();
    ui->pre_revisitperiodInput->setEnabled(false);
    ui->pre_rightEyePressureInput->clear();
    ui->pre_rightGlassDioptreInput->clear();
    ui->pre_rightGlassVisionInput->clear();
    ui->pre_rightNoGlassInput->clear();

    patientCase.clear();
}

void PrescriptorMainWin::backupData() {
    QString backupDir = bacAndRecDialog.getExistingDirectory(this, "Sao lưu dữ liệu", QDir::homePath(), QFileDialog::ShowDirsOnly)
                        + "/Backup-" + QDateTime::currentDateTime().toString("dd-MM-yyyy-hh-mm-ss");
    QDirIterator it(dbDirPath, QDirIterator::Subdirectories);
    QDir().mkpath(backupDir);
    while (it.hasNext()) {
        QString sourcePath = it.next();
        QStringList fileName = sourcePath.split("/");

        QFile(sourcePath).copy(backupDir + "/" + fileName[fileName.count() - 1]);
    }

    ui->prop_currentState->setText("Đã sao lưu thành công!");
}

void PrescriptorMainWin::restoreData() {
    QString restoreDir = bacAndRecDialog.getExistingDirectory(this, "Khôi phục dữ liệu", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!QFile(restoreDir + "/prescriptions.db").exists() || !QFile(restoreDir + "/template.html").exists()) {
        QMessageBox::critical(nullptr, "Lỗi", "Bản sao lưu thiếu mẫu đơn (template.html) hoặc dữ liệu bệnh nhân (prescriptions.db)");
        return;
    }

    QDir dir(dbDirPath);
    dir.removeRecursively();

    QDirIterator it(restoreDir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString sourcePath = it.next();
        QStringList fileName = sourcePath.split("/");

        QFile(sourcePath).copy(dbDirPath + "/" + fileName[fileName.count() - 1]);
    }
    ui->prop_currentState->setText("Đã khôi phục thành công!");
}

// Switching cell
void PrescriptorMainWin::enterName() {
    ui->pre_addressInput->setFocus();
}

void PrescriptorMainWin::enterAddress() {
    ui->pre_birthInput->setFocus();
    ui->pre_birthInput->setCursorPosition(0);
}

void PrescriptorMainWin::enterBOD() {
    ui->pre_YearsOld->setText("(" + QString::number(QDate::currentDate().year() - ui->pre_birthInput->text().split('/')[2].toInt()) +" tuổi)");
    ui->pre_phoneNumberInput->setFocus();
}

void PrescriptorMainWin::enterPhoneNum() {
    ui->pre_rightNoGlassInput->setFocus();
}

void PrescriptorMainWin::enterRightNoGlass() {
    ui->pre_rightGlassDioptreInput->setFocus();
}

void PrescriptorMainWin::enterRightDioptre() {
    ui->pre_rightGlassVisionInput->setFocus();
}

void PrescriptorMainWin::enterRightWithGlass() {
    ui->pre_rightEyePressureInput->setFocus();
}

void PrescriptorMainWin::enterRightPressure() {
    ui->pre_leftNoGlassInput->setFocus();
}

void PrescriptorMainWin::enterLeftNoGlass() {
    ui->pre_leftGlassDioptreInput->setFocus();
}

void PrescriptorMainWin::enterLeftDioptre() {
    ui->pre_leftGlassVisionInput->setFocus();
}

void PrescriptorMainWin::enterLeftWithGlass() {
    ui->pre_leftEyePressureInput->setFocus();
}

void PrescriptorMainWin::enterLeftPressure() {
    ui->pre_resultInput->setFocus();
}
