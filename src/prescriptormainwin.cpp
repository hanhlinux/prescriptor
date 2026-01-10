#include "prescriptormainwin.h"
#include "ui_prescriptormainwin.h"

PrescriptorMainWin::PrescriptorMainWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PrescriptorMainWin)
{
    ui->setupUi(this);

    prepareResources();
    prepareUIComponents();
    setupPrescribeTab();
    setupSearchTab();
    setupPropertiesTab();
}

PrescriptorMainWin::~PrescriptorMainWin() {
    for (int i = 0; i < 4; i++) {
        if(dataCompletion[i].success)
            dataCompletion[i].save();
    }

    delete ui;
}

void PrescriptorMainWin::operationError(const QString &msg) {
    QMessageBox::critical(nullptr, "Lỗi", msg);
}

void PrescriptorMainWin::fatalError(const QString &msg) {
    operationError(msg);
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}


void PrescriptorMainWin::prepareResources() {
    bool allCompletionSuccess = true;
    dbDirPath = QDir::homePath() + "/.prescriptor/";

    if (!QDir(dbDirPath).exists())
        QDir().mkpath(dbDirPath);

    int initSuccess = patientDb.init(dbDirPath);
    if (initSuccess == PatientDb::CreationErr)
        fatalError("Không tạo được CSDL để lưu đơn thuốc!");
    else if (initSuccess == PatientDb::ConnectionErr)
        fatalError("Không kết nối được với CSDL sử dụng lưu đơn thuốc!");

    if(!printer.prepareTemplate(dbDirPath, ui->tabWidget->widget(3), patientDb.fontSize))
        fatalError("Thiếu file mẫu đơn của phần mềm HOẶC file mẫu đơn không thể mở được");

    for (int i = 0; i < int(patientDb.completionList.size()); i++) {
        dataCompletion[i].setup(dbDirPath, patientDb.completionList[i]);
        if (!dataCompletion[i].success && allCompletionSuccess) {
            allCompletionSuccess = false;
            QMessageBox::warning(ui->Central, "Warning", "Không thể đọc một số danh sách dữ liệu gợi ý bệnh có sẵn");
        }
    }

    if (dataCompletion[DataCompletion::Diagnosis].completer != nullptr)
        ui->pre_diagnosisInput->setCompleter(dataCompletion[DataCompletion::Diagnosis].completer);
    if (dataCompletion[DataCompletion::Result].completer != nullptr)
        ui->pre_resultInput->setCompleter(dataCompletion[DataCompletion::Result].completer);
    if (dataCompletion[DataCompletion::Drugs].completer != nullptr)
        ui->pre_drugsInput->setCompleter(dataCompletion[DataCompletion::Drugs].completer);
    if (dataCompletion[DataCompletion::DrugsUsage].completer != nullptr)
        ui->pre_drugUsageInput->setCompleter(dataCompletion[DataCompletion::DrugsUsage].completer);
}

void PrescriptorMainWin::prepareUIComponents() {
    searchCurrentRow = -1;

    windowScroll = new QScrollArea();
    windowScroll->setWidget(ui->PrescriptTab);
    windowScroll->setWidgetResizable(true);
    ui->tabWidget->insertTab(0, windowScroll, "Kê đơn thuốc");
    ui->tabWidget->setCurrentIndex(0);

    ui->pre_drugsView->setColumnCount(4);
    ui->pre_drugsView->setHorizontalHeaderLabels(patientDb.drugsTableHeader);
    ui->pre_drugsView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->pre_revisitperiodInput->setEnabled(false);

    patientInfoTableModel = new QSqlQueryModel();
    patientInfoTableModel->setQuery(patientDb.getQuery());
    ui->search_patientTableView->setModel(patientInfoTableModel);
    ui->search_patientTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < patientDb.searchTableHeader.count(); i++) {
        if (i > PatientCase::ID && i < PatientCase::Result && i != PatientCase::Sex)
            patientInfoTableModel->setHeaderData(i, Qt::Horizontal, patientDb.searchTableHeader[i]);
        else
            ui->search_patientTableView->hideColumn(i);
    }

    ui->prop_addrInput->setText(patientDb.shopAddress);
    ui->prop_doctorInput->setText(patientDb.doctorName);
    ui->prop_nameInput->setText(patientDb.shopName);
    ui->prop_phoneNumInput->setText(patientDb.shopPhoneNum);
}

