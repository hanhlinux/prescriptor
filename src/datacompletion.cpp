#include "datacompletion.h"

DataCompletion::DataCompletion() {}

DataCompletion::DataCompletion(QString dbDirPath, QString type, QObject *parent)
    : QObject{parent}
{
    setup(dbDirPath, type);
}

void DataCompletion::setup(QString dbDirPath, QString type) {
    QString fullPath = dbDirPath + "/" + type + ".txt";

    file.setFileName(fullPath);
    if (!file.open(QIODevice::ReadWrite)) {
        success = false;
        return;
    }

    stream.setDevice(&file);
    while (!stream.atEnd()) content << stream.readLine();
    file.close();

    success = true;
    model = new QStringListModel(content);
    completer = new QCompleter(model);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
}

void DataCompletion::save() {
    if (success == false || !file.open(QIODevice::WriteOnly))
        return;

    stream.setDevice(&file);
    for (int i = 0; i < int(content.count()); i++)
        stream << content[i] << "\n";
    file.close();
}

void DataCompletion::update(QString arg) {
    content << arg;
    content.sort();
    model->setStringList(content);
}
