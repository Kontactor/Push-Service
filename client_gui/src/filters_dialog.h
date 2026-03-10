#ifndef FILTERS_DIALOG_H
#define FILTERS_DIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QSortFilterProxyModel>

#include "ui_filters_dialog.h"

namespace Ui {
class FiltersDialog;
}

class MultiFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit MultiFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

    void setColumnFilter(int column, const QString& filterText, bool enabled) {
        if (enabled && !filterText.isEmpty()) {
            filters_[column] = filterText.split('|', Qt::SkipEmptyParts);
        } else {
            filters_.remove(column);
        }
        invalidateFilter();
    }

    void clearFilters() {
        filters_.clear();
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override {
        if (filters_.isEmpty())
            return true;

        for (auto it = filters_.constBegin(); it != filters_.constEnd(); ++it) {
            int column = it.key();
            const QStringList& filterList = it.value();

            QModelIndex index = sourceModel()->index(sourceRow, column, sourceParent);
            QString data = sourceModel()->data(index).toString();

            bool matchFound = false;
            for (const QString& filterWord : filterList) {
                if (data.contains(filterWord, Qt::CaseInsensitive)) {
                    matchFound = true;
                    break;
                }
            }

            if (!matchFound)
                return false;
        }

        return true;
    }

private:
    QMap<int, QStringList> filters_;
};

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
