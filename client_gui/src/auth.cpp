#include "auth.h"

#include <QDebug>
#include <QMessageBox>
#include <memory.h>

#include "client_lib_stub.h"
#include "mainwindow.h"
#include "ui_auth.h"

Auth::Auth(QWidget* parent) : QWidget(parent), ui(new Ui::Auth) {
    ui->setupUi(this);

    init();
}

Auth::Auth(QString username, QWidget* parent) : QWidget(parent), ui(new Ui::Auth) {
    ui->setupUi(this);

    init();
    ui->leUsername->setText(username);
}

Auth::~Auth() {
    qDebug() << "Auth destructor";
    delete ui;
}

void Auth::init() {
    ui->btnSignInPage->setChecked(true);
    ui->Pages->setCurrentWidget(ui->AuthorizationPage);
}

void Auth::on_btnSignIn_clicked() {
    if (!validateForm()) {
        QMessageBox::warning(this, "Ошибка заполнения", "Логин и пароль должны быть заполнены!");
        return;
    }

    const auto auth_result = performAuthorization();

    if (!auth_result.has_value()) {
        QMessageBox::warning(this, "Ошибка авторизации", "Пользователь не найден!");
        return;
    }

    // TODO Заглушка
    const auto clientType =
        client_lib::authorizeClient(ui->leUsername->text().toStdString(), ui->lePassword->text().toStdString());
    user_ = user_manager::User(ui->leUsername->text(), clientType.value_or(client_lib::ClientType::Unknown));

    if (user_.clientType == client_lib::ClientType::Unknown) {
        QMessageBox::warning(this, "Ошибка авторизации", "Неизвестный пользователь!");
        return;
    }
    // Конец заглушки

    showMainWindow(auth_result.value());
}

bool Auth::validateForm() const {
    if (ui->leUsername->text().isEmpty() || ui->lePassword->text().isEmpty()) {
        return false;
    }

    return true;
}

std::optional<std::shared_ptr<client_lib::ClientApp>> Auth::performAuthorization() const {
    QString username = ui->leUsername->text();
    QString password = ui->lePassword->text();

    // PushMessageHandlerStub заглушка. В клиентской библиотеке пока только интерфейс
    auto handler = std::make_shared<client_lib::PushMessageHandlerStub>();
    auto client = std::make_shared<client_lib::ClientApp>(handler);

    if (client->login(username.toStdString(), password.toStdString())) {
        return client;
    }

    return std::nullopt;
}

void Auth::on_btnSignInPage_clicked() {
    ui->Pages->setCurrentWidget(ui->AuthorizationPage);
}

void Auth::on_btnSignUpPage_clicked() {
    ui->Pages->setCurrentWidget(ui->RegistrationPage);
}

void Auth::showMainWindow(std::shared_ptr<client_lib::ClientApp> client) {
    MainWindow* mainWindow = new MainWindow(client, user_);
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    mainWindow->show();

    this->close();
}
