#ifndef PRINTER_H
#define PRINTER_H

#include <QtPrintSupport/QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <QStandardPaths>
#include <QDateTime>
#include <QRectF>
#include <QPdfView>
#include <QPdfDocument>

#include "patientcase.h"
#include "patientdb.h"

class Printer
{
public:
    Printer();

    void prepareTemplate(const QString &dirPath);
    void exportAndShowPrescription(const PatientCase &patientCase, const PatientDb &patientDb);

private:
    QString templateContent;
    QFile templateFile;
    QTextDocument prescriptionOutput;
    QPrinter prescriptionPrinter;
    QPdfView prescriptionViewer;
    QPdfDocument *prescriptionPdfDoc;
    QPageLayout pageLayout;

    QString addElement(QString element);
};

#endif // PRINTER_H
