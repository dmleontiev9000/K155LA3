#include <plugin.h>
#include <style.h>
#include <icons.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QFontMetrics>
#include <QtPlugin>
#include <QTreeWidgetItem>
#include <QStringRef>
#include <QDebug>

using namespace K::Help::Internal;

using K::Core::Style;

static const char TRCTX[] = {"help"};
namespace {
    struct Entry : public QTreeWidgetItem {
        Entry() : QTreeWidgetItem(QTreeWidgetItem::UserType) {
        }
        QUrl url;
    };
} //namespace ::
Plugin::Plugin()
    : mSettings(QSettings::UserScope, "org.k.help")
{
}
Plugin::~Plugin()
{

}
const QString& Plugin::path() const {
    return mLocation;
}
void Plugin::setPath(const QString &s) {
    if (!mLocked)
        mLocation = s;
}

static const QString widgetId("k.ide.help");
bool Plugin::init() {
    if (!loadPlugins({"gui"})) {
        mErrorString = tr("failed to load gui plugin", TRCTX);
        return false;
    }

    if (mLocation.isEmpty()) {
        mLocation = IfCore::instance()->appPath()+"/share/help";
    }

    mHelpLocation.setPath(mLocation);
    if (!mHelpLocation.isReadable())
    {
        mErrorString = tr("failed to load help plugin: invalid location", TRCTX);
        return false;
    }

    mEngine = new QHelpEngine(mHelpLocation.absoluteFilePath("index.qdocconf"), this);
    //connect(mEngine, SIGNAL(setupStarted()), this, SLOT(helpInitStarted()));
    //connect(mEngine, SIGNAL(setupFinished()), this, SLOT(helpInitFinished()));

    if (!mEngine->setupData()) {
        mErrorString = tr("failed to init help plugin: ", TRCTX)+mEngine->error();
        delete mEngine;
        mEngine = nullptr;
        return false;
    }

    mListPos = 0;
    mTitleList.clear();
    mUrlList.clear();

    QPalette white(QColor(255,255,255));
    QSplitter * widget = new QSplitter(Qt::Horizontal);
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetRemoved()));

    /*create left panel*/ {
        mTopicSelector = mEngine->contentWidget();
        mTopicSelector->setPalette(white);
        mTopicSelector->setAutoFillBackground(true);
        QSplitter * vs = new QSplitter(Qt::Vertical);
        vs->addWidget(mTopicSelector);

        QWidget     * wrapper = new QWidget;
        vs->addWidget(wrapper);
        vs->setStretchFactor(0, 0);

        QVBoxLayout * vbox = new QVBoxLayout(wrapper);
        QLabel * header = new QLabel(tr("Bookmarks", TRCTX));
        Style::instance()->addPanel(header, Style::Toolbar);

        vbox->addWidget(header);
        mBookmarkSelector = new QTreeWidget;
        mBookmarkSelector->setHeaderHidden(true);
        mBookmarkSelector->setAutoFillBackground(true);
        mBookmarkSelector->setPalette(white);
        mBookmarkSelector->setSelectionMode(QAbstractItemView::NoSelection);
        connect(mBookmarkSelector, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
                this, SLOT(bookmarkClicked(QTreeWidgetItem*)));
        vbox->addWidget(mBookmarkSelector, 1);

        widget->addWidget(vs);
    }

    /*create right(main) panel*/ {
        QWidget * wrapper = new QWidget();
        QGridLayout * grid = new QGridLayout(wrapper);
        mWebPage = new QTextBrowser();
        mWebPage->setPalette(white);
        mWebPage->setAutoFillBackground(true);
        mWebPage->setOpenExternalLinks(false);
        mWebPage->setOpenLinks(false);
        mWebPage->setSearchPaths(QStringList(mHelpLocation.absolutePath()));
        connect(mWebPage, SIGNAL(anchorClicked(QUrl)),
                this, SLOT(topicClicked(QUrl)));
        grid->addWidget(mWebPage, 0, 0, 1, 2);
        grid->setRowStretch(0, 1);
        grid->addWidget(new QLabel(tr("Search:", TRCTX)), 1, 0, 1, 1);
        mSearchLine = new QLineEdit;
        grid->addWidget(mSearchLine, 1, 1, 1, 1);
        grid->setColumnStretch(1, 1);
        connect(mSearchLine, SIGNAL(editingFinished()), this,
                SLOT(searchOnPage()));
        widget->addWidget(wrapper);
        widget->setStretchFactor(1, 1);
    }

    QWidget * toolbar = new QWidget;
    Style::instance()->addPanel(toolbar, Style::Toolbar);
    /*create toolbar*/ {
        auto hbox = new QHBoxLayout(toolbar);

        auto hme  = new QToolButton();
        hme->setIcon(Core::getIcon("home"));
        hbox->addWidget(hme, 0, Qt::AlignVCenter);
        connect(hme, SIGNAL(clicked(bool)), this, SLOT(home()));

        auto bck  = new QToolButton();
        bck->setIcon(Core::getIcon("left"));
        hbox->addWidget(bck, 0, Qt::AlignVCenter);
        connect(bck, SIGNAL(clicked(bool)), this, SLOT(back()));

        auto fwd  = new QToolButton();
        fwd->setIcon(Core::getIcon("right"));
        hbox->addWidget(fwd, 0, Qt::AlignVCenter);
        connect(fwd, SIGNAL(clicked(bool)), this, SLOT(forward()));

        mHistoryBox = new QComboBox;
        hbox->addWidget(mHistoryBox, 1, Qt::AlignVCenter);
        connect(mHistoryBox, SIGNAL(currentIndexChanged(int)), this, SLOT(historyClicked(int)));
    }

    //send widgets to gui
    QScopedPointer<QWidget> windowPtr(widget);
    QScopedPointer<QWidget> toolbarPtr(toolbar);
    emit addWidget(&windowPtr, &toolbarPtr, widgetId, GUI_POSITION_RIGHT);
    emit addMode(this,
                 "help",
                 tr("Help", TRCTX),
                 tr("Switch to \"Help\" mode", TRCTX),
                 widgetId, true, GUI_MODE_TAIL);
    if (!windowPtr.isNull() || !toolbarPtr.isNull()) {
        mErrorString = tr("Failed to insert widget to gui", TRCTX);
        emit removeWidget(widgetId);
        emit removeMode(this);
        unloadPlugins();
        delete mEngine;
        mEngine = nullptr;
        return false;
    }
    mGuiExists = true;

    /*fill bookmarks*/ {
        int n = mSettings.beginReadArray("bookmarks");
        for(int i = 0; i < n; ++i) {
            mSettings.setArrayIndex(i);

            QUrl url = mSettings.value("url").toUrl();
            if (url.isEmpty())
                continue;
            //FIXME: check scheme!

            QString name = mSettings.value("name").toString();
            if (name.isEmpty())
                continue;

            Entry * bme = new Entry();
            bme->setText(0, name);
            bme->url = url;
            mBookmarkSelector->addTopLevelItem(bme);
        }
        mSettings.endArray();
    }

    home();
    mLocked = true;
    return true;
}
void Plugin::widgetRemoved() {
    mGuiExists = false;
}
void Plugin::topicClicked(const QUrl &url) {
    openUrl(url, QString(), false);
}

