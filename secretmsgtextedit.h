#ifndef SECRETMSGTEXTEDIT_H
#define SECRETMSGTEXTEDIT_H

#include <QWidget>
#include <QPlainTextEdit>
#include "mainwindow.h"


class secretMsgTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit secretMsgTextEdit(QWidget *parent = 0);

protected:
    virtual void keyPressEvent( QKeyEvent *event);

signals:

public slots:

private:

};

#endif // SECRETMSGTEXTEDIT_H
