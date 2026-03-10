#ifndef ADMINPANEL_H
#define ADMINPANEL_H

#include <QWidget>

namespace Ui {
class AdminPanel;
}

class AdminPanel : public QWidget {
    Q_OBJECT

public:
    explicit AdminPanel(QWidget* parent = nullptr);
    ~AdminPanel();

signals:
    void pushMessage(const QString& message);

private slots:
    void on_btnPushMessage_clicked();

private:
    void updateButtonState();
    Ui::AdminPanel* ui;
};

#endif  // ADMINPANEL_H
