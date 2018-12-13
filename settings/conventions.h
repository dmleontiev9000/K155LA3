#pragma once


#ifndef Q_MOC_RUN

#  define K_SETTINGS_ADD_PANEL //слот: уведомляет окно настроек об добавлении панели
                               //сигнатура: QScopedPointer<QWidget>* dialog,
                               //const QIcon &icon,
                               //const QString &title,
                               //const QString &widgetid

#  define K_SETTINGS_REMOVE_PANEL//слот: уведомляет окно настроек об удалении панели
                               //сигнатура: const QString& unique_id

#  define K_SETTINGS_SHOW_PANEL//слот: уведомляет окно настроек об необходимости показа панели
                               //сигнатура: const QString& unique_id


#endif

