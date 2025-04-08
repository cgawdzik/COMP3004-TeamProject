#include "firstrowhighlightdelegate.h"

void FirstRowHighlightDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const {
    QStyleOptionViewItem opt = option;
    QStyledItemDelegate::initStyleOption(&opt, index);

    // First, paint the standard cell
    QStyledItemDelegate::paint(painter, opt, index);

    // Now, if it's the first row, draw a thicker border
    if (index.row() == 0) {
        painter->save();
        QPen thickPen(Qt::black, 3); // Change color/thickness as desired
        painter->setPen(thickPen);
        // Draw a rectangle around the entire cell
        painter->drawRect(opt.rect.adjusted(1, 1, -1, -1));
        painter->restore();
    }
}
