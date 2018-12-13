#pragma once

#include "../core/interfaces.h"
#include "../gui/conventions.h"
#include "../documents/openedmodel.h"
#include "../documents/bookmarkmodel.h"
#include "../documents/editor.h"
#include "conventions.h"
#include "interfaces.h"
#include "projectmodel.h"
#include <QStackedWidget>
#include <QObject>
#include <QSettings>
#include <QSplitter>
#include <QToolButton>
#include <QTreeView>
#include <QListView>

#ifndef IDE_LIBRARY
#error "this header is internal"
#endif

namespace K {
namespace IDE {

class Plugin
        : public QObject
        , public K::IfPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "k.ide")
    Q_INTERFACES(K::IfPlugin)
public:
    Plugin();
    ~Plugin();
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

    K_GUI_ADD_WIDGET
    void addWidget(QScopedPointer<QWidget>* widget,
                   QScopedPointer<QWidget>* toolbar,
                   const QString& widgetid,
                   int position);

    K_GUI_REMOVE_WIDGET
    void removeWidget(const QString& widgetid);

public slots:
    K_IDE_OPEN_PROJECT
    void openProject(const QString& name,
                     const QString& path);

    K_IDE_ADD_PROJECT_DIALOG
    void addDialog(QScopedPointer<QWidget>* dialog,
                   const QString& unique_id,
                   const QString& name,
                   const QString& tooltip,
                   const QString& iconname);
    K_IDE_REMOVE_PROJECT_DIALOG
    void removeDialog(const QString& unique_id);

    K_IDE_ACTIVATE_PROJECT_DIALOG
    void activateDialog(const QString& path);

    K_IDE_CLOSE_PROJECT_DIALOG
    void closeDialog();

    K_IDE_REGISTER_EDITOR
    void registerEditor(QScopedPointer<Editor> *editor,
                        const QString& unique_id);

    K_IDE_UNREGISTER_EDITOR
    void unregisterEditor(const QString& unique_id);

    K_IDE_ADD_DOCUMENT
    void addDocument(Document * doc);

    K_IDE_DEL_DOCUMENT
    void delDocument(Document * doc);

    K_IDE_ACTIVATE_DOCUMENT
    void activateDocument(const QString& path);

private:
    K_DECLARE_PLUGIN_METHODS

    void sync();
    void setTitle();
    void activateDocumentAtLine(Document * doc, const QString* anchor, int line);

    QString               mErrorString;
    QSettings             mSettings;

    QWidget             * mProjectPanel  = nullptr;
    ProjectModel        * mProjectModel  = nullptr;
    ProjectModel        * mRecentModel   = nullptr;
    QWidget             * mStackW1       = nullptr;
    QStackedWidget      * mStackW2       = nullptr;
    QToolButton         * mTitleButton   = nullptr;
    QWidget             * mInDialog      = nullptr;

    QStackedWidget      * mEditorView    = nullptr;
    QStackedWidget      * mEditorToolbar = nullptr;
    QWidget             * mEditorStub    = nullptr;
    QWidget             * mEditorStubbar = nullptr;
    Document            * mCurrentDocument= nullptr;

    QSplitter           * mSideView      = nullptr;
    OpenedDocumentModel * mDocModel      = nullptr;
    BookmarkModel       * mBookModel     = nullptr;
    QTreeView           * mProjectView   = nullptr;
    QListView           * mBookmarkView  = nullptr;

    QHash<QString, QWidget*> mDialogs;
    QHash<QString, Editor*>  mEditors;
    QHash<QString, QWidget*> mToolbars;
};

} //namespace IDE
} //namespace K
