#include "testfiletrigramcounter.h"
#include <QtTest/QtTest>
#include "TrigramCounter/filetrigramcounter.h"
#include "TrigramContainer/trigramcontainer.h"
#include <QFile>
#include <QDir>
#include <iostream>

TestFileTrigramCounter::TestFileTrigramCounter(QObject *parent) : QObject(parent) {

}

Q_DECLARE_METATYPE(TrigramContainer);

void TestFileTrigramCounter::testHelper(const QVector<QString> &fileTexts, QVector<QVector<uint32_t>> &answer) {
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

    for (int i = 0; i < files.size(); ++i) {
        QFile(files[i]).remove();
    }

    QCOMPARE(spy.size(), 1);
    auto result = spy.at(0).at(0).value<QVector<TrigramContainer>>();
    QCOMPARE(result.size(), answer.size());
    for (int i = 0; i < result.size(); ++i) {
        QCOMPARE(result[i].size(), answer[i].size());
        for (auto e : answer[i]) {
            QVERIFY(result[i].contains(e));
        }
    }
}

void makeTrigrams(QString &text, QVector<uint32_t> &ans) {
    std::string s = text.toStdString();
    std::vector<unsigned char> inp(s.length());
    std::copy(s.begin(), s.end(), inp.begin());

    QSet<uint32_t> trigrams;
    for (size_t i = 0; i < inp.size() - 2; ++i) {
        uint32_t hash = 0;
        for (size_t j = 0; j < 3; ++j) {
            hash <<= 8;
            hash += inp[i + j];
        }
        trigrams.insert(hash);
    }

    if (trigrams.size() > 20000) {
        return;
    }

    for (auto e : trigrams) {
        ans.push_back(e);
    }
}

void TestFileTrigramCounter::unic() {
    QVector<QVector<uint32_t>> trigrams(1);
    QVector<QString> texts;
    texts.push_back("It's some text for testing trigrams");

    makeTrigrams(texts[0], trigrams[0]);

    testHelper(texts, trigrams);
}

void TestFileTrigramCounter::few() {
    QVector<QVector<uint32_t>> trigrams(10);
    QVector<QString> texts;

    for (int i = 0; i < 10; ++i) {
        QString fileText;
        for (int i = 0; i < rand() % 100 + 30; ++i) {
            fileText += static_cast<char>(std::rand() % 26 + 'a');
        }

        texts.push_back(fileText);
        makeTrigrams(texts[i], trigrams[i]);
    }


    testHelper(texts, trigrams);
}

void TestFileTrigramCounter::many() {
    QVector<QVector<uint32_t>> trigrams(100);
    QVector<QString> texts;

    for (int i = 0; i < 100; ++i) {
        QString fileText;
        for (int i = 0; i < rand() % 10000 + 30; ++i) {
            fileText += static_cast<char>(std::rand() % 26 + 'a');
        }

        texts.push_back(fileText);
        makeTrigrams(texts[i], trigrams[i]);
    }


    testHelper(texts, trigrams);
}

void TestFileTrigramCounter::binary() {
    QVector<QVector<uint32_t>> trigrams;
    QVector<QString> texts;

    QString fileText;
    for (int i = 0; i < 100000; ++i) {
        fileText += static_cast<char>(std::rand() % 100 + 20);
    }

    testHelper(texts, trigrams);
}

void TestFileTrigramCounter::russian() {

}

