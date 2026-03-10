#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSortFilterProxyModel>

#include "client_gui_event_handler.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

const static QColor ERROR_FOREGROUND = Qt::red;
const static QColor ERROR_BACKGROUND = QColor(255, 200, 200);
const static QColor WARNING_FOREGROUND = Qt::black;
const static QColor WARNING_BACKGROUND = QColor(255, 255, 200);
const static QColor INFO_FOREGROUND = Qt::blue;
const static QColor INFO_BACKGROUND = QColor(255, 255, 255);
const static QColor HIGHLIGHTED_ROW = QColor("grey");
const static QColor HIGHLIGHTED_ROW_TEXT = Qt::white;

const static QString NON_EXISTENT_VALUE = "^$";

QMap<QString, QPair<QColor, QColor>> push_styles = {{"Error", {ERROR_FOREGROUND, ERROR_BACKGROUND}},
                                                    {"Warning", {WARNING_FOREGROUND, WARNING_BACKGROUND}},
                                                    {"Info", {INFO_FOREGROUND, INFO_BACKGROUND}}};

const static QColor ERROR_Foreground = Qt::red;
const static QColor ERROR_Background = QColor(255, 200, 200);
const static QColor WARNING_Foreground = Qt::black;
const static QColor WARNING_Background = QColor(255, 255, 200);
const static QColor INFO_Foreground = Qt::blue;
const static QColor INFO_Background = QColor(255, 255, 255);

const static QRegularExpression re_ip_address(
    "^(([1-9]?\\d|1\\d\\d|25[0-5]|2[0-4]\\d)\\.){3}([1-9]?\\d|1\\d\\d|"
    "25[0-5]|2[0-4]\\d)$");
const static QRegularExpression re_ip_port(
    "^(6553[0-5]|655[0-2]\\d|65[0-4]\\d{2}|6[0-4]\\d{3}|[1-5]\\d{4}|[1-9]\\d{0,3}|0)$");

bool isValidIPv4(const QString& ip) {
    QRegularExpressionMatch match = re_ip_address.match(ip);
    return match.hasMatch();
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    auto* eventHandler = new ClientGUIEventHandler(this);
    auto handler = std::shared_ptr<ClientGUIEventHandler>(eventHandler);
    client_ = std::make_shared<client_lib::ClientApp>(handler);

    filters_dialog = new FiltersDialog(this);

    connect(eventHandler, &ClientGUIEventHandler::pushReceived, this, &MainWindow::handlePushReceived);
    connect(eventHandler, &ClientGUIEventHandler::acceptPush, this, &MainWindow::handleAcceptPush);
    connect(eventHandler, &ClientGUIEventHandler::connected, this, &MainWindow::handleConnected);
    connect(eventHandler, &ClientGUIEventHandler::disconnected, this, &MainWindow::handleDisconnected);
    connect(eventHandler, &ClientGUIEventHandler::reconnect, this, &MainWindow::handleReconnect);
    connect(eventHandler, &ClientGUIEventHandler::dataSend, this, &MainWindow::handleDataSend);
    connect(eventHandler, &ClientGUIEventHandler::errorOccurred, this, &MainWindow::handleError);

    connect(ui->pushButton_filters, &QPushButton::clicked, this, &MainWindow::on_pushButton_filters_clicked);
    connect(filters_dialog, &FiltersDialog::filterChanged, this, &MainWindow::onFilterChanged);

    init();
}

MainWindow::~MainWindow() {
    qDebug() << "MainWindow destructor";

    delete filters_dialog;
    delete ui;
}

void MainWindow::init() {
    model_ = new QStandardItemModel();
    model_->setHorizontalHeaderLabels({"Время", "Приоритет", "Тип", "Источник", "Текст"});

    proxyModel_ = new MultiFilterProxyModel(this);
    proxyModel_->setSourceModel(model_);

    ui->qtv_push_list->setModel(proxyModel_);
    ui->qtv_push_list->horizontalHeader()->setStretchLastSection(true);
    ui->qtv_push_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->qtv_push_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->qtv_push_list->setSelectionMode(QAbstractItemView::SingleSelection);

    QPalette push_list_palette = ui->qtv_push_list->palette();
    push_list_palette.setColor(QPalette::Highlight, HIGHLIGHTED_ROW);
    push_list_palette.setColor(QPalette::HighlightedText, HIGHLIGHTED_ROW_TEXT);
    ui->qtv_push_list->setPalette(push_list_palette);

    auto ip_address_validator = new QRegularExpressionValidator(re_ip_address, this);
    ui->leServerAddress->setValidator(ip_address_validator);

    auto ip_port_validator = new QRegularExpressionValidator(re_ip_port, this);
    ui->leServerPort->setValidator(ip_port_validator);

    ui->label_connection_status->setPixmap(QPixmap(":/resources/disconnected.png"));
}

