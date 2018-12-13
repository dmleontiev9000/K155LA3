#include "utils.h"
#include <string.h>
#include <QString>
#include <QMetaMethod>
#include <QMetaObject>
#include <QVector>
#include <QDebug>

QString K::objectPath(QObject * obj) {
    static QString null("<null>");
    static QString unnamed("<unnamed>");
    if (!obj) {
        return null;
    }
    QString str;
    if (obj->parent()) {
        str = objectPath(obj->parent());
        str.append(QChar('.'));
    }
    QString name = obj->objectName();
    if (name.isEmpty())
        name = unnamed;

    str.append(name);
    return str;
}

/*
 * у Qt в принципе есть функционал автоподключения сигналов,
 * но в стандартном QSignalMapperе это сделано с элементами
 * овцеёбства: нужно определенным образом назвать сигналы и
 * слоты, потом дать имена объектам, и всё это должно совпасть.
 * Я конечно понимаю, ОТКУДА это растёт: из необходимости
 * бесплатно без смс подключать гуи, нарисованые в дизайнере.
 *
 * Но нам надо подключать совсем не гуи, а плагины и плагины
 * могут друг о друге вообще ничего не знать, а следовательно
 * и сигналы-слоты обозвать не могут так, чтоб их подключил
 * маппер. Однако разработчики Qt сделали мега-вещь: тэги
 * на сигналах и слотах. Мы можем просто присобачить к сигналу
 * или слоту любую строку. Вот на этой фиче и основано действие
 * автоконнектора.
 */
#if QT_VERSION >= 0x050000
#define KMetaMethod QMetaMethod
#else
namespace {

class KMetaMethod : public QMetaMethod{
public:
    KMetaMethod(const QMetaMethod& meta) {
        QMetaMethod::operator =(meta);
        auto types = QMetaMethod::parameterTypes();
        mPcs.resize(types.size());
        for(int i = 0; i < types.size(); ++i) {
            mPcs.append(QMetaType::type(types[i].constData()));
        }
    }
    QByteArray methodSignature() const {
        return QByteArray::fromRawData(signature(), strlen(signature()));
    }
    int parameterCount() const {
        return mPcs.size();
    }
    int parameterType(int index) {
        return mPcs.at(index);
    }
private:
    QVector<int> mPcs;
};

}
#endif

bool K::connectSignalToTaggedSlot(QObject *obj, const char *signal, QObject *receiver, const char * method) {
    const QMetaObject * mo1 = obj->metaObject();
    int idx1 = mo1->indexOfSignal(signal);
    if (idx1 < 0)
        return false;
    QMetaMethod ms = mo1->method(idx1);

    const QMetaObject * mo2 = receiver->metaObject();
    int n = mo2->methodCount();
    for(int i = n-1; i >= 0; ++i) {
        QMetaMethod mm = mo2->method(i);
        if (!mm.tag())
            continue;
        if (strcmp(mm.tag(), method))
            continue;

        if (QObject::connect(obj, ms, receiver, mm))
            return true;
    }
    return false;
}

static bool strprefix(const char * str, const char * prefix) {
    for(;;) {
        char c1 = *str;
        char c2 = *prefix;

        if (!c2) {
            return true;
        }
        if (c1 == c2) {
            ++str;
            ++prefix;
            continue;
        }
        return false;
    }
}

bool K::canAutoConnect(QObject *obj, const char * const *signals_, const char * const *slots_) {
    quint64 sigmask  = 0;
    quint64 slotmask = 0;
    quint64 sigexp   = ~0ull;
    quint64 slotexp  = ~0ull;

    for(int j = 0; signals_[j]; ++j)
        sigexp <<= 1;
    sigexp = ~sigexp;
    for(int j = 0; slots_[j]; ++j)
        slotexp <<= 1;
    slotexp = ~slotexp;

    const QMetaObject * mo1 = obj->metaObject();
    int min_method = QObject::staticMetaObject.methodCount();
    int methods1 = mo1->methodCount();
    for(int i = min_method; i < methods1; ++i) {
        QMetaMethod m1 = mo1->method(i);
        if (!m1.tag())
            continue;

        if (signals_ && m1.methodType() == QMetaMethod::Signal) {
            for(int j = 0; signals_[j]; ++j) {
                if (!strcmp(m1.tag(), signals_[j])) {
                    sigmask |= (1ull<<j);
                    break;
                }
            }
        }
        if (slots_ && m1.methodType() == QMetaMethod::Slot) {
            for(int j = 0; slots_[j]; ++j) {
                if (!strcmp(m1.tag(), slots_[j])) {
                    slotmask |= (1ull<<j);
                    break;
                }
            }
        }
    }
    return (sigexp == sigmask) && (slotexp == slotmask);
}

