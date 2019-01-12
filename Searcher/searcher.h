#ifndef SEARCHER_H
#define SEARCHER_H

#include <QObject>

class Searcher : public QObject
{
    Q_OBJECT
public:
    explicit Searcher(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SEARCHER_H