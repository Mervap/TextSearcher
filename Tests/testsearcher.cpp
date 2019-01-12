#include "testsearcher.h"
#include <QtTest/QtTest>
#include <QVector>
#include "../Searcher/searcher.h"
#include "../TrigramCounter/filetrigramcounter.h"
#include <QDir>
#include <iostream>

TestSearcher::TestSearcher(QObject *parent) : QObject(parent) {

}

Q_DECLARE_METATYPE(TrigramContainer);

void TestSearcher::testHelper(const QString &input, const QVector<QString> &fileTexts, QSet<QString> &ans) {
    Index index;
    FileTrigramCounter ftc;

    QVector<QString> files;
    for (int i = 0; i < fileTexts.size(); ++i) {
        QString s = QDir::currentPath() + "test" + QString::number(i) + ".txt";
        files.push_back(s);

        QFile file(files[i]);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(fileTexts[i].toStdString().c_str());
            file.close();

            ftc.addFile(file.fileName());
        } else {
            std::cerr << "Can't create file for test\n";
        }
    }

    qRegisterMetaType<QVector<TrigramContainer>>("QVector<TrigramContainer>");
    QSignalSpy spy(&ftc, SIGNAL(updateIndex(QVector<TrigramContainer>)));
    ftc.countTrigrams();

    index.allDirs.insert(QDir::currentPath());
    for (int i = 0; i < spy.size(); ++i) {
        auto result = spy.at(i).at(0).value<QVector<TrigramContainer>>();
        for (int i = 0; i < result.size(); ++i) {
            index.files[QDir::currentPath()].push_back(result[i]);
        }
    }

    Searcher searcher(input, &index);
    qRegisterMetaType<QVector<QPair<QString, QString>>>("QVector<QPair<QString, QString>>");
    QSignalSpy spy1(&searcher, SIGNAL(updateFileList(QVector<QPair<QString, QString>>)));
    QSignalSpy spy2(&searcher, SIGNAL(searchFinish()));
    searcher.find();

    QTest::qWait(100);

    for (int i = 0; i < files.size(); ++i) {
        QFile(files[i]).remove();
    }

    int cnt = 0;
    for (int i = 0; i < spy1.size(); ++i) {
        auto res = spy1.at(i).at(0).value<QVector<QPair<QString, QString>>>();
        for (int j = 0; j < res.size(); ++j) {
            QVERIFY(ans.contains(res[j].second));
            ++cnt;
        }
    }

    QCOMPARE(cnt, ans.size());

}

void TestSearcher::noOne() {
    QVector<QString> fileTexts;
    fileTexts.push_back("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    fileTexts.push_back("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    fileTexts.push_back("ccccccccccccccccccccccccccccccccccccccccccccccc");
    fileTexts.push_back("ddddddddddddddddddddddddddddddddddddddddddddddd");

    QSet<QString> ans;
    testHelper("eeee", fileTexts, ans);
}

void TestSearcher::oneOfOne() {
    QVector<QString> fileTexts;
    fileTexts.push_back("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    QSet<QString> ans;
    ans.insert(QDir::currentPath() + "test0.txt");
    testHelper("aaaa", fileTexts, ans);
}

void TestSearcher::oneOfMany() {
    QVector<QString> fileTexts;
    fileTexts.push_back("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    fileTexts.push_back("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    fileTexts.push_back("ccccccccccccccccccccccccccccccccccccccccccccccc");
    fileTexts.push_back("ddddddddddddddddddddddddddddddddddddddddddddddd");

    QSet<QString> ans;
    ans.insert(QDir::currentPath() + "test0.txt");
    testHelper("aaaa", fileTexts, ans);
}

void TestSearcher::manyOfMany() {
    QVector<QString> fileTexts;
    fileTexts.push_back("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    fileTexts.push_back("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    fileTexts.push_back("ccccccccccaaaaccccccccccccccccccccccccccccccccc");
    fileTexts.push_back("ddddddddddddddddddddddddddddddddddddddddddddddd");

    QSet<QString> ans;
    ans.insert(QDir::currentPath() + "test0.txt");
    ans.insert(QDir::currentPath() + "test2.txt");
    testHelper("aaaa", fileTexts, ans);
}

void TestSearcher::allOfMany() {
    QVector<QString> fileTexts;
    fileTexts.push_back("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    fileTexts.push_back("bbbbbbbbbbbbbbbbbaaaabbbbbbbbbbbbbbbbbbbbbbbbbb");
    fileTexts.push_back("cccccccaaaacccccccccccccccccccccccccccccccccccc");
    fileTexts.push_back("dddddddddddddddddddddddddddddaaaadddddddddddddd");

    QSet<QString> ans;
    ans.insert(QDir::currentPath() + "test0.txt");
    ans.insert(QDir::currentPath() + "test1.txt");
    ans.insert(QDir::currentPath() + "test2.txt");
    ans.insert(QDir::currentPath() + "test3.txt");
    testHelper("aaaa", fileTexts, ans);
}
