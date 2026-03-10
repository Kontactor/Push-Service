#ifndef AUTH_H
#define AUTH_H

#include <QWidget>

#include "client_lib.h"
#include "user_manager.h"

namespace Ui {
class Auth;
}

class Auth : public QWidget {
    Q_OBJECT

public:
    explicit Auth(QWidget* parent = nullptr);
    Auth(QString username, QWidget* parent = nullptr);
    ~Auth();

private slots:
    void on_btnSignIn_clicked();
    void on_btnSignInPage_clicked();
    void on_btnSignUpPage_clicked();

private:
    Ui::Auth* ui;
    user_manager::User user_;  // Заглушка. Надеюсь на клиенте будет реализовано хранение (username и тип клиента)

    void init();
    bool validateForm() const;
    std::optional<std::shared_ptr<client_lib::ClientApp>> performAuthorization() const;
    void showMainWindow(std::shared_ptr<client_lib::ClientApp> client);
};

#endif  // AUTH_H
