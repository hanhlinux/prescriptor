#ifndef PRINTER_H
#define PRINTER_H

#include <QtPrintSupport/QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextEdit>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <QStandardPaths>
#include <QDateTime>
#include <QRectF>
#include <QPdfView>
#include <QPdfDocument>
#include <QWidget>
#include <QLayout>
#include <QObject>

#include "patientcase.h"
#include "patientdb.h"

class Printer: public QObject
{
public:
    Printer();

    bool prepareTemplate(const QString &dirPath, QWidget *prev, float fontSize);
    void processTemplate();
    void exportAndShowPrescription(const PatientCase &patientCase, const PatientDb &patientDb);

private:
    QString templateContent;
    QFile templateFile;
    QTextDocument prescriptionOutput;
    QPrinter prescriptionPrinter;
    QPdfView prescriptionViewer;
    QPdfDocument *prescriptionPdfDoc;
    QPageLayout pageLayout;
    QWidget *preview;

    QString loadPatientInfo(const PatientCase &patientCase, const PatientDb &patientDb);
    QString addElement(QString element);
};

#endif // PRINTER_H
