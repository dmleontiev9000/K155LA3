#pragma once
#include <QXmlStreamReader>

namespace K {
namespace CE {
namespace OPDK {
namespace Import {

class Base {
public:
    bool match(QXmlStreamReader * reader) {
        return (reader->namespaceUri()==ns_ &&
                reader->name() == tag_);
    }
    static QLatin1String ops;
    static QLatin1String opc;
protected:
    Base(const QLatin1String& ns, const char * tag)
        : ns_(ns), tag_(tag) {}
    Base(const char * ns, const char * tag)
        : ns_(ns), tag_(tag) {}
    virtual bool params(QXmlStreamReader * reader,
                        QStringList * messageLog)
    {
        Q_UNUSED(reader);
        Q_UNUSED(messageLog)
        return true;
    }
    virtual Base * startElement(QXmlStreamReader * reader,
                                QStringList * messageLog)
    {
        Q_UNUSED(reader);
        Q_UNUSED(messageLog)
        return nullptr;
    }
    virtual bool           endElement(QXmlStreamReader * reader,
                                      QStringList * messageLog)
    {
        Q_UNUSED(reader);
        Q_UNUSED(messageLog);
        return true;
    }
    virtual bool text(QXmlStreamReader * reader,
                      QStringList * messageLog)
    {
        Q_UNUSED(reader);
        Q_UNUSED(messageLog);
        return true;
    }
    void log(QXmlStreamReader * reader,
             QStringList * messageLog,
             const QString& msg)
    {
        if (messageLog) {
            messageLog->append(QString::number(reader->lineNumber())+QChar(':')+msg);
        }
    }
    virtual bool end();
private:
    QLatin1String ns_, tag_;
};

} //namespace Import
} //namespace SI2
} //namespace CE
} //namespace K
