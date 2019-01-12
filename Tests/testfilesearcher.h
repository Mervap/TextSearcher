#ifndef TESTFILESEARCHER_H
#define TESTFILESEARCHER_H

#include <QObject>

class TestFileSearcher : public QObject {
    Q_OBJECT
public:
    explicit TestFileSearcher(QObject *parent = nullptr);

signals:

private slots:
    void emptyFile();
    void LatinText();
    void RussiaText();
    void BigText();
    void randomText();

private:
    void testHelper(const QString &, const QString &, bool);
};

#endif // TESTFILESEARCHER_H
