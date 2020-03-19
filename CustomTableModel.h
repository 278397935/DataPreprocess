#ifndef CUSTOMTABLEMODEL_H
#define CUSTOMTABLEMODEL_H

#include <QSqlTableModel>

#include <QColor>
#include <QFont>

class CustomTableModel : public QSqlTableModel
{

public:
    CustomTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    ~CustomTableModel();

    QVariant data(const QModelIndex &idx, int role) const;

signals:

public slots:
};

#endif // CUSTOMTABLEMODEL_H
