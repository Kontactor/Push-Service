#ifndef FILTERS_DIALOG_H
#define FILTERS_DIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QVBoxLayout>

#include "ui_filters_dialog.h"

namespace Ui {
class FiltersDialog;
}

class FiltersDialog : public QDialog {
    Q_OBJECT

public:
    explicit FiltersDialog(QWidget* parent = nullptr);

signals:
    // Сигнал для изменения фильтра
    void filterChanged(const QString& filterText, int column, bool enabled);

private slots:

private:
    void updateFilter();

private:
    Ui::FiltersDialog* ui;
};

#endif  // FILTERS_DIALOG_H
