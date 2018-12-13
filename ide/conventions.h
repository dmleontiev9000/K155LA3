#pragma once


#ifndef Q_MOC_RUN

#  define K_IDE_OPEN_PROJECT   //сигнал: уведомляет окно проектов об открытии
                               //нового проекта
                               //сигнатура: const QString& name,
                               //           const QString& path

#  define K_IDE_ACTIVATE_DOCUMENT
                               //слот: вызывается при активации проекта
                               //в окне проектов или документа
                               //в окне редактора
                               //сигнатура: const QString& path

#  define K_IDE_REGISTER_EDITOR
                               //слот: регистрирует новое окно редактора
                               //сигнатура: QScopedPointer<Editor>* editor,
                               //           const QString& unique_id
#  define K_IDE_UNREGISTER_EDITOR
                               //слот: дерегистрирует новое окно редактора
                               //сигнатура: const QString& unique_id


#  define K_IDE_ADD_PROJECT_DIALOG
                               //слот: задает диалог создания проекта
                               //сигнатура: QScopedPointer<QWidget>* dialog,
                               //           const QString& unique_id,
                               //           const QString& name,
                               //           const QString& tooltip,
                               //           const QString& iconname

#  define K_IDE_REMOVE_PROJECT_DIALOG
                               //слот: удаляет диалог создания проекта
                               //сигнатура: const QString& unique_id

#  define K_IDE_ACTIVATE_PROJECT_DIALOG
                               //слот: требует активации диалога
                               //сигнатура: const QString& unique_id

#  define K_IDE_CLOSE_PROJECT_DIALOG
                               //слот: сообщает что работа диалога завершена
                               //не забывайте его дернуть

#  define K_IDE_ADD_DOCUMENT   //слот: добавляет документ
                               //сигнатура: Document * document

#  define K_IDE_DEL_DOCUMENT   //слот: удаляет документ
                               //сигнатура: Document * document


#endif