void Plugin::openUrl(const QUrl &_url, const QString& _title, bool keep) {
    Q_ASSERT(mUrlList.size() == mTitleList.size());
    if (!mGuiExists)
        return;

    if (mListPos < 0 || mListPos >= mUrlList.size())
        keep = false;

    if (!keep) {
        int m = mTitleList.size()-1;
        while(m > mListPos) {
            mTitleList.pop_back();
            mUrlList.pop_back();
            mHistoryBox->removeItem(m);
            --m;
        }
    }
    QUrl       url   = _url;
    QString    title = _title;

    bool notfound = true;
    if (url.scheme() == "search")
    {
        auto links = mEngine->linksForIdentifier(url.path());
        if (links.size() == 1) {
            url = links.begin().value();
            title = links.begin().key();
        } else if (!links.isEmpty()){
            static QString title(tr("<html><head><title>Search results</title></head><body>"
                                    "<h2>Several items are available:<h2><br>", TRCTX));
            QString page;
            page.reserve(1024);
            page.append(title);
            for(auto i = links.constBegin(); i != links.constEnd(); ++i) {
                page.append("<div onclick=\"location.href='");
                page.append(i.value().toString(QUrl::None));
                page.append("';\"><img width=\"32px\" src=\"entry.png\"><div>"
                            "<p style=\"font-size:16;font-color:#000\">");
                page.append(i.key());
                page.append("</p><br><p style=\"font-size:12;font-color:#080\">");
                page.append(i.value().toString(QUrl::None));
                page.append("</p></div></div><br>");
            }
            page.append("</body></html>");
            mWebPage->setHtml(page);
            notfound = false;
        }
    } else if (url.scheme() == "qthelp") {
        url = mEngine->findFile(url);
        if (url.isValid()) {
            mWebPage->setSource(url);
            title = mWebPage->documentTitle();
            notfound = false;
        }
    } else if (url.scheme() == "file") {
        mWebPage->setSource(url);
        title = mWebPage->documentTitle();
        notfound = false;
    }

    if (notfound) {
        url = QUrl("notfound:notfound");
        title = tr("Not found", TRCTX);
        QString msg = tr("<h1>Not found<h1><br>"
                         "contents not found by given link", TRCTX);
        mWebPage->setHtml(msg);
    }

    if (!keep) {
        mUrlList.push_back(url);
        mTitleList.push_back(title);
        if (mUrlList.size() > 30) {
            mUrlList.pop_front();
            mTitleList.pop_front();
            mHistoryBox->removeItem(0);
        }
    } else {
        mUrlList[mListPos] = url;
        mTitleList[mListPos] = title;
    }
}
void Plugin::bookmarkClicked(QTreeWidgetItem * item) {
    if (item->type() != QTreeWidgetItem::UserType)
        return;
    Entry * e = static_cast<Entry*>(item);
    openUrl(e->url, e->text(0), false);
}
void Plugin::historyClicked(int index) {
    if (index >= 0 || index < mUrlList.size()) {
        mListPos = index;
        openUrl(mUrlList[index], mTitleList[index], true);
    }
}
void Plugin::titleChanged(const QString &title) {
    Q_ASSERT(mListPos >= 0);
    Q_ASSERT(mListPos <= mTitleList.size());
    Q_ASSERT(mTitleList.size() == mUrlList.size());

    if (mTitleList.isEmpty())
        return;

    mTitleList[mListPos] = title;
    mHistoryBox->setItemText(mListPos, title);
}
void Plugin::searchOnPage() {
    if (!mGuiExists)
        return;

    QString text = mSearchLine->text();
    if (text.isEmpty())
        return;

    mWebPage->find(text);
}
void Plugin::home() {
    if (!mGuiExists)
        return;

    QUrl url;
    url.setScheme("file");
    url.setPath(mHelpLocation.absoluteFilePath("index.html"));
    openUrl(url, tr("Home"), false);
}
void Plugin::back() {
    if (!mGuiExists)
        return;

    if (mListPos) {
        mHistoryBox->setCurrentIndex(--mListPos);
    }
}
void Plugin::forward() {
    if (!mGuiExists)
        return;

    if (mListPos < mUrlList.size() - 1) {
        mHistoryBox->setCurrentIndex(++mListPos);
    }
}
void Plugin::stop() {
    emit removeWidget(widgetId);
    emit removeMode(this);
    mGuiExists = false;
}
void Plugin::cleanup() {
    delete mEngine;
    mEngine = nullptr;
    mLocked    = false;
    unloadPlugins();
}
bool Plugin::requiresGui() const {
    return true;
}
QObject * Plugin::self() {
    return this;
}
QString Plugin::errorString() {
    return mErrorString;
}
void Plugin::openPage(const QString& topic) {
    if (!mGuiExists)
        return;

    QUrl url;
    url.setScheme("search");
    url.setPath(topic);
    openUrl(url, QString(), false);
}
