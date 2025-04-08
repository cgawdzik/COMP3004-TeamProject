#ifndef FIRSTROWHIGHLIGHTDELEGATE_H
#define FIRSTROWHIGHLIGHTDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>

class FirstRowHighlightDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate; // use parent constructor

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // FIRSTROWHIGHLIGHTDELEGATE_H
