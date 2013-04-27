#ifndef CLICKABLELABEL_HPP
#define CLICKABLELABEL_HPP

#include <QLabel>
#include <QWheelEvent>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    ClickableLabel(QWidget *parent);

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
};

#endif // CLICKABLELABEL_HPP
