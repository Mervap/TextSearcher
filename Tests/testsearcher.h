#ifndef TESTSEARCHER_H
#define TESTSEARCHER_H

#include <QObject>

class TestSearcher : public QObject {
    Q_OBJECT
public:
    explicit TestSearcher(QObject *parent = nullptr);

signals:

private slots:
    void noOne();
    void oneOfOne();
    void oneOfMany();
    void manyOfMany();
    void allOfMany();

private:
    void testHelper(const QString &, const QVector<QString> &, QSet<QString> &);
};

#endif // TESTSEARCHER_H
