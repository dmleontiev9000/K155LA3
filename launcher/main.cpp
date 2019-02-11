#include "../core/interfaces.h"
#include "core.h"
#include <stdio.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("k155la3");
    a.setOrganizationDomain("mygpu.ru");
    a.setOrganizationName("mygpu");
#if QT_VERSION >= 0x050000
    a.setApplicationDisplayName(a.translate("app", "K155LA3"));
#endif
    K::Launcher::Core core;
    if (core.loadPlugin("gui")) {
        core.runApp();
    } else {
        qCritical("failed to launch gui");
    }
    return 0;
}
