#include "icons.h"
#include <QFileIconProvider>
#include <QHash>
#include <QFileInfo>
#include <QDebug>
static QHash<QString, QIcon> mIconSet;
static QString mIconPath(".");
/*****************************************
 * utility functions                     *
 *****************************************/
void K::Core::setIconPath(const QString &path) {
    mIconPath = path;
}
QIcon K::Core::getIcon(const char *name)
{
    return getIcon(QLatin1String(name));
}
QIcon K::Core::getIcon(const QString &name)
{
    auto i = mIconSet.find(name);
    if (i == mIconSet.end()) {
        static const QString fmt("%1/%2x%2/%3.png");
        QIcon newicon = QIcon::fromTheme(name);
        if (newicon.isNull()) {
            static const int sizes[]={16,24,32,48,64};
            for(uint i = 0; i < (sizeof(sizes)/sizeof(sizes[0])); ++i)
            {
                QString iconpath = fmt.arg(mIconPath).arg(sizes[i]).arg(name);
                if (QFile::exists(iconpath)) {
                    QPixmap pixmap(iconpath, "PNG");
                    newicon.addPixmap(pixmap);
                }
            }
        }
        i = mIconSet.insert(name, newicon);
        return i.value();
    }
    return i.value();
}
QIcon K::Core::getIconForFile(const QString &path, const char * def)
{
    static QFileIconProvider ipv;

    QFileInfo info(path);
    if (info.exists()) {
        QIcon icon = getIcon(info.completeSuffix());
        if (icon.isNull())
            icon = getIcon(def);
        return icon;
    }
    return getIcon(def);
}
