#include "filters_dialog.h"

FiltersDialog::FiltersDialog(QWidget* parent) : QDialog(parent), ui(new Ui::FiltersDialog) {
    ui->setupUi(this);

    ui->checkBox_info->setChecked(true);
    ui->checkBox_warning->setChecked(true);
    ui->checkBox_error->setChecked(true);

    connect(ui->checkBox_info, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
    connect(ui->checkBox_warning, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
    connect(ui->checkBox_error, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
}

void FiltersDialog::updateFilter() {
    QStringList selectedTypes;

    if (ui->checkBox_info->isChecked())
        selectedTypes << "Info";
    if (ui->checkBox_warning->isChecked())
        selectedTypes << "Warning";
    if (ui->checkBox_error->isChecked())
        selectedTypes << "Error";

    if (selectedTypes.isEmpty()) {
        emit filterChanged("", 2, false);
    } else {
        QString pattern = selectedTypes.join("|");
        emit filterChanged(pattern, 2, true);
    }
}
