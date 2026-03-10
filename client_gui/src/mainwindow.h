#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <memory.h>

#include "client_lib.h"
#include "filters_dialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnExit_clicked();
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();
    void on_pushButton_filters_clicked();
    void onFilterChanged(const QString& filterText, int column, bool enabled);
    void handlePushReceived(uint32_t id, const Push& push);
    void handleAcceptPush(uint32_t id);
    void handleConnected();
    void handleDisconnected();
    void handleReconnect();
    void handleDataSend(std::size_t size);
    void handleError(const QString& error);

private:
    Ui::MainWindow* ui;
    std::shared_ptr<client_lib::ClientApp> client_;
    QStandardItemModel* model_;
    MultiFilterProxyModel* proxyModel_;
    FiltersDialog* filters_dialog;

    void init();
    void connectToServer();
    void disconnectFromServer();
};
#endif  // MAINWINDOW_H
