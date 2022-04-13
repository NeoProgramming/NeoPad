#pragma once

#include <qglobal.h>
#include <QByteArray>
#include <openssl/aes.h>

// based on: https://github.com/alexeylysenko/CryptFileDevice
// one object can either encrypt or decrypt


bool isEncrypted(const QString &path);
bool decryptFile(const QString &path, const QString &password, QByteArray &data);
bool encryptFile(const QString &path, const QString &password, const QByteArray &data);
bool recryptFile(const QString &path, const QString &oldPassword, const QString &newPassword);

class Cryptor
{
	Q_DISABLE_COPY(Cryptor)
public:
	enum class AesKeyLength : quint32 {
		kAesKeyLength128,
		kAesKeyLength192,
		kAesKeyLength256
	};
public:
	Cryptor();
	Cryptor(const QByteArray &password, const QByteArray &salt);

private:
	struct CtrState {
		unsigned char ivec[AES_BLOCK_SIZE];
		unsigned int num;
		unsigned char ecount[AES_BLOCK_SIZE];
	};
public:
	bool initCipher();
	void encrypt(const QByteArray &plainText, QByteArray &cipherText);
	void decrypt(const QByteArray &cipherText, QByteArray &plainText);

	void setPassword(const QByteArray &password);
	void setSalt(const QByteArray &salt);
	void setKeyLength(AesKeyLength keyLength);
	void setNumRounds(int numRounds);
private:
	QByteArray m_password;
	QByteArray m_salt;
	AesKeyLength m_aesKeyLength = AesKeyLength::kAesKeyLength256;
	int m_numRounds = 5;

	CtrState m_ctrState = {};
	AES_KEY m_aesKey = {};
};

