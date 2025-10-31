#include "printer.h"


Printer::Printer() {}

void Printer::prepareTemplate(const QString &dirPath) {
    QTextStream input;
    QStringList temp;
    QFont font;
    font.setFamily("Times New Roman");
    font.setPointSize(18);

    if (!QDir(dirPath).exists())
        QDir().mkpath(dirPath);

    templateFile.setFileName(dirPath + "/template.html");

    if (!templateFile.exists() || !templateFile.open(QFile::ReadWrite)) {
        QMessageBox::critical(nullptr, "Lỗi", "Thiếu file mẫu đơn của phần mềm HOẶC file mẫu đơn không thể mở được");
        exit(1);
    }

    input.setDevice(&templateFile);
    while (!input.atEnd()) temp << input.readLine();
    templateFile.close();

    templateContent = temp.join("\n");
    prescriptionPdfDoc = new QPdfDocument();

    prescriptionOutput.setDefaultFont(font);

    pageLayout.setPageSize(QPageSize::A5, QMarginsF(15, 15, 15, 15));
    pageLayout.setOrientation(QPageLayout::Portrait);
    pageLayout.setUnits(QPageLayout::Millimeter);
    QSizeF ps;
    ps.setWidth(prescriptionPrinter.width());
    ps.setHeight(prescriptionPrinter.width() / 0.75);
    prescriptionOutput.setPageSize(ps);
    prescriptionPrinter.setPageLayout(pageLayout);
    prescriptionPrinter.setResolution(300);
    prescriptionPrinter.setOutputFormat(QPrinter::PdfFormat);
}

void Printer::exportAndShowPrescription(const PatientCase &patientCase, const PatientDb &patientDb) {
    QStringList drugs;
    QString result = patientCase.result;
    QString diagnosis = patientCase.diagnosis;
    result.replace("\n", "<br>");
    diagnosis.replace("\n", "<br>");

    templateContent.replace("$SHOPNAME$", patientDb.shopName);
    templateContent.replace("$SHOPADDRESS$", patientDb.shopAddress);
    templateContent.replace("$SHOPPHONE$", patientDb.shopPhoneNum);
    templateContent.replace("$DOCTORNAME$", patientDb.doctorName);

    templateContent.replace("$PATIENTNAME$", patientCase.name);
    templateContent.replace("$PATIENTBIRTH$", patientCase.dateOfBirth);
    templateContent.replace("$PATIENTADDRESS$", patientCase.address);
    templateContent.replace("$PATIENTPHONENUMBER$", patientCase.phoneNumber);
    templateContent.replace("$PATIENTRESULT$", result);
    templateContent.replace("$PATIENTDIAGNOSIS$", diagnosis);
    templateContent.replace("$PATIENTPRESCRIBEDATE$", patientCase.prescribeDate);

    templateContent.replace("$RNG$", patientCase.rightNoGlass);
    templateContent.replace("$RD$", patientCase.rightDioptre);
    templateContent.replace("$RWG$", patientCase.rightWithGlass);
    templateContent.replace("$RP$", patientCase.rightPressure);
    templateContent.replace("$LNG$", patientCase.leftNoGlass);
    templateContent.replace("$LD$", patientCase.leftDioptre);
    templateContent.replace("$LWG$", patientCase.leftWithGlass);
    templateContent.replace("$LP$", patientCase.leftPressure);


    if (patientCase.gender == PatientCase::Male) {
        templateContent.replace("$PATIENTGENDER$", "Nam");
    }
    else {
        templateContent.replace("$PATIENTGENDER$", "Nữ");
    }

    if (patientCase.revisitPeriod != "") {
        templateContent.replace("$PATIENTREVISIT$", "Hẹn tái khám bệnh nhân sau " + patientCase.revisitPeriod);
    }
    else {
        templateContent.replace("$PATIENTREVISIT$", "");
    }

    if (patientCase.warnNoPhone == true) {
        templateContent.replace("$PATIENTWARNNOPHONE$", "Hạn chế tiếp xúc với các thiết bị điện tử như TV, iPad (tablet), điện thoại thông minh,...");
    }
    else {
        templateContent.replace("$PATIENTWARNNOPHONE$", "");
    }

    for (int i = 0; i < patientCase.drugsList.count(); i++) {
        drugs << "<li>" + addElement(patientCase.drugsList[i][0])
                    + addElement(patientCase.drugsList[i][1])
                    + addElement(patientCase.drugsList[i][2] + ".")
                    + "</li>";
    }

    templateContent.replace("$PATIENTDRUGS$", drugs.join("\n"));

    prescriptionOutput.setHtml(templateContent);

    prescriptionPrinter.setOutputFileName(
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/"
        + QDateTime::currentDateTime().toString("dd-MM-yyyy-HH-mm-ss") + ".pdf");
    prescriptionPrinter.outputFileName();
    prescriptionOutput.print(&prescriptionPrinter);
    prescriptionPdfDoc->load(prescriptionPrinter.outputFileName());
    prescriptionViewer.setDocument(prescriptionPdfDoc);
    prescriptionViewer.setZoomMode(QPdfView::ZoomMode::FitInView);
    prescriptionPrinter.setOutputFileName(0);
    prescriptionViewer.resize(800, 600);
    prescriptionViewer.show();
    prescriptionViewer.raise();

    QPrintDialog printDiag(&prescriptionPrinter);
    if (printDiag.exec() == QDialog::Accepted) {
        prescriptionOutput.print(&prescriptionPrinter);
    }
}

QString Printer::addElement(QString element) {
    if (element.back() == '.' || element == "") return element;
    else return element + ", ";
}
