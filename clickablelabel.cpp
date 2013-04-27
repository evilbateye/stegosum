#include "clickablelabel.hpp"
#include <QDebug>

ClickableLabel::ClickableLabel(QWidget * parent) :
    QLabel(parent),
    mPoint(0, 0),
    mDiff(0, 0)
{
}

void ClickableLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);

    emit clicked();
}

void ClickableLabel::wheelEvent(QWheelEvent * e)
{
    emit scrolled(e->delta());
}

void ClickableLabel::mousePressEvent(QMouseEvent * e)
{
    if (e->buttons() == Qt::LeftButton) {
        mPoint = e->pos();
    }
}

void ClickableLabel::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() == Qt::LeftButton) {
        mDiff = (mPoint - e->pos()) + mDiff;
        emit mousePressedAndMoved(mDiff);
        mPoint = e->pos();
    }
}
