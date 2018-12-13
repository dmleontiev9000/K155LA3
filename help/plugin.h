#pragma once

#include "../core/interfaces.h"
#include "../gui/conventions.h"
#include "conventions.h"
#include <QDir>
#include <QSplitter>
#include <QTreeWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSettings>
#include <QtHelp/QHelpEngine>
#include <QtHelp/QHelpContentWidget>
#include <QTextBrowser>

#ifndef HELP_LIBRARY
#error "this header is internal"
#endif

namespace K {
namespace Help {
namespace Internal {

class Plugin
    : public QObject
    , public K::IfPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "k.help")
    Q_INTERFACES(K::IfPlugin)
    Q_PROPERTY(QString path READ path WRITE setPath)
public:
    Plugin();
    ~Plugin();

    const QString& path() const;
    void setPath(const QString& s);

signals:
    K_GUI_ADD_MODE
    void addMode(QObject * object,
                 const QString& iconname,
                 const QString& title,
                 const QString& tooltip,
                 const QString& widgetid,
                 bool exclusive,
                 int order);

    K_GUI_REMOVE_MODE
    void removeMode(QObject* object);

    K_GUI_ACTIVATE_MODE
    void activateHelpMode(const QString& widgetId,
                          bool exclusive);

    K_GUI_ADD_WIDGET
    void addWidget(QScopedPointer<QWidget>* widget,
                   QScopedPointer<QWidget>* toolbar,
                   const QString& widgetid,
                   int position);

    K_GUI_REMOVE_WIDGET
    void removeWidget(const QString& widgetid);

    K_GUI_HIDE_MODE
    void escPressed(const QString& widgetid);

public slots:
    K_HELP_OPEN_PAGE
    void openPage(const QString &topic);
private slots:
    void titleChanged(const QString& title);
    void widgetRemoved();

    void searchOnPage();
    void home();
    void back();
    void forward();
    void topicClicked(const QUrl& url);
    void historyClicked(int index);
    void bookmarkClicked(QTreeWidgetItem *);
private:
    K_DECLARE_PLUGIN_METHODS

    QHelpEngine * mEngine = nullptr;

    void sync();
    void openUrl(const QUrl& url, const QString& title, bool keep);

    bool          mLocked = false;
    QString       mErrorString;
    QString       mLocation;
    QSettings     mSettings;
    bool          mGuiExists = false;
    QDir          mHelpLocation;

    QHelpContentWidget * mTopicSelector;
    QTreeWidget  *mBookmarkSelector;
    QHash<QString, QTreeWidgetItem*> mTopicSet;

    QTextBrowser *mWebPage;
    QLineEdit    *mSearchLine;
    QComboBox    *mHistoryBox;

    QStringList   mTitleList;
    QList<QUrl>   mUrlList;
    int           mListPos = 0;
};

} //namespace Internal
} //namespace Help
} //namespace K
