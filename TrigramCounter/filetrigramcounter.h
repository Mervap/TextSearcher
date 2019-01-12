#ifndef FILETRIGRAMCOUNTER_H
#define FILETRIGRAMCOUNTER_H

#include <QObject>
#include <QVector>
#include "../TrigramContainer/trigramcontainer.h"

class FileTrigramCounter : public QObject {
    Q_OBJECT
public:
    explicit FileTrigramCounter() : filenames(QVector<QString>()) {}

    void addFile(QString filename);

public slots:
    void countTrigrams();

signals:
    void message(QString);
    void updateProgress();
    void updateIndex(QVector<TrigramContainer>);

private:
    void countFileTrigrams(QString filename, TrigramContainer &container);
    void InterruptionRequest();

    QVector<QString> filenames;

    static const int BUFFER_SIZE = 1 << 14;
    static const int TRIGRAM_SIZE = 3;

};

#endif // FILETRIGRAMCOUNTER_H
