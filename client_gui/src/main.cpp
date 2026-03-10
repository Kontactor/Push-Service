#include <QApplication>
#include <memory.h>

#include "auth.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    Auth* auth = new Auth();
    auth->setAttribute(Qt::WA_DeleteOnClose);
    auth->show();

    return a.exec();
}
