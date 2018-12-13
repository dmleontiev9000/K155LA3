#include <plugin.h>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QListView>
#include <QPainter>
#include <style.h>
#include <icons.h>
#include <detaileditemdelegate.h>

using namespace K::IDE;
using K::Core::Style;

static const char TRCTX[] = {"ide"};

namespace {
    class EditorStub : public QWidget {
    public:
        EditorStub() {}
        void paintEvent(QPaintEvent *) {
            auto s = size();
            QPainter p;
            p.begin(this);

            p.fillRect(0,0,s.width(),s.height(),QColor(64,64,64));

            p.end();
        }
    };
}
Plugin::Plugin()
    : mSettings(QSettings::UserScope, "org.k155la3")
{
    mSettings.beginGroup("gui");
}
Plugin::~Plugin()
{
}
QObject * Plugin::self()
{
    return this;
}
bool Plugin::requiresGui() const
{
    return true;
}
QString Plugin::errorString()
{
    return mErrorString;
}
static const QString projectWidgetId("k.ide.projectmanager");
static const QString editorWidgetId("k.ide.editor");
bool Plugin::init()
{
    if (!loadPlugins({"gui"})) {
        mErrorString = tr("failed to load gui plugin", TRCTX);
        return false;
    }
    auto style = Style::instance();

    /*create project panel*/ {
        mProjectPanel = new QWidget;
        connect(mProjectPanel, &QObject::destroyed,
        [=] () { mProjectPanel = nullptr; });
        mProjectModel = new ProjectModel(-1, mProjectPanel);
        mRecentModel  = new ProjectModel(10, mProjectPanel);

        mProjectPanel->setPalette(QPalette(Qt::white));
        mProjectPanel->setAutoFillBackground(true);
        auto vbox = new QVBoxLayout(mProjectPanel);

        mStackW1 = new QWidget;
        vbox->addWidget(mStackW1);
        QGridLayout * grid    = new QGridLayout(mStackW1);
        QLabel      * lab1    = new QLabel();
        lab1->setTextFormat(Qt::RichText);
        lab1->setText(tr("<h2>Create new project</h2>", TRCTX));
        grid->addWidget(lab1, 0, 0, 1, 1, Qt::AlignCenter);
        QLabel      * lab2    = new QLabel();
        lab2->setTextFormat(Qt::RichText);
        lab2->setText(tr("<h2>Open existing project</h2>", TRCTX));
        grid->addWidget(lab2, 0, 1, 1, 1, Qt::AlignCenter);

        QListView   * list1 = new QListView();
        list1->setFrameShape(QFrame::NoFrame);
        list1->setItemDelegate(new Core::DetailedItemDelegate(list1));
        list1->setSelectionMode(QAbstractItemView::NoSelection);
        list1->setModel(mProjectModel);
        connect(list1, &QListView::clicked,
                [=] (const QModelIndex& index)
        {
            if (!mProjectPanel || !mSideView)
                return;

            const ProjectModel::Entry * e = mProjectModel->at(index);
            if (!e) return;

            mProjectModel->pokeElement(index.row());
            activateDialog(e->mUid);
        });
        grid->addWidget(list1, 1, 0, 1, 1);
        QListView   * list2 = new QListView();
        list2->setFrameShape(QFrame::NoFrame);
        list2->setItemDelegate(new Core::DetailedItemDelegate(list2));
        list2->setSelectionMode(QAbstractItemView::NoSelection);
        list2->setModel(mRecentModel);
        connect(list2, &QListView::clicked,
                [=](const QModelIndex& index)
        {
            if (!mProjectPanel)
                return;

            const ProjectModel::Entry * e = mRecentModel->at(index);
            if (!e)
                return;

            QString copy = e->mAux;
            mRecentModel->pokeElement(index.row());
            //use copy because we can be called from signal handler
            activateDocument(copy);
        });
        grid->addWidget(list2, 1, 1, 1, 1);
        grid->setRowStretch(1, 1);

        mStackW2 = new QStackedWidget;
        mStackW2->hide();
        vbox->addWidget(mStackW2,1);

        auto toolbar = new QWidget;
        QHBoxLayout * hbox = new QHBoxLayout(toolbar);
        hbox->setSpacing(0);
        hbox->setMargin(0);
        mTitleButton = new QToolButton();
        mTitleButton->setAutoRaise(false);
        mTitleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        K::Core::Style::instance()->addPanel(mTitleButton,
                                             K::Core::Style::DontShade);
        setTitle();
        hbox->addWidget(mTitleButton);
        connect(mTitleButton, &QToolButton::clicked, this, &Plugin::closeDialog);
        hbox->addStretch(1);

        int n = mSettings.beginReadArray("recent");
        n = qBound(0, n, 10);
        for(int i = 0; i < n; ++i) {
            mSettings.setArrayIndex(i);
            QString path = mSettings.value("path").toString();
            QString name = mSettings.value("name").toString();
            if (path.isEmpty() || name.isEmpty())
                continue;
            QIcon icon = K::Core::getIconForFile(path, "projects");
            mRecentModel->addElement(icon, name, path, path);
        }
        mSettings.endArray();

        QScopedPointer<QWidget> pWindow(mProjectPanel);
        QScopedPointer<QWidget> pToolbar(toolbar);
        emit addWidget(&pWindow, &pToolbar,
                       projectWidgetId,
                       GUI_POSITION_CENTER);
        emit addMode(this,
                     "project",
                     tr("Projects", TRCTX),
                     tr("Switch to \"Projects\" mode", TRCTX),
                     projectWidgetId, true, 0);
    }

    /*create editor sidebar*/ {
        mSideView = new QSplitter(Qt::Vertical);
        connect(mSideView, &QObject::destroyed,
        [=] () { mSideView = nullptr; });

        mDocModel     = new OpenedDocumentModel(mSideView);
        mBookModel    = new BookmarkModel(mSideView);

        mProjectView = new QTreeView;
        mProjectView->setModel(mDocModel);
        mProjectView->setHeaderHidden(true);
        mProjectView->setPalette(QPalette(Qt::white));
        mProjectView->setAutoFillBackground(true);
        mSideView->addWidget(mProjectView);

        QWidget * bookbar = new QWidget;
        style->addPanel(bookbar, K::Core::Style::Toolbar);
        QHBoxLayout * hbox= new QHBoxLayout(bookbar);
        hbox->addWidget(new QLabel(tr("Bookmarks", TRCTX)), 1);
        QToolButton * button = new QToolButton;
        button->setCheckable(true);
        QIcon down_icon = Core::getIcon("down");
        QIcon up_icon   = Core::getIcon("up");
        QIcon mix;
        mix.addPixmap(down_icon.pixmap(16,16), QIcon::Normal, QIcon::Off);
        mix.addPixmap(up_icon.pixmap(16,16), QIcon::Normal, QIcon::On);
        button->setIcon(mix);
        style->addPanel(button, Style::Flat);
        hbox->addWidget(button);
        mBookmarkView = new QListView;
        mBookmarkView->setModel(mBookModel);
        mBookmarkView->setPalette(QPalette(Qt::white));
        mBookmarkView->setAutoFillBackground(true);
        mSideView->addWidget(bookbar);
        mSideView->addWidget(mBookmarkView);
        mSideView->restoreState(mSettings.value("sw_geom").toByteArray());
        connect(mSideView, &QSplitter::splitterMoved, [=](){
            mSettings.setValue("sw_geom", mSideView->saveState());
        });

        bool bookmarks_initially_visible = mSettings.value("show_bookmarks").toBool();
        button->setChecked(!bookmarks_initially_visible);
        mBookmarkView->setVisible(bookmarks_initially_visible);

        connect(button, &QToolButton::toggled,
                [=] (bool toggled)
        {
            mBookmarkView->setVisible(!toggled);
            mSettings.setValue("show_bookmarks", !toggled);
        });

        connect(mProjectView, &QTreeView::activated,
                [=](const QModelIndex& index)
        {
            auto    model = static_cast<const OpenedDocumentModel*>(index.model());
            Document * d = model->document(index);
            if (d) activateDocumentAtLine(d, nullptr, -1);
        });
        connect(mBookmarkView, &QTreeView::activated,
                [=](const QModelIndex& index)
        {
            QString tag;
            int     line;
            auto    model = static_cast<const BookmarkModel*>(index.model());
            Document * d = model->document(index, &tag, &line);
            if (tag.isEmpty()) tag = QString::number(line);
            if (d) activateDocumentAtLine(d, tag.isEmpty() ? nullptr : &tag, line);
        });

        QWidget * padding = new QLabel(tr("Projects", TRCTX));
        QScopedPointer<QWidget> pToolbar(padding);
        QScopedPointer<QWidget> pWindow(mSideView);
        emit addWidget(&pWindow, &pToolbar,
                       editorWidgetId,
                       GUI_POSITION_LEFT);


    }

    /*create editor panel*/ {
        mEditorView = new QStackedWidget;
        connect(mEditorView, &QObject::destroyed,
        [=] () { mEditorView = nullptr; });

        mEditorToolbar = new QStackedWidget;
        connect(mEditorToolbar, &QObject::destroyed,
        [=] () { mEditorToolbar = nullptr; });

        mEditorStub = new ::EditorStub();
        mEditorView->addWidget(mEditorStub);
        mEditorStubbar = new QWidget();
        mEditorToolbar->addWidget(mEditorStubbar);

        QScopedPointer<QWidget> pToolbar(mEditorToolbar);
        QScopedPointer<QWidget> pWindow(mEditorView);
        emit addWidget(&pWindow, &pToolbar,
                       editorWidgetId,
                       GUI_POSITION_CENTER);

        emit addMode(this,
                     "editor",
                     tr("Editor", TRCTX),
                     tr("Switch to \"Editor\" mode", TRCTX),
                     editorWidgetId, false, 10);
    }
    return true;
}
void Plugin::stop()
{
    mProjectPanel  = nullptr;
    mProjectModel  = nullptr;
    mRecentModel   = nullptr;
    mStackW1       = nullptr;
    mStackW2       = nullptr;
    mTitleButton   = nullptr;
    mInDialog      = nullptr;

    mEditorView    = nullptr;
    mEditorToolbar = nullptr;
    mEditorStub    = nullptr;
    mEditorStubbar = nullptr;
    mCurrentDocument= nullptr;

    mSideView      = nullptr;
    mDocModel      = nullptr;
    mBookModel     = nullptr;
    mProjectView   = nullptr;
    mBookmarkView  = nullptr;

    mDialogs.clear();
    mEditors.clear();
    mToolbars.clear();
}
//======================================================
void Plugin::activateDocument(const QString &path) {
    if (!mSideView || !mEditorView)
        return;

    Document * d = mDocModel->document(path);
    if (!d) return;

    activateDocumentAtLine(d, nullptr, -1);
}
void Plugin::activateDocumentAtLine(Document *doc, const QString *anchor, int line)
{
    auto ids = doc->editorIds();
    if (ids.isEmpty()) return;

    auto i = ids.constBegin();
    auto i_editor = mEditors.constFind(*i);
    auto i_toolbar= mToolbars.constFind(*i);
    while(i_editor == mEditors.constEnd()) {
        if (++i == ids.constEnd())
            return;
        i_editor = mEditors.constFind(*i);
        i_toolbar= mToolbars.constFind(*i);
    }
    mEditorView->setCurrentWidget(i_editor.value());
    if (i_toolbar != mToolbars.constEnd())
        mEditorToolbar->setCurrentWidget(i_toolbar.value());
    else
        mEditorToolbar->setCurrentWidget(mEditorStubbar);

    i_editor.value()->setDocument(doc, anchor, line);
    mCurrentDocument = doc;
}
//======================================================
void Plugin::registerEditor(QScopedPointer<Editor> *editorptr, const QString &unique_id)
{
    Editor  * editor = editorptr->take();
    Q_ASSERT(editor != nullptr);
    mEditors.insert(unique_id, editor);
    QWidget * toolbar = editor->toolbar();
    if (toolbar)
        mToolbars.insert(unique_id, toolbar);

}
void Plugin::unregisterEditor(const QString &unique_id)
{
    mEditorView->setCurrentWidget(mEditorStub);
    mEditorToolbar->setCurrentWidget(mEditorStubbar);
    mCurrentDocument = nullptr;

    if (mEditorView) {
        auto i = mEditors.find(unique_id);
        if (i != mEditors.constBegin()) {
            delete i.value();
        }
        auto j = mToolbars.find(unique_id);
        if (j != mToolbars.constBegin()) {
            delete j.value();
        }
    }
}
//======================================================
void Plugin::addDocument(Document *doc) {
    Q_ASSERT(doc != nullptr);
    if (mSideView) {
        connect(doc, SIGNAL(closed(Document*)),
                this, SLOT(delDocument(Document*)));
        connect(doc, SIGNAL(closed(Document*)),
                this, SLOT(delDocument(Document*)));
        mDocModel->addDocument(doc);
    }
}
void Plugin::delDocument(Document *doc) {
    Q_ASSERT(doc != nullptr);
    if (mCurrentDocument == doc) {
        mCurrentDocument = nullptr;
        if (mEditorView) {
            mEditorView->setCurrentWidget(mEditorStub);
            mEditorToolbar->setCurrentWidget(mEditorStubbar);
        }
    }
}
//======================================================
void Plugin::setTitle() {
    if (mInDialog) {
        mTitleButton->setText(tr("Open or create new project", TRCTX));
        mTitleButton->setIcon(Core::getIcon("home"));
    } else {
        mTitleButton->setText(tr("Open or create new project", TRCTX));
        mTitleButton->setIcon(Core::getIcon("home"));
    }
}
void Plugin::addDialog(QScopedPointer<QWidget> *dialog,
                          const QString &unique_id,
                          const QString &name,
                          const QString &tooltip,
                          const QString &iconname)
{
    if (!mProjectPanel)
        return;

    QWidget * w = dialog->take();
    Q_ASSERT(w != nullptr);
    mDialogs.insert(unique_id, w);
    mStackW2->addWidget(w);
    mProjectModel->addElement(Core::getIcon(iconname),
                              name,
                              tooltip,
                              unique_id);
}
void Plugin::removeDialog(const QString &unique_id) {
    if (!mProjectPanel)
        return;

    auto i = mDialogs.find(unique_id);
    if (i != mDialogs.end())
    {
        mProjectModel->removeElement(unique_id);
        mStackW2->removeWidget(i.value());
        delete i.value();
        mDialogs.remove(unique_id);
    }
}
void Plugin::activateDialog(const QString &unique_id) {
    auto i = mDialogs.find(unique_id);
    if (i != mDialogs.end())
    {
        if (mInDialog) {
            QMetaObject::invokeMethod(mInDialog, "deactivateDialog",
                                      Qt::DirectConnection);
        }
        mInDialog = i.value();
        QMetaObject::invokeMethod(mInDialog, "activateDialog",
                                  Qt::DirectConnection);
        setTitle();
        mStackW1->hide();
        mStackW2->show();
    }
}
void Plugin::closeDialog() {
    if (mInDialog)
    {
        QMetaObject::invokeMethod(mInDialog, "deactivateDialog",
                                  Qt::DirectConnection);
        mInDialog = nullptr;
        setTitle();
        mStackW1->show();
        mStackW2->hide();
    }
}

void Plugin::sync() {
    int n = mRecentModel->rowCount();
    mSettings.beginWriteArray("recent", n);
    for(int i = 0; i < n; ++i)
    {
        auto ptr = mRecentModel->at(i);
        mSettings.setArrayIndex(i);
        mSettings.setValue("name", ptr->mName);
        mSettings.setValue("path", ptr->mAux);
    }
    mSettings.endArray();
}
void Plugin::openProject(const QString &name, const QString &path) {
    if (mRecentModel->pokeElementByUid(path))
        return;

    mRecentModel->addElement(Core::getIconForFile(path, "projects"), name, path, path);
    sync();
}
//======================================================
void Plugin::cleanup() {
    unloadPlugins();
}