void PrescriptorMainWin::setupPrescribeTab() {
    switchCell(ui->pre_nameInput, ui->pre_addressInput);
    switchCell(ui->pre_addressInput, ui->pre_birthInput);
    switchCell(ui->pre_birthInput, ui->pre_phoneNumberInput);
    switchCell(ui->pre_phoneNumberInput, ui->pre_rightNoGlassInput);
    switchCell(ui->pre_rightNoGlassInput, ui->pre_rightGlassDioptreInput);
    switchCell(ui->pre_rightGlassDioptreInput, ui->pre_rightGlassVisionInput);
    switchCell(ui->pre_rightGlassVisionInput, ui->pre_rightEyePressureInput);
    switchCell(ui->pre_rightEyePressureInput, ui->pre_leftNoGlassInput);
    switchCell(ui->pre_leftNoGlassInput, ui->pre_leftGlassDioptreInput);
    switchCell(ui->pre_leftGlassDioptreInput, ui->pre_leftGlassVisionInput);
    switchCell(ui->pre_leftGlassVisionInput, ui->pre_leftEyePressureInput);
    switchCell(ui->pre_leftEyePressureInput, ui->pre_resultInput);


    connect(ui->pre_removePreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::clearAll);
    connect(ui->pre_drugsInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::drugsNameEnter);
    connect(ui->pre_drugHowmanyInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::drugsNumEnter);
    connect(ui->pre_drugUsageInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::drugsUseEnter);
    connect(ui->pre_savePreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::savePre);
    connect(ui->pre_revisitCheckbox, &QCheckBox::checkStateChanged, this, &PrescriptorMainWin::revisitCheck);
    // Dirty-fix approach for lineEdit completion
    connect(ui->pre_resultInput, &QLineEdit::returnPressed, this,
            [this]() {resAndDiagEnter(ui->pre_resultInput, ui->pre_resultListInput, ui->pre_diagnosisInput, DataCompletion::Result);}, Qt::QueuedConnection);
    connect(ui->pre_diagnosisInput, &QLineEdit::returnPressed, this,
            [this]() {resAndDiagEnter(ui->pre_diagnosisInput, ui->pre_diagnosisListInput, ui->pre_drugsInput, DataCompletion::Diagnosis);}, Qt::QueuedConnection);

    connect(dataCompletion[0].completer, qOverload<const QString&>(&QCompleter::activated),
            this, [this]() {ui->pre_resultInput->clear();}, Qt::QueuedConnection);
    connect(dataCompletion[1].completer, qOverload<const QString&>(&QCompleter::activated),
            this, [this]() {ui->pre_diagnosisInput->clear();}, Qt::QueuedConnection);
    connect(ui->pre_printPreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::printPatientInfo);
}

void PrescriptorMainWin::setupSearchTab() {
    connect(ui->search_searchBtn, &QPushButton::clicked, this, &PrescriptorMainWin::searchPatient);
    connect(ui->search_nameOrPhoneNumInput, &QLineEdit::returnPressed, this, &PrescriptorMainWin::searchPatient);
    connect(ui->search_patientTableView, &QTableView::clicked, this, &PrescriptorMainWin::showPatientInfo);
    connect(ui->search_editPreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::editPatientInfo);
    connect(ui->search_inheritOldPreBtn, &QPushButton::clicked, this, [this]() {inheritPatientInfo(false);});
    connect(ui->search_removePreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::deletePatientInfo);
    connect(ui->search_printPreBtn, &QPushButton::clicked, this, &PrescriptorMainWin::printPatientInfo);
}

void PrescriptorMainWin::setupPropertiesTab() {
    connect(ui->prop_saveInfo, &QPushButton::clicked, this, &PrescriptorMainWin::savePrescriptorInfo);
    connect(ui->prop_backupBtn, &QPushButton::clicked, this, &PrescriptorMainWin::backupData);
    connect(ui->prop_recoverBtn, &QPushButton::clicked, this, &PrescriptorMainWin::restoreData);
}

