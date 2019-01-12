#include "testfilesearcher.h"
#include <QtTest/QtTest>
#include "../Searcher/filesearcher.h"
#include <QFile>
#include <QDir>
#include <iostream>

TestFileSearcher::TestFileSearcher(QObject *parent) : QObject(parent) {
}

void TestFileSearcher::testHelper(const QString &input, const QString &fileText, bool ok) {
    FileSearcher fs(input.toStdString());
    QFile file(QDir::currentPath() + "test.txt");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(fileText.toStdString().c_str());
        file.close();

        fs.addFile(file.fileName());
        QSignalSpy spy(&fs, SIGNAL(updateFileList(QString)));
        fs.search();

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.at(0).at(0), ok ? file.fileName() : "");
    } else {
        std::cerr << "Can't create file for test\n";
    }

    file.remove();
}

void TestFileSearcher::emptyFile() {
    testHelper("aaa", "", false);
}

void TestFileSearcher::LatinText() {
    testHelper("aaa", "aaa", true);  //In - en, File - en
    testHelper("aaa", "ааа", false); //In - en, File - ru
}
void TestFileSearcher::RussiaText() {
    testHelper("ааа", "ааа", true);  //In - ru, File - ru
    testHelper("ааа", "aaa", false); //In - ru, File - en
}

void TestFileSearcher::randomText() {
    QString fileText;
    for (int i = 0; i < 1000; ++i) {
        fileText += static_cast<char>(std::rand() % 26 + 'a');
    }

    testHelper("I want find this", fileText, false);

    fileText += "I want find this";

    for (int i = 0; i < 1000; ++i) {
        fileText += static_cast<char>(std::rand() % 26 + 'a');
    }

    testHelper("I want find this", fileText, true);
}

void TestFileSearcher::BigText() {
    QString fileText;
    for (int i = 0; i < (1 << 14) - 1; ++i) {
        fileText += "a";
    }

    fileText += "bbb";

    for (int i = 0; i < 1000; ++i) {
        fileText += "a";
    }

    testHelper("bbb", fileText, true);
}
