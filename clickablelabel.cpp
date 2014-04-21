#include <QDebug>
#include "clickablelabel.hpp"

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

        if (mHSB->value() == mHSB->minimum() && mDiff.x() < 0) mDiff.setX(0);
        if (mHSB->value() == mHSB->maximum() && mDiff.x() > 0) mDiff.setX(0);
        if (mVSB->value() == mVSB->minimum() && mDiff.y() < 0) mDiff.setY(0);
        if (mVSB->value() == mVSB->maximum() && mDiff.y() > 0) mDiff.setY(0);

        emit mousePressedAndMoved(mDiff);
        mPoint = e->pos();
    }
}
