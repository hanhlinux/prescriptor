#include "printer.h"

Printer::Printer() {}

bool Printer::prepareTemplate(const QString &dirPath, QWidget *prev, float fontSize) {
    QTextStream input;
    QStringList temp;
    QFont font;

    preview = prev;
    font.setFamily("Calibri");
    font.setPixelSize(fontSize);
    templateFile.setFileName(dirPath + "/template.html");

    if (!templateFile.exists() || !templateFile.open(QFile::ReadWrite)) {
        return false;
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
    ps.setHeight(prescriptionPrinter.width() / 0.73);
    prescriptionOutput.setPageSize(ps);
    prescriptionPrinter.setPageLayout(pageLayout);
    prescriptionPrinter.setResolution(300);
    prescriptionPrinter.setOutputFormat(QPrinter::PdfFormat);

    return true;
}

QString Printer::loadPatientInfo(const PatientCase &patientCase, const PatientDb &patientDb) {
    QString result = patientCase.result;
    QString diagnosis = patientCase.diagnosis;
    QStringList drugs;
    QStringList prescribeDMY = patientCase.prescribeDate.split(" ")[0].split("-");
    QString allNotes;
    QString prescriptionContent = templateContent;

    result.replace("\n", "<br>");
    diagnosis.replace("\n", "<br>");

    prescriptionContent.replace("$SHOPNAME$", patientDb.shopName);
    prescriptionContent.replace("$SHOPADDRESS$", patientDb.shopAddress);
    prescriptionContent.replace("$SHOPPHONE$", patientDb.shopPhoneNum);
    prescriptionContent.replace("$DOCTORNAME$", patientDb.doctorName);

    prescriptionContent.replace("$PATIENTNAME$", patientCase.name);
    prescriptionContent.replace("$PATIENTBIRTH$", patientCase.dateOfBirth);
    prescriptionContent.replace("$PATIENTADDRESS$", patientCase.address);
    prescriptionContent.replace("$PATIENTPHONENUMBER$", patientCase.phoneNumber);
    prescriptionContent.replace("$PATIENTRESULT$", result);
    prescriptionContent.replace("$PATIENTDIAGNOSIS$", diagnosis);
    prescriptionContent.replace("$PATIENTPRESCRIBETIME$", patientCase.prescribeDate);
    prescriptionContent.replace("$PATIENTNOTE$", patientCase.note);
    prescriptionContent.replace("$PD$", prescribeDMY[0]);
    prescriptionContent.replace("$PM$", prescribeDMY[1]);
    prescriptionContent.replace("$PY$", prescribeDMY[2]);

    prescriptionContent.replace("$RNG$", patientCase.rightNoGlass);
    prescriptionContent.replace("$RD$", patientCase.rightDioptre);
    prescriptionContent.replace("$RWG$", patientCase.rightWithGlass);
    prescriptionContent.replace("$RP$", patientCase.rightPressure);
    prescriptionContent.replace("$LNG$", patientCase.leftNoGlass);
    prescriptionContent.replace("$LD$", patientCase.leftDioptre);
    prescriptionContent.replace("$LWG$", patientCase.leftWithGlass);
    prescriptionContent.replace("$LP$", patientCase.leftPressure);

    if (patientCase.note.length() > 0) {
        allNotes += "<li>" + patientCase.note + "</li>\n";
    }
    if (patientCase.gender == PatientCase::Male) {
        prescriptionContent.replace("$PATIENTGENDER$", "Nam");
    }
    else {
        prescriptionContent.replace("$PATIENTGENDER$", "Nữ");
    }

    if (patientCase.revisitPeriod != "") {
        prescriptionContent.replace("$PATIENTREVISIT$", "Hẹn tái khám bệnh nhân sau " + patientCase.revisitPeriod);
        allNotes += "<li>Hẹn tái khám bệnh nhân sau " + patientCase.revisitPeriod + "</li>\n";
    }
    else {
        prescriptionContent.replace("$PATIENTREVISIT$", "");
    }

    if (patientCase.warnNoPhone == true) {
        prescriptionContent.replace("$PATIENTWARNNOPHONE$", "Hạn chế tiếp xúc với các thiết bị điện tử như TV, iPad (tablet), điện thoại thông minh,...");
        allNotes += "<li>Hạn chế tiếp xúc với các thiết bị điện tử như TV, iPad (tablet), điện thoại thông minh,...</li>\n";
    }
    else {
        prescriptionContent.replace("$PATIENTWARNNOPHONE$", "");
    }

    for (int i = 0; i < patientCase.drugsList.count(); i++) {
        drugs << "<li>" + addElement(patientCase.drugsList[i][0])
        + addElement(patientCase.drugsList[i][1])
            + addElement(patientCase.drugsList[i][2] + ".")
            + "</li>";
    }

    prescriptionContent.replace("$PATIENTDRUGS$", drugs.join("\n"));
    prescriptionContent.replace("$PATIENTALLNOTES$", allNotes);
    return prescriptionContent;
}


void Printer::exportAndShowPrescription(const PatientCase &patientCase, const PatientDb &patientDb) {
    if (patientCase.prescribeDate.length() == 0) return;

    prescriptionOutput.setHtml(loadPatientInfo(patientCase, patientDb));

    prescriptionPrinter.setOutputFileName(
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/"
         + "temp.pdf");
    prescriptionPrinter.outputFileName();
    prescriptionOutput.print(&prescriptionPrinter);
    prescriptionPdfDoc->load(prescriptionPrinter.outputFileName());
    prescriptionViewer.setDocument(prescriptionPdfDoc);
    prescriptionViewer.setZoomMode(QPdfView::ZoomMode::FitInView);
    prescriptionPrinter.setOutputFileName(0);
    preview->layout()->addWidget(&prescriptionViewer);

    QPrintDialog printDiag(&prescriptionPrinter);

    if (printDiag.exec() == QDialog::Accepted) {
        prescriptionOutput.print(&prescriptionPrinter);
    }
}

QString Printer::addElement(QString element) {
    if (element.back() == '.' || element == "") return element;
    else return element + ", ";
}
