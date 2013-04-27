#include "secretmsgtextedit.h"
#include <QKeyEvent>

secretMsgTextEdit::secretMsgTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{

}

void secretMsgTextEdit::keyPressEvent(QKeyEvent *event)
{
    QPlainTextEdit::keyPressEvent(event);
}