void PrescriptorMainWin::resizeEvent(QResizeEvent *event)   {
    ui->pre_drugsView->setColumnWidth(0, this->width() / 100 * 10);
    ui->pre_drugsView->setColumnWidth(1, this->width() / 100 * 35);
    ui->pre_drugsView->setColumnWidth(2, this->width() / 100 * 10);
    ui->pre_drugsView->horizontalHeader()->setStretchLastSection(true);
    QMainWindow::resizeEvent(event);
}

void PrescriptorMainWin::resAndDiagEnter(QLineEdit *current, QTextEdit *list, QLineEdit *next, int type) {
    if (current->text() == "") {
        next->setFocus();
        return;
    }

    if (!dataCompletion[type].content.contains(current->text()))
        dataCompletion[type].update(current->text());

    QString br = "";
    if (list->toPlainText() != "")
        br = "\n";
    list->setText(list->toPlainText() + br + current->text());
    current->clear();
}

void PrescriptorMainWin::drugsNameEnter() {
    if (ui->pre_drugsInput->text() == "") return;

    if (!dataCompletion[2].content.contains(ui->pre_drugsInput->text()))
        dataCompletion[2].update(ui->pre_drugsInput->text());
    ui->pre_drugHowmanyInput->setFocus();
}

void PrescriptorMainWin::drugsNumEnter() {
    if (ui->pre_drugHowmanyInput->text() == "") return;
    ui->pre_drugUsageInput->setFocus();
}

void PrescriptorMainWin::insertDrugRow(const QString &name, const QString &num, const QString &usage) {
    int rowPos = ui->pre_drugsView->rowCount();
    QPushButton *delDrugbtn = new QPushButton("Xoá bỏ");

    connect(delDrugbtn, &QPushButton::clicked, this, &PrescriptorMainWin::deleteDrug);
    ui->pre_drugsView->insertRow(rowPos);
    ui->pre_drugsView->setCellWidget(rowPos, 0, delDrugbtn);
    ui->pre_drugsView->setItem(rowPos, 1, new QTableWidgetItem(name));
    ui->pre_drugsView->setItem(rowPos, 2, new QTableWidgetItem(num));
    ui->pre_drugsView->setItem(rowPos, 3, new QTableWidgetItem(usage));
}

void PrescriptorMainWin::drugsUseEnter() {
    insertDrugRow(ui->pre_drugsInput->text(), ui->pre_drugHowmanyInput->text(), ui->pre_drugUsageInput->text());

    if (!dataCompletion[3].content.contains(ui->pre_drugUsageInput->text()))
        dataCompletion[3].update(ui->pre_drugUsageInput->text());

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
    loadSearchPatientInfo();
}

