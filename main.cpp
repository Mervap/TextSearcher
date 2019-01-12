#include "mainwindow.h"
#include <QApplication>
#include <QtTest/QtTest>
#include "Tests/testfilesearcher.h"
#include "Tests/testfiletrigramcounter.h"
#include "Tests/testsearcher.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QTest::qExec(new TestFileSearcher, argc, argv);
    QTest::qExec(new TestFileTrigramCounter, argc, argv);
    QTest::qExec(new TestSearcher, argc, argv);

    return a.exec();
}
