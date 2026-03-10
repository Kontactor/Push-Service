#include "mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSortFilterProxyModel>

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

static QColor ERROR_Foreground = Qt::red;
static QColor ERROR_Background = QColor(255, 200, 200);
static QColor WARNING_Foreground = Qt::black;
static QColor WARNING_Background = QColor(255, 255, 200);
static QColor INFO_Foreground = Qt::blue;
static QColor INFO_Background = QColor(255, 255, 255);

#include "auth.h"
#include "client_lib.h"
#include "client_lib_stub.h"  // Заглушка
#include "ui_mainwindow.h"
#include "user_manager.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    init();
}

MainWindow::MainWindow(std::shared_ptr<client_lib::ClientApp> client, const user_manager::User user, QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), user_(user), client_(client) {
    ui->setupUi(this);

    model_ = new QStandardItemModel();
    model_->setHorizontalHeaderLabels({"Время", "Приоритет", "Тип", "Текст"});

    proxyModel_ = new QSortFilterProxyModel(this);
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

    filters_dialog = new FiltersDialog(this);

    connect(ui->pushButton_filters, &QPushButton::clicked, this, &MainWindow::on_pushButton_filters_clicked);
    connect(filters_dialog, &FiltersDialog::filterChanged, this, &MainWindow::onFilterChanged);

    init();
}

MainWindow::~MainWindow() {
    qDebug() << "MainWindow destructor";

    stopPushNotifications();

    delete filters_dialog;

    delete ui;
}

void MainWindow::init() {
    if (!client_) {
        QMessageBox::critical(this, "Initialization Error", "Client is not initialized");
        throw std::runtime_error("Client is not initialized");
    }

    ui->lblUsername->setText(user_.username);
    QString clientTypeInfo = user_manager::GetClietTypeStr(user_.clientType);
    ui->lblClientType->setText(clientTypeInfo);

    if (user_.clientType == client_lib::ClientType::Admin) {
        setupAdminPanel();
    }

    // TODO Заглушка
    startPushNotifications();
}

void MainWindow::onFilterChanged(const QString& filterText, int column, bool enabled) {
    if (enabled && !filterText.isEmpty()) {
        proxyModel_->setFilterKeyColumn(column);

        if (filterText.contains('|')) {
            QRegularExpression regExp(filterText);
            proxyModel_->setFilterRegularExpression(regExp);
        } else {
            proxyModel_->setFilterFixedString(filterText);
        }
    } else {
        proxyModel_->setFilterFixedString(NON_EXISTENT_VALUE);
    }
}

void MainWindow::onFilterChanged(const QString& filterText, int column, bool enabled) {
    if (enabled && !filterText.isEmpty()) {
        proxyModel_->setFilterKeyColumn(column);

        if (filterText.contains('|')) {
            QRegularExpression regExp(filterText);
            proxyModel_->setFilterRegularExpression(regExp);
        } else {
            proxyModel_->setFilterFixedString(filterText);
        }
    } else {
        proxyModel_->setFilterFixedString("");
    }
}

void MainWindow::setupAdminPanel() {
    adminPanel = new AdminPanel();
    adminPanel->setAttribute(Qt::WA_DeleteOnClose);
    adminPanel->layout()->setContentsMargins(0, 0, 0, 10);

    ui->VL_MainView->insertWidget(1, adminPanel);

    connect(adminPanel, &AdminPanel::pushMessage, this, &MainWindow::onPushMessage);
}

void MainWindow::on_btnLogOut_clicked() {
    LogOut();
}

void MainWindow::on_btnExit_clicked() {
    QApplication::quit();
}

void MainWindow::LogOut() {
    Auth* authWindow = new Auth(user_.username);
    authWindow->setAttribute(Qt::WA_DeleteOnClose);
    authWindow->show();

    this->close();
}

void MainWindow::on_btnConnect_clicked() {
    connectToServer();
}

void MainWindow::on_btnDisconnect_clicked() {
    disconnect();
}

void MainWindow::connectToServer() {
    // TODO Проверить, что сервер включен

    if (ui->leServerAddress->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка заполнения", "Не ввели Ip-адрес сервера!");
        return;
    }

    // TODO
    client_lib::connect(ui->leServerAddress->text().toStdString(), ui->leServerPort->text().toInt());
}

void MainWindow::disconnectFromServer() {
    // TODO заглушка
    client_lib::disconnect();
}

void MainWindow::onPushMessage(const QString& message) {
    pushMessage(message);
}

void MainWindow::pushMessage(const QString& message) {
    if (message.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Сообщение не может быть пустым"));
        return;
    }

    qDebug() << "Push message requested from admin panel:" << message;

    // TODO
    // client_->getCurrentUser()->sendPush(message.toStdString());

    // ui->listMessageWidget->addItem(message);
}

void MainWindow::startPushNotifications() {
    if (!notifyPushTimer_) {
        notifyPushTimer_ = std::make_unique<QTimer>(this);
        connect(notifyPushTimer_.get(), &QTimer::timeout, this, &MainWindow::onPushNotification);
    }

    notifyPushTimer_->start(5000);

    qDebug() << "Заглушка пуш-уведомлений запущена (интервал: 5 сек)";
}

void MainWindow::stopPushNotifications() {
    if (notifyPushTimer_ && notifyPushTimer_->isActive()) {
        notifyPushTimer_->stop();
        qDebug() << "Заглушка пуш-уведомлений остановлена";
    }
}

void MainWindow::onPushNotification() {
    notifyCounter_++;

    // Структура данных
    // TODO: заменить типы данных
    // QDateTime time ?
    // enum PushTupe type ?
    struct Message {
        QString time;
        QString priority;
        QString type;
        QString text;
    };

    // Список тестовых сообщений
    // TODO: после изменения Message нужно добавить преобразование типов в QString
    QList<Message> new_messages = {{"10:30:15", "Normal", "Info", "Сервер запущен"},
                                   {"11:45:20", "High", "Error", "Ошибка соединения"},
                                   {"12:15:50", "Critical", "Warning", "Высокая нагрузка"},
                                   {"12:22:42", "Normal", "Info", "Информационное сообщение"},
                                   {"13:55:00", "Critical", "Warning", "Высокая нагрузка"},
                                   {"14:08:10", "High", "Error", "Ошибка отправки сообщения"}};

    int index = QRandomGenerator::global()->bounded(new_messages.size());
    Message new_message = new_messages[index];

    QList<QStandardItem*> row;

    QStandardItem* timeItem = new QStandardItem(new_message.time);
    QStandardItem* priorityItem = new QStandardItem(new_message.priority);
    QStandardItem* typeItem = new QStandardItem(new_message.type);
    QStandardItem* textItem = new QStandardItem(new_message.text);

    if (auto it = push_styles.find(new_message.type); it != push_styles.end()) {
        typeItem->setForeground(it->first);
        QBrush bg(it->second);
        for (auto* item : {timeItem, priorityItem, typeItem, textItem})
            item->setBackground(bg);
    }

    row << timeItem << priorityItem << typeItem << textItem;

    model_->appendRow(row);

    ui->qtv_push_list->scrollToBottom();

    // Логируем
    qDebug() << "Пуш-уведомление: " + new_message.time + " " + new_message.priority + " " + new_message.type + " " +
                    new_message.text;
}

void MainWindow::on_pushButton_filters_clicked() {
    filters_dialog->show();
}
