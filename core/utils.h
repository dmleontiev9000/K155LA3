#pragma once

#include "core_global.h"
#include <QString>
#include <QObject>
#include <QSharedPointer>

#define DLL_IMPORT(retval, name, ...) \
    typedef retval (*name ## _t) (__VA_ARGS__); \
    const name ## _t name = (name ## _t) resolve(#name)
#define DLL_IMPORT_F(func, retval, name, ...) \
    typedef retval (*name ## _t) (__VA_ARGS__); \
    const name ## _t name = (name ## _t) func(#name)
#define DLL_IMPORT_T(T, retval, name, ...) \
    typedef T retval (*name ## _t) (__VA_ARGS__); \
    const name ## _t name = (name ## _t) resolve(#name)
#define DLL_IMPORT_FT(func, T, retval, name, ...) \
    typedef T retval (*name ## _t) (__VA_ARGS__); \
    const name ## _t name = (name ## _t) func(#name)

namespace K {

/*
 * utility function
 */
QString K_CORE_EXPORT objectPath(QObject * obj);
bool K_CORE_EXPORT connectSignalToTaggedSlot(QObject * obj, const char * signal,
                                             QObject * receiver, const char * method);
bool K_CORE_EXPORT canAutoConnect(QObject * obj, const char * const *signals_, const char * const * slots_);
bool K_CORE_EXPORT canAutoConnect(QObject * src, QObject * dst, const char * prefix = nullptr);
void K_CORE_EXPORT autoConnectByTags(QObject * src, QObject * dst, Qt::ConnectionType type, const char * prefix = nullptr);
bool K_CORE_EXPORT autoConnectByTagsBidirectional(QObject * src, QObject * dst, Qt::ConnectionType type, const char * prefix = nullptr);

class K_CORE_EXPORT AutoDeleter : public QObject {
    Q_OBJECT
public:
    AutoDeleter(QObject * parent);
    AutoDeleter(QObject * watchOnObject, QObject * parent);
    void add(QObject * obj);
    ~AutoDeleter();
private slots:
    void objectDeleted(QObject * obj);
private:
    QObjectList mObjects;
};

} //namespace K;

