#include "trigramcontainer.h"

void TrigramContainer::insert(uint32_t value) {
    trigrams.push_back(value);
}

bool TrigramContainer::contains(uint32_t value) const {
    return trigrams.contains(value);
}

int TrigramContainer::size() const {
    return trigrams.size();
}

void TrigramContainer::clear() {
    trigrams.clear();
}

QString TrigramContainer::getFilename() {
    return filename;
}

bool TrigramContainer::isEmpty() const {
    return trigrams.isEmpty();
}