bool K::canAutoConnect(QObject *src, QObject *dst, const char *prefix) {
    const QMetaObject * mo1 = src->metaObject();
    const QMetaObject * mo2 = dst->metaObject();
    int min_method = QObject::staticMetaObject.methodCount();
    int methods1 = mo1->methodCount();
    int methods2 = mo2->methodCount();
    for(int i = min_method; i < methods1; ++i) {
        KMetaMethod m1 = mo1->method(i);
        if (!m1.tag())
            continue;
        if (!m1.tag()[0])
            continue;
        if (prefix && !strprefix(m1.tag(), prefix))
            continue;
        if (m1.methodType() != QMetaMethod::Signal)
            continue;
        int pc = m1.parameterCount();

        bool has_match = false;
        for(int j = min_method; j < methods2; ++j) {
            KMetaMethod m2 = mo2->method(j);
            if (!m2.tag())
                continue;
            if (!m2.tag()[0])
                continue;
            if (m2.methodType() != QMetaMethod::Slot)
                continue;
            if (strcmp(m1.tag(), m2.tag()))
                continue;

            if (pc != m2.parameterCount())
                continue;

            bool match = true;
            for(int k = 0; k < pc; ++k) {
                if (m1.parameterType(k) !=
                    m2.parameterType(k)) {
                    match = false;
                    break;
                }
            }
            if (match) {
                has_match = true;
                break;
            }
        }
        if (!has_match)
            return false;
    }
    return true;
}

void K::autoConnectByTags(QObject *src, QObject *dst, Qt::ConnectionType type, const char * prefix) {
    type = Qt::ConnectionType(type|Qt::UniqueConnection);

    const QMetaObject * mo1 = src->metaObject();
    const QMetaObject * mo2 = dst->metaObject();
    int min_method = QObject::staticMetaObject.methodCount();
    int methods1 = mo1->methodCount();
    int methods2 = mo2->methodCount();

    for(int i = min_method; i < methods1; ++i) {
        KMetaMethod m1 = mo1->method(i);
        if (!m1.tag())
            continue;
        if (prefix && !strprefix(m1.tag(), prefix))
            continue;
        if (m1.methodType() != QMetaMethod::Signal)
            continue;
        int pc = m1.parameterCount();

        for(int j = min_method; j < methods2; ++j) {
            KMetaMethod m2 = mo2->method(j);
            if (!m2.tag())
                continue;
            if (m2.methodType() != QMetaMethod::Slot)
                continue;
            if (strcmp(m1.tag(), m2.tag()))
                continue;

            if (pc != m2.parameterCount())
                continue;

            bool match = true;
            for(int k = 0; k < pc; ++k) {
                if (m1.parameterType(k) !=
                    m2.parameterType(k)) {
                    match = false;
                    break;
                }
            }
            if (!match)
                continue;

            QObject::connect(src, m1, dst, m2, type);
        }
    }
}

bool K::autoConnectByTagsBidirectional(QObject *src, QObject *dst, Qt::ConnectionType type, const char *prefix) {
    if (!canAutoConnect(src, dst, prefix) ||
        !canAutoConnect(dst, src, prefix))
        return false;

    autoConnectByTags(src, dst, type, prefix);
    autoConnectByTags(dst, src, type, prefix);
    return true;
}

K::AutoDeleter::AutoDeleter(QObject *parent)
    : QObject(parent)
{
}
K::AutoDeleter::AutoDeleter(QObject *watchOnObject, QObject *parent)
    : QObject(parent)
{
    add(watchOnObject);
}
void K::AutoDeleter::add(QObject *obj) {
    Q_ASSERT(mObjects.contains(obj) == false);
    mObjects.append(obj);
    connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(objectDeleted(QObject*)));
}
void K::AutoDeleter::objectDeleted(QObject *obj) {
    mObjects.removeOne(obj);
}
K::AutoDeleter::~AutoDeleter()
{
    for(auto i = mObjects.constBegin(); i != mObjects.constEnd(); ++i)
        (*i)->deleteLater();
    mObjects.clear();
}

