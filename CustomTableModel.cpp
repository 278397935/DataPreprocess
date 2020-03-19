#include "CustomTableModel.h"

CustomTableModel::CustomTableModel(QObject *parent, QSqlDatabase db) : QSqlTableModel(parent,db)
{

}

CustomTableModel::~CustomTableModel()
{

}

QVariant CustomTableModel::data(const QModelIndex &idx, int role) const
{
    QVariant value  = QSqlTableModel::data(idx,role);

    if(Qt::TextAlignmentRole == role)
    {
        value   = int(Qt::AlignRight | Qt::AlignBottom);
        return value;
    }

    if(Qt::FontRole == role)
    {
        if(idx.column() >= 5 && idx.column() <= 9)
        {
            QFont font;
            font.setBold(true);
            return QVariant(font);
        }
    }
    return value;
}
