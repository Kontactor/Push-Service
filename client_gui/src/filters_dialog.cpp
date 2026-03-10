#include "filters_dialog.h"

FiltersDialog::FiltersDialog(QWidget* parent) : QDialog(parent), ui(new Ui::FiltersDialog) {
    ui->setupUi(this);

    ui->checkBox_pushPriority_normal->setChecked(true);
    ui->checkBox_pushPriority_high->setChecked(true);
    ui->checkBox_pushPriority_critical->setChecked(true);

    ui->checkBox_pushCategory_notification->setChecked(true);
    ui->checkBox_pushCategory_warning->setChecked(true);
    ui->checkBox_pushCategory_error->setChecked(true);

    connect(ui->checkBox_pushPriority_normal, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
    connect(ui->checkBox_pushPriority_high, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
    connect(ui->checkBox_pushPriority_critical, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);

    connect(ui->checkBox_pushCategory_notification, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
    connect(ui->checkBox_pushCategory_warning, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
    connect(ui->checkBox_pushCategory_error, &QCheckBox::checkStateChanged, this, &FiltersDialog::updateFilter);
}

void FiltersDialog::updateFilter() {
    QCheckBox* senderCheckbox = qobject_cast<QCheckBox*>(sender());
    if (!senderCheckbox)
        return;

    QHash<QCheckBox*, QPair<int, QString>> checkboxMapping = {
        {ui->checkBox_pushPriority_normal, {1, "Normal"}},
        {ui->checkBox_pushPriority_high, {1, "High"}},
        {ui->checkBox_pushPriority_critical, {1, "Critical"}},
        {ui->checkBox_pushCategory_notification, {2, "Notification"}},
        {ui->checkBox_pushCategory_warning, {2, "Warning"}},
        {ui->checkBox_pushCategory_error, {2, "Error"}}};

    auto mapping = checkboxMapping.value(senderCheckbox);
    int filterType = mapping.first;

    QStringList selectedTypes;

    for (auto it = checkboxMapping.begin(); it != checkboxMapping.end(); ++it) {
        if (it.value().first == filterType && it.key()->isChecked()) {
            selectedTypes << it.value().second;
        }
    }

    if (selectedTypes.isEmpty()) {
        emit filterChanged("", filterType, false);
    } else {
        QString pattern = selectedTypes.join("|");
        emit filterChanged(pattern, filterType, true);
    }
}
