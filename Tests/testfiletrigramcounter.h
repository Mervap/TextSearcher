#ifndef TESTFILETRIGRAMCOUNTER_H
#define TESTFILETRIGRAMCOUNTER_H

#include <QObject>

class TestFileTrigramCounter : public QObject {
    Q_OBJECT
public:
    explicit TestFileTrigramCounter(QObject *parent = nullptr);

signals:

private slots:
    void unic();
    void few();
    void many();
    void binary();
    void russian();

private:
    void testHelper(const QVector<QString> &, QVector<QVector<uint32_t>> &);
};

#endif // TESTFILETRIGRAMCOUNTER_H
