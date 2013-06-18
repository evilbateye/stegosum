#ifndef CLICKABLELABEL_HPP
#define CLICKABLELABEL_HPP

#include <QLabel>
#include <QWheelEvent>
#include <QScrollBar>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    ClickableLabel(QWidget *parent);
    void setHSlider(QScrollBar * sb) { mHSB = sb; }
    void setVSlider(QScrollBar * sb) { mVSB = sb; }

signals:
    void clicked();
    void scrolled(int);
    void mousePressedAndMoved(QPoint);

protected:
    void mouseDoubleClickEvent(QMouseEvent * e);
    void wheelEvent(QWheelEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);

private:
    QPoint mPoint;
    QPoint mDiff;
    QScrollBar * mHSB;
    QScrollBar * mVSB;
};

#endif // CLICKABLELABEL_HPP
