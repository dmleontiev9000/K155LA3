#pragma once

/*
 * может понадобиться задать определенный стиль widgetам но имеющиеся
 * стили кривые и могут не все и что хуже - не делают это автоматически.
 *   K_SET_WIDGET_STYLE void signalname(QWidget * widget)
 *   K_UNSET_WIDGET_STYLE void signalname(QWidget * widget)
 */
#ifndef Q_MOC_RUN
#  define K_SET_WIDGET_STYLE
#  define K_UNSET_WIDGET_STYLE
#endif

/*
 * для работы с отрисовщиком используется
 * TODO: описать протокол
 */
#ifndef Q_MOC_RUN
#  define K_RENDERER_HINT_LOCAL   0x0001
#  define K_RENDERER_HINT_REMOTE  0x0002
#  define K_RENDERER_HINT_COMPUTE 0x0004
#  define K_CREATE_RENDERER
#  define K_DELETE_RENDERER

#  define K_RENDERER_CREATED
#  define K_RENDERER_CONTEXT_LOST

#  define K_RENDERER_TRIGGER
#  define K_BIND_RENDERER_ENDPOINT
#  define K_SET_RENDERER_DATA
#endif

#define GUI_POSITION_LEFT (-1)
#define GUI_POSITION_CENTER (0)
#define GUI_POSITION_RIGHT (1)

#define GUI_MODE_FIRST (-1)
#define GUI_MODE_MIDDLE (-2)
#define GUI_MODE_LAST (-3)
#define GUI_MODE_TAIL (-4)
/*
 * Для GUI приложений предоставляется унифицированый интерфейс для работы с окном.
 * Интерфейс может быть скомпонован различными способами, например в стиле QtCreator.
 *
 * Предоставляются следующие стандартные области окна:
 *    Переключатель режимов: toolbar с кнопками с иконками. Возможно будут подписи(не факт).
 *       Потрудитесь предоставить иконки разных размеров(16,24,32,48,64 и т.д.). Размер может быть разным
 *    Главное окно(вида QStackedWidget) в которое можно вложить свои widgetы
 *       Для каждого widgetа предусмотрена отдельная панель инструментов
 *       фиксированного размера.
 *    Окно сообщений типа tabwidget. Может быть показано, или скрыто с экрана
 *       В окне сообщений можно добавить свой widget. Он будет пронумерован(иконок нет!)
 *    Меню. Ну просто меню как у всех.
 *
 * Вообще этот интерфейс заточен под имитацию QtCreator, но может пригодиться и на ПКК, если
 * расположить элементы в ином формате.
 */
#ifndef Q_MOC_RUN

#  define K_GUI_WANT_CLOSE     //сигнал: уведомляет о намерении завершить работу
                               //сигнатура: QTreeWidget   *unsaved_list
                               //должен подключаться только в режиме Qt::DirectConnection
                               //создайте и добавьте в unsaved_list QTreeWidgetItemы
                               //с описанием того, какие элементы не были сохранены
                               //данные элементы переходят во владение модуля Gui и
                               //предназначены только для отображения. так что
                               //не стоит сохранять в них какую-либо дополнительную
                               //информацию.

#  define K_GUI_SAVE_DATA      //сигнал: уведомляет о намерении завершить работу
                               //с сохранением всех несохранённых элементов
                               //просто сохраните здесь все измененные файлы

#  define K_GUI_ADD_MODE       //сигнал: регистрирует новый рабочий режим в окне
                               //сигнатура: QObject       *object,
                               //           const QString& iconname,
                               //           const QString& title,
                               //           const QString& tooltip,
                               //           const QString& widgetid,
                               //           bool nosidebars,
                               //           unsigned order
                               //обратите внимание, что сигналы об активации
                               //данного режима будут приходить к тому объекту,
                               //который запросил создание режима
                               //оbject может поддерживать следующие API:
#  define K_GUI_ACTIVATED      //слот: сообщает об активации. параметр bool isActivated
                               //на этом всё про K_GUI_ADD_MODE
                               //всё, что будет написано ниже, уже не относится
                               //к objectу

#  define K_GUI_ACTIVATE_MODE  //сигнал: требует показать рабочий режим
                               //сигнатура: const QString& widgetid
                               //           bool exclusive

#  define K_GUI_HIDE_MODE      //сигнал: требует скрыть рабочий режим
                               //сигнатура: const QString& widgetid

#  define K_GUI_ADD_WIDGET     //сигнал: регистрирует новое рабочее окно
                               //сигнатура: QScopedPointer<QWidget>* widget,
                               //           QScopedPointer<QWidget>* toolbar,
                               //           const QString& widgetid,
                               //           int position
                               //toolbar может быть nullptr, если не нужен
                               //position = -1, 0, +1. -1 - виджет слева
                               //+1 виджет справа
                               //ВАЖНО:
                               //у widget и toolbar не должно быть родителя
                               //эти окна не должны быть показаны на экране
                               //то есть вообще ни разу, свежесозданы и отданы
                               //в GUI. Их владельцем должен быть QScopedPointer
                               //метод сам заберет QWidget из умного указателя.
                               //эти окна должны быть созданы в главном потоке.
                               //эти окна могут жить дольше, чем ваш плагин
                               //и вообще вы отдаёте владение этими окнами другому плагину.
                               //учитывайте это.

#  define K_GUI_REMOVE_MODE    //cигнал: удаляет ранее добавленый режим
                               //сигнатура: QObject       *object,

#  define K_GUI_REMOVE_WIDGET  //сигнал: удаляет рабочее окно
                               //или:       const QString& widgetid

#  define K_GUI_ADD_ACTION     //сигнал: добавляет действие на панель действий
                               //сигнатура: K::Widgets::Action *action
#  define K_GUI_REMOVE_ACTION  //сигнал: удаляет действие с панели действий
                               //сигнатура: K::Widgets::Action *action

#  define K_GUI_GEOMETRY_CHANGE//сигнал: уведомляет о положении рабочей области
                               //сигнатура: QRect position
#  define K_GUI_HIDE_POPUPS    //сигнал: скрыть popups


#  define K_GUI_ADD_INFOTAB    //сигнал: добавляет новый tab в окно сообщений
                               //сигнатура: QScopedPointer<QWidget>* tab,
                               //           const QString& tabname,
                               //           unsigned order
                               //widget tab должен/может поддерживать следующие API:
#  define K_GUI_ACTIVATED      //слот: сообщает об активации. параметр bool isActivated
#  define K_GUI_SHOW           //сигнал[не обязательно]: показать эту страницу

#  define K_GUI_ADDMENU        //сигнал[не обязательно]: добавить меню
                               //сигнатура: const QString& path_in_menu,
                               //           QMenuItem* menuitem



#endif
