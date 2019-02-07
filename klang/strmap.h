#pragma once

#include "klang_global.h"
#include <QObject>
#include <QElapsedTimer>
#include <QList>
namespace K {
namespace Lang {

class Node1;
class String;
class StringMap : QObject {
    Q_OBJECT
private:
    enum {N_BUCKETS = 128};
public:
    struct Entry {
        qint64  age;
        Entry * next;
        Node1 * node;
        quint32 hash;
        quint32 length;
        quint16 text[0];
    };

    StringMap(QObject * parent = nullptr);
    ~StringMap();

    void addNode(Node1 * node, uint from, uint to);
    void removeNode(Node1 * node);

    Node1 *exactNameMatch(const String * str, uint from, uint to) const;
    QList<Node1*> guessNameMatch(const String * str, uint from, uint to) const;
private:
    void timerEvent(QTimerEvent *);
    Entry * map[N_BUCKETS];
    QElapsedTimer timer;
};

} //namespace Lang
} //namespace K
