#include "utils.hpp"
#include <QtCrypto/QtCrypto>

Utils::Utils()
{
}

qint32 Utils::pixelsNeeded(qint32 bits, quint8 selectedColors, quint8 toHowManyBits)
{
    qint32 pixels = bits / (selectedColors * toHowManyBits);
    if (bits % (selectedColors * toHowManyBits)) pixels++;
    return pixels;
}

QByteArray Utils::encrypt(const QByteArray & msg, const QString & password)
{
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding);

    QCA::SymmetricKey key = QCA::SymmetricKey(password.toAscii());
    QCA::InitializationVector iv = QCA::InitializationVector(16);

    cipher.setup(QCA::Encode, key, iv);

    QByteArray ret = iv.toByteArray();
    ret += cipher.process(msg).toByteArray();
    return ret;
}

QByteArray Utils::decrypt(const QByteArray & msg, const QString & password)
{
    if (password.isEmpty()) return QByteArray();

    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding);

    QCA::SymmetricKey key = QCA::SymmetricKey(password.toAscii());

    QCA::InitializationVector iv(msg.left(16));

    cipher.setup(QCA::Decode, key, iv);

    return cipher.process(msg.mid(16)).toByteArray();
}
