#include "trigramcontainer.h"

void TrigramContainer::insert(uint32_t value) {
    trigrams.push_back(value);
}

bool TrigramContainer::contains(uint32_t value) {
    return trigrams.contains(value);
}

int TrigramContainer::size() {
    return trigrams.size();
}

void TrigramContainer::clear() {
    trigrams.clear();
}

QString TrigramContainer::getFilename() {
    return filename;
}

bool TrigramContainer::isEmpty() {
    return trigrams.isEmpty();
}

