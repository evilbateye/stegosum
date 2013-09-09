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
