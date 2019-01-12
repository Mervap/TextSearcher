#ifndef TRIGRAMCONTAINER_H
#define TRIGRAMCONTAINER_H

#include <QString>
#include <QVector>


class TrigramContainer {

public:
    TrigramContainer() : filename(""), trigrams(QVector<uint32_t>()) {}
    TrigramContainer(QString filename) : filename(filename), trigrams(QVector<uint32_t>()) {}

    void insert(uint32_t);
    bool contains(uint32_t) const;
    int size() const;
    void clear() const;
    bool isEmpty() const;
    QString getFilename();

private:
    QString filename;
    QVector<uint32_t> trigrams;
};

#endif // TRIGRAMCONTAINER_H
