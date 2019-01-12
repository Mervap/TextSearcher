#ifndef INDEX_H
#define INDEX_H

#include <QObject>
#include <QSet>
#include <QHash>
#include "TrigramContainer/trigramcontainer.h"

class Index {
public:
    Index() {}

    friend class TrigramCounter;
    friend class MainWindow;

private:
    QHash<QString, QVector<TrigramContainer>> files;
    QHash<QString, QVector<QString>> dirs;
    QSet<QString> allDirs;
};

#endif // INDEX_H
