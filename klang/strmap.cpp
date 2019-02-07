#include "strmap.h"
#include "node_p.h"

using namespace K::Lang;

StringMap::StringMap(QObject *parent)
    : QObject(parent)
{
    timer.start();
    for(int i = 0; i < N_BUCKETS; ++i)
        map[i] = nullptr;
    startTimer(5000);
}
StringMap::~StringMap() {
    /*
     * Уничтожение StringMap будет происходить в деструкторе
     * контекста, там же где будет уничтожен пул нод
     * поэтому вручную ничего чистить нет смысла
     */
    for(int i = 0; i < N_BUCKETS; ++i) {
        auto p = map[i];
        while(p) {
            auto n = p->next;
            delete p;
            p = n;
        }
    }
}
void StringMap::addNode(Node1 *node, uint from, uint to) {
    if (node->mSMEntry) removeNode(node);
    if (!node->mText) return;

    quint32 hash = node->mText->hash(from, to);
    auto pi = &map[hash%N_BUCKETS];
    auto ni = map[hash%N_BUCKETS];
    while(ni) {
        if (ni->hash != hash || ni->length != to - from)
        {
            pi = &ni->next;
            ni = ni->next;
            continue;
        }
        uint k = 0;
        for(; k < ni->length; ++k) {
            auto s = S::sym(node->mText->at(k+from));
            if (s != ni->text[k])
                break;
        }
        if (k != ni->length) {
            pi = &ni->next;
            ni = ni->next;
            continue;
        }

        if (ni->node) ni->node->mNLPrev = &node->mNLNext;
        node->mNLNext = ni->node;
        node->mNLPrev = &ni->node;
        node->mSMEntry = ni;
        ni->node = node;
        return;
    }
    ni = (Entry*)malloc(sizeof(Entry)+2*(to-from));
    *pi = ni;
    ni->hash = hash;
    ni->length = to-from;
    ni->node = node;
    ni->age = 0;
    for(uint k = 0; k < ni->length; ++k)
        ni->text[k] = S::sym(node->mText->at(k+from));
    node->mNLNext = nullptr;
    node->mNLPrev = &ni->node;
    node->mSMEntry = ni;
}
void StringMap::removeNode(Node1 *node) {
    if (node->mSMEntry) {
        if (node->mNLNext) node->mNLNext->mNLPrev = node->mNLPrev;
        *node->mNLPrev = node->mNLNext;
        node->mNLPrev  = &node->mNLNext;
        if (node->mSMEntry->node == nullptr)
            node->mSMEntry->age = timer.elapsed();
        node->mNLNext  = nullptr;
        node->mSMEntry = nullptr;
    }
}
void StringMap::timerEvent(QTimerEvent*) {
    qint64 t = timer.elapsed() - 1000;
    for(int i = 0; i < N_BUCKETS; ++i) {
        auto p = &map[i];
        auto n = map[i];
        while(n) {
            auto q = n;
            if (!n->node && n->age < t) {
                *p = n->next;
                n  = n->next;
                delete q;
            } else {
                n  = n->next;
                p  = &n->next;
            }
        }
    }
}

Node1 * StringMap::exactNameMatch(const String *str, uint from, uint to) const {
    Q_ASSERT(from < to);
    Q_ASSERT(to < str->length());

    uint hash = str->hash(from, to);
    for(auto ni = map[hash%N_BUCKETS]; ni; ni = ni->next)
    {
        if (!ni->node)
            continue;
        if (ni->hash != hash || ni->length != to - from)
            continue;
        uint k = 0;
        for(; k < ni->length; ++k) {
            auto s = S::sym(str->at(k+from));
            if (s != ni->text[k])
                break;
        }
        if (k != ni->length) continue;
        return ni->node;
    }
    return nullptr;
}
QList<Node1 *> StringMap::guessNameMatch(const String *str, uint from, uint to) const {
    Q_ASSERT(from < to);
    Q_ASSERT(to < str->length());
    QList<Node1 *> out;

    uint hash = str->at(from) & S::HKEY_MASK;
    for(auto ni   = map[hash%N_BUCKETS]; ni; ni = ni->next)
    {
        if (!ni->node)
            continue;
        if (ni->length < to - from)
            continue;

        uint i = from;
        uint j = 0;
        uint ec = 0;
        while(i != to && j < ni->length)
        {
            auto s1 = S::sym(str->at(i));
            auto s2 = ni->text[i];
            if (s1 == s2) {
                ++i; ++j;
                continue;
            }
            if (s1 == '_') {
                ++i;
                continue;
            }
            if (s2 == '_') {
                ++j;
                continue;
            }
            if (QChar(s1).toLower() != QChar(s2).toLower())
                ++ec;
            ++i; ++j;
        }
        if (i != to)
            continue;
        if (((to-from)>>2) < ec)
            continue;
        out.append(ni->node);
    }
    return out;
}