void MainWindow::onFilterChanged(const QString& filterText, int column, bool enabled) {
    proxyModel_->setColumnFilter(column, filterText, enabled);
}

void MainWindow::on_btnExit_clicked() {
    client_->stopClientLoop();
    QApplication::quit();
}

void MainWindow::on_btnConnect_clicked() {
    connectToServer();
}

void MainWindow::on_btnDisconnect_clicked() {
    disconnectFromServer();
}

void MainWindow::connectToServer() {
    if (!isValidIPv4(ui->leServerAddress->text())) {
        QMessageBox::warning(this, "Ошибка заполнения", "Неверный Ip-адрес сервера!");
        return;
    }

    if (ui->leServerPort->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка заполнения", "Введите номер порта!");
        return;
    }

    client_->configure(ui->leServerAddress->text().toStdString(), ui->leServerPort->text().toStdString());
    client_->runClientAppLoop();
    client_->connect();
}

void MainWindow::disconnectFromServer() {
    client_->disconnect();
}

void MainWindow::on_pushButton_filters_clicked() {
    filters_dialog->show();
}

// Вынести в отдельный файл или убрать в common/push/push.h и возвращать std::string
QString pushPriorityToString(PushPriority client_type) {
    switch (client_type) {
        case PushPriority::Normal:
            return "Normal";
        case PushPriority::High:
            return "High";
        case PushPriority::Critical:
            return "Critical";
        default:
            return "Unknown priority";
    }
}

// Вынести в отдельный файл или убрать в common/push/push.h и возвращать std::string
QString pushCategoryToString(PushCategory client_type) {
    switch (client_type) {
        case PushCategory::Notification:
            return "Notification";
        case PushCategory::Warning:
            return "Warning";
        case PushCategory::Error:
            return "Error";
        default:
            return "Unknown category";
    }
}

// Вынести в отдельный файл или убрать в common/push/push.h и возвращать std::string
QString pushSourceToString(PushSource client_type) {
    switch (client_type) {
        case PushSource::Source1:
            return "Source1";
        case PushSource::Source2:
            return "Warning";
        case PushSource::Source3:
            return "Source3";
        default:
            return "Unknown source";
    }
}

void MainWindow::handlePushReceived(uint32_t id, const Push& push) {
    QDateTime now = QDateTime::currentDateTime();

    QList<QStandardItem*> row;

    QStandardItem* timeItem = new QStandardItem(now.time().toString("hh:mm:ss"));
    QStandardItem* priorityItem = new QStandardItem(pushPriorityToString(push.priority));
    QStandardItem* typeItem = new QStandardItem(pushCategoryToString(push.category));
    QStandardItem* sourceItem = new QStandardItem(pushSourceToString(push.source));
    QStandardItem* textItem = new QStandardItem(push.text.c_str());

    if (auto it = push_styles.find(pushCategoryToString(push.category)); it != push_styles.end()) {
        typeItem->setForeground(it->first);
        QBrush bg(it->second);
        for (auto* item : {timeItem, priorityItem, typeItem, sourceItem, textItem})
            item->setBackground(bg);
    }

    row << timeItem << priorityItem << typeItem << sourceItem << textItem;

    model_->appendRow(row);

    ui->qtv_push_list->scrollToBottom();

    qDebug() << "Пуш-уведомление: " + now.time().toString("hh:mm:ss") + " " + pushPriorityToString(push.priority) +
                    " " + pushCategoryToString(push.category) + " " + pushSourceToString(push.source) + " " +
                    push.text.c_str();
}

void MainWindow::handleAcceptPush(uint32_t id) {
    qDebug() << "onAcceptPush message";
}

void MainWindow::handleConnected() {
    qDebug() << "Connected to server";
    ui->label_connection_status->setPixmap(QPixmap(":/resources/connected.png"));
}

void MainWindow::handleDisconnected() {
    qDebug() << "Disconnected from server";
    ui->label_connection_status->setPixmap(QPixmap(":/resources/disconnected.png"));
}

void MainWindow::handleReconnect() {
    qDebug() << "onReconnect message";
}

void MainWindow::handleDataSend(std::size_t size) {
    qDebug() << "onDataSend message";
}

void MainWindow::handleError(const QString& error) {
    qDebug() << "Error message: " << error;
}
