#include "adminpanel.h"

#include <QDebug>
#include <QMessageBox>

#include "ui_adminpanel.h"

AdminPanel::AdminPanel(QWidget* parent) : QWidget(parent), ui(new Ui::AdminPanel) {
    ui->setupUi(this);

    ui->btnPushMessage->setEnabled(false);

    connect(ui->cmbbxPushType, &QComboBox::currentTextChanged, this, &AdminPanel::updateButtonState);
    connect(ui->txtMessage, &QTextEdit::textChanged, this, &AdminPanel::updateButtonState);
}

AdminPanel::~AdminPanel() {
    qDebug() << "Admin panel destructor";
    delete ui;
}

// TODO: после утверждения формата пушей переработать для раздельной отправки сообщения и его типа
// либо структурой, либо двумя аргументами
void AdminPanel::on_btnPushMessage_clicked() {
    QString message = ui->cmbbxPushType->currentText() + " " + ui->txtMessage->toPlainText().trimmed();

    emit pushMessage(message);

    ui->txtMessage->clear();
    ui->cmbbxPushType->setCurrentIndex(0);
}

void AdminPanel::updateButtonState() {
    QString selectedText = ui->cmbbxPushType->currentText();
    bool hasTextSelected = !selectedText.isEmpty();

    QString messageText = ui->txtMessage->toPlainText().trimmed();
    bool hasMessageText = !messageText.isEmpty();

    ui->btnPushMessage->setEnabled(hasTextSelected && hasMessageText);
}