void PrescriptorMainWin::loadSearchPatientInfo() {
    QString result = cellCol(PatientCase::Result).toString();
    QString diagnosis = cellCol(PatientCase::Diagnosis).toString();
    QStringList drugsList = cellCol(PatientCase::Drugs).toString().split("\n", Qt::SkipEmptyParts);
    QStringList pBold = {"p", "strong"};
    QStringList tdBold = {"td", "strong"};

    displayContent.clear();
    result.replace("\n", "<br>");
    diagnosis.replace("\n", "<br>");

    insertInfo(patientDb.searchTableHeader[PatientCase::PrescribeDate], cellCol(PatientCase::PrescribeDate).toString());
    insertInfo(patientDb.searchTableHeader[PatientCase::Name], cellCol(PatientCase::Name).toString());
    if (cellCol(PatientCase::Sex).toBool() == PatientCase::Male)
        insertInfo(patientDb.searchTableHeader[PatientCase::Sex], "Nam");
    else
        insertInfo(patientDb.searchTableHeader[PatientCase::Sex], "Nữ");
    insertInfo(patientDb.searchTableHeader[PatientCase::BOD], cellCol(PatientCase::BOD).toString());
    insertInfo(patientDb.searchTableHeader[PatientCase::PhoneNumber], cellCol(PatientCase::PhoneNumber).toString());
    insertInfo(patientDb.searchTableHeader[PatientCase::Address], cellCol(PatientCase::Address).toString());

    displayContent << "<table width=100% border=1><tr>\n";
    insertLine(pBold, "Thị lực: ");
    insertLine("td", "&nbsp;");
    insertLine(tdBold, "Thị lực không kính (X/10)");
    insertLine(tdBold, "Số đo độ (Dioptre)");
    insertLine(tdBold, "Thị lực có kính (X/10)");
    insertLine(tdBold, "Nhãn áp (mmHg)");
    displayContent << "</tr><tr>";
    insertLine(tdBold, "MP");
    insertLine("td", cellCol(PatientCase::RNG).toString());
    insertLine("td", cellCol(PatientCase::RD).toString());
    insertLine("td", cellCol(PatientCase::RWG).toString());
    insertLine("td", cellCol(PatientCase::RP).toString());
    displayContent << "</tr><tr>";
    insertLine(tdBold, "MT");
    insertLine("td", cellCol(PatientCase::LNG).toString());
    insertLine("td", cellCol(PatientCase::LD).toString());
    insertLine("td", cellCol(PatientCase::LWG).toString());
    insertLine("td", cellCol(PatientCase::LP).toString());
    displayContent << "</tr></table>";

    insertLine(pBold, "Kết quả khám:");
    insertLine("p", result);

    insertLine(pBold, "Chẩn đoán:");
    insertLine("p", diagnosis);

    insertLine(pBold, "Danh sách thuốc:");
    displayContent << "<ol>";
    for (int i = 0; i < drugsList.count(); i++) {
        QString res = drugsList[i].replace("::", ", ") + ".";
        insertLine("li", res);
    }
    displayContent << "</ol>";

    insertLine(pBold, "Ghi chú: ");
    displayContent << "<ul>";
    if (cellCol(PatientCase::Note).toString().length() > 0)
        insertLine("li", cellCol(PatientCase::Note).toString());
    if (cellCol(PatientCase::RevisitPeriod).toString().length() > 0)
        insertLine("li", cellCol(PatientCase::RevisitPeriod).toString());
    if (cellCol(PatientCase::WarnNoPhone).toBool())
        insertLine("li", "Hạn chế tiếp xúc với các thiết bị điện tử như TV, iPad (tablet), điện thoại thông minh,...");
    displayContent << "</ul>";

    ui->search_patientInfoView->setHtml(displayContent.join("\n"));
}

void PrescriptorMainWin::insertLine(const QString &tag, const QString &content, QString format) {
    displayContent << "<" + tag + " " + format + " >" + content + "</" + tag + ">";
}

void PrescriptorMainWin::insertLine(const QStringList &tag, const QString &content) {
    QString res;
    for (int i = 0; i < int(tag.count()); i++) {
        res += "<" + tag[i] + ">";
    }
    res += content;
    for (int i = int(tag.count()) - 1; i >= 0; i--) {
        res += "</" + tag[i] + ">";
    }

    displayContent << res;
}

void PrescriptorMainWin::insertInfo(const QString &key, const QString &val) {
    QString line = "<p><strong>key: </strong>val</p>";
    line.replace("key", key);
    line.replace("val", val);
    displayContent << line;
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

    for (int i = 0; i < patientCase.drugsList.count(); i++)
        insertDrugRow(patientCase.drugsList[i][0], patientCase.drugsList[i][1], patientCase.drugsList[i][2]);
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

    patientCase.prescribeDate = cellCol(1).toString();
    patientCase.phoneNumber = cellCol(2).toString();
    patientCase.name = cellCol(3).toString();
    patientCase.dateOfBirth = cellCol(4).toString();
    patientCase.gender = cellCol(5).toBool();
    patientCase.address = cellCol(6).toString();

    if (copyFull == true) {
        patientCase.id = cellCol(0).toInt();
        patientCase.result = cellCol(7).toString();
        patientCase.diagnosis = cellCol(8).toString();
        patientCase.note = cellCol(9).toString();
        patientCase.warnNoPhone = cellCol(10).toBool();
        patientCase.revisitPeriod = cellCol(12).toString();
        patientCase.rightNoGlass = cellCol(13).toString();
        patientCase.rightDioptre = cellCol(14).toString();
        patientCase.rightWithGlass = cellCol(15).toString();
        patientCase.rightPressure = cellCol(16).toString();
        patientCase.leftNoGlass = cellCol(17).toString();
        patientCase.leftDioptre = cellCol(18).toString();
        patientCase.leftWithGlass = cellCol(19).toString();
        patientCase.leftPressure = cellCol(20).toString();
        QStringList temp = cellCol(11).toString().split("\n", Qt::SkipEmptyParts);
        for (int i = 0; i < temp.count(); i++) {
            patientCase.drugsList.append(temp[i].split("::"));
        }
    }
}

