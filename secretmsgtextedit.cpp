#include <QKeyEvent>
#include "secretmsgtextedit.h"

secretMsgTextEdit::secretMsgTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{

}

void secretMsgTextEdit::keyPressEvent(QKeyEvent *event)
{
    QPlainTextEdit::keyPressEvent(event);
}
