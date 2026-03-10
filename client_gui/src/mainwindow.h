#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QTimer>
#include <memory.h>

#include "adminpanel.h"
#include "filters_dialog.h"
#include "user_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    MainWindow(std::shared_ptr<client_lib::ClientApp> client,
               const user_manager::User user,
               QWidget* parent = nullptr);  // TODO Заглушка. Временно передаю User
    ~MainWindow();

private slots:
    void on_btnLogOut_clicked();
    void on_btnExit_clicked();
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();
    void onPushMessage(const QString& message);
    void onPushNotification();
    void on_pushButton_filters_clicked();
    void onFilterChanged(const QString& filterText, int column, bool enabled);

private:
    Ui::MainWindow* ui;
    AdminPanel* adminPanel = nullptr;
    user_manager::User user_;
    std::shared_ptr<client_lib::ClientApp> client_;
    std::unique_ptr<QTimer> notifyPushTimer_;
    int notifyCounter_ = 0;
    QStandardItemModel* model_;
    QSortFilterProxyModel* proxyModel_;
    FiltersDialog* filters_dialog;

    void init();
    void setupAdminPanel();
    void LogOut();
    void connectToServer();
    void disconnectFromServer();
    void pushMessage(const QString& message);
    void startPushNotifications();  // Запуск заглушки. Прием пуш-сообщений от сервера
    void stopPushNotifications();  // Остановка заглушки
};
#endif  // MAINWINDOW_H