QVariant PrescriptorMainWin::cellCol(int column) {
    return patientInfoTableModel->data(patientInfoTableModel->index(searchCurrentRow, column));
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
    patientDb.removePatient(cellCol(0).toString());
    patientInfoTableModel->setQuery(patientDb.getQuery());
}

bool PrescriptorMainWin::savePre() {
    if (ui->pre_nameInput->text() == "") {
        operationError("Tên bệnh nhân không được để trống!");
        return false;
    }

    if (patientCase.id != 0 && ui->pre_nameInput->text() == patientCase.name)
        patientDb.removePatient(QString::number(patientCase.id));

    int tempID = patientDb.getNextID();
    if (tempID == -1) {
        operationError("Không thêm được đơn thuốc vào CSDL [ID]");
        return false;
    }

    patientCase.id = tempID;
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
        operationError("Không thêm được đơn thuốc vào CSDL! Chi tiết lỗi: \n" + patientDb.query->lastError().text());
        ui->pre_stateLabel->setText("Không lưu được thông tin bệnh nhân!");
        return false;
    }

    ui->pre_stateLabel->setText("Đã lưu thành công!");
    patientInfoTableModel->setQuery(patientDb.getQuery());
    return true;
}

void PrescriptorMainWin::printPatientInfo() {
    if (ui->tabWidget->currentIndex() == 0) {
        if (!savePre()) return;
    }
    else if (ui->tabWidget->currentIndex() == 1 && searchCurrentRow != -1) {
        copyPatientInfo(true);
    }
    ui->tabWidget->setCurrentIndex(3);
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
    QList<QLineEdit *> lineInput = ui->tabWidget[0].findChildren<QLineEdit *>();
    for (int i = 0; i < int(lineInput.count()); i++) {
        lineInput[i]->clear();
    }

    QList<QTextEdit *> listInput = ui->tabWidget[0].findChildren<QTextEdit *>();
    for (int i = 0; i < int(listInput.count()); i++) {
        listInput[i]->clear();
    }

    ui->pre_revisitCheckbox->setChecked(false);
    ui->pre_revisitperiodInput->setEnabled(false);
    ui->pre_drugsView->model()->removeRows(0, ui->pre_drugsView->model()->rowCount());
    ui->pre_YearsOld->clear();

    patientCase.clear();
}

void PrescriptorMainWin::backupData() {
    QString backupDir = bacAndRecDialog.getExistingDirectory(this, "Sao lưu dữ liệu", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (backupDir == "") return;
    backupDir += "/Backup-" + QDateTime::currentDateTime().toString("dd-MM-yyyy-hh-mm-ss");

    bool success = patientDb.backupData(backupDir);

    if (!success) {
        ui->prop_currentState->clear();
        operationError("Đã có lỗi xảy ra trong quá trình sao lưu!");
        return;
    }
        ui->prop_currentState->setText("Đã sao lưu thành công!");
}

void PrescriptorMainWin::restoreData() {
    QString restoreDir = bacAndRecDialog.getExistingDirectory(this, "Khôi phục dữ liệu", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (restoreDir == "") return;
    int code = patientDb.restoreData(restoreDir);

    if (code == PatientDb::CorruptedBackup) {
        ui->prop_currentState->clear();
        operationError("Bản sao lưu thiếu mẫu đơn (template.html) hoặc dữ liệu bệnh nhân (prescriptions.db)!");
        return;
    }
    else if (code == PatientDb::RWError) {
        ui->prop_currentState->clear();
        operationError("Có lỗi xảy ra trong quá trình sao chép bản sao lưu!");
        return;
    }

    ui->prop_currentState->setText("Đã khôi phục thành công!");
}

void PrescriptorMainWin::switchCell(QLineEdit *from, QLineEdit *to) {
    connect(from, &QLineEdit::returnPressed, to, [to]() {to->setFocus();});
}

void PrescriptorMainWin::enterBOD() {
    QStringList birth = ui->pre_birthInput->text().split('/');
    if (birth.count() == 3)
        ui->pre_YearsOld->setText("(" + QString::number(QDate::currentDate().year() - ui->pre_birthInput->text().split('/')[2].toInt()) +" tuổi)");
    else
        ui->pre_YearsOld->clear();
}
