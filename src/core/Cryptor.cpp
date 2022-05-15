#include "Cryptor.h"
#include <openssl/evp.h>
#include <openssl/modes.h>
#include <qendian.h>
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>

static int const kSaltMaxLength = 8;
static int const MIN_KEY_LENGTH = 18;
static char SIG[] = "NEO\x01\xF3\x08@\x01";
static char SALT[] = "AbsoluteHackGame";

bool isEncrypted(const QString &path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) 
		return false;
	QByteArray sig = file.read(8);
	file.close();
	return sig == SIG;
}

bool decryptFile(const QString &path, const QString &password, QByteArray &plainText)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	QByteArray sig = file.read(8);
	if (sig != SIG)
		return false;
	QByteArray echash = file.read(32);
	QByteArray cypher = file.readAll();

	Cryptor cr(password.toLocal8Bit(), SALT);
	if (!cr.initCipher())
		return false;
	QByteArray hash;
	cr.decrypt(echash, hash);
	cr.decrypt(cypher, plainText);
	if (hash != QCryptographicHash::hash(plainText, QCryptographicHash::Sha3_256))
		return false;
	return true;
}

bool encryptFile(const QString &path, const QString &password, const QByteArray &plainText)
{
	Cryptor cr(password.toLocal8Bit(), SALT);
	if (!cr.initCipher())
		return false;
		
	QByteArray hash = QCryptographicHash::hash(plainText, QCryptographicHash::Sha3_256);
	QByteArray echash;
	cr.encrypt(hash, echash);
	QByteArray cypher;
	cr.encrypt(plainText, cypher);

	// test
	Cryptor tcr(password.toLocal8Bit(), SALT);
	if (!tcr.initCipher())
		return false;
	QByteArray test;
	tcr.decrypt(echash, test);
	if (test != hash)
		return false;
	tcr.decrypt(cypher, test);
	if (test != plainText)
		return false;
	
	// dump to file
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly))
		return false;
	file.write(SIG, 8);
	file.write(echash);
	file.write(cypher);
	file.close();

	return true;
}

bool recryptFile(const QString &path, const QString &oldPassword, const QString &newPassword)
{
	QByteArray plain;
	if (!QFileInfo::exists(path) || !QFileInfo(path).isFile())
		return false;

	// decrypt
	if (isEncrypted(path)) {
		if (!decryptFile(path, oldPassword, plain))
			return false;
	}
	else {
		if (newPassword.isEmpty())
			return true;
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly))
			return false;
		plain = file.readAll();
		file.close();
	}

	// encrypt
	if (newPassword.isEmpty()) {
		QFile file(path);
		if (!file.open(QIODevice::WriteOnly))
			return false;
		file.write(plain);
		file.close();
	}
	else {
		if (!encryptFile(path, newPassword, plain))
			return false;
	}

	return true;
}

Cryptor::Cryptor()
{}

Cryptor::Cryptor(const QByteArray &password, const QByteArray &salt)
{
	setPassword(password);
	setSalt(salt);
}

void Cryptor::setPassword(const QByteArray &password) 
{
	m_password = password;
}

void Cryptor::setSalt(const QByteArray &salt) 
{
	if (salt.isEmpty()) {
		m_salt.clear();
		return;
	}

	m_salt = salt.leftJustified(kSaltMaxLength, '\0', true);
}

void Cryptor::setKeyLength(AesKeyLength keyLength) 
{
	m_aesKeyLength = keyLength;
}

void Cryptor::setNumRounds(int numRounds) 
{ 
	m_numRounds = numRounds; 
}


bool Cryptor::initCipher() 
{
	// create cipher object
	const EVP_CIPHER *cipher = EVP_enc_null();
	if (m_aesKeyLength == AesKeyLength::kAesKeyLength128)
		cipher = EVP_aes_128_ctr();
	else if (m_aesKeyLength == AesKeyLength::kAesKeyLength192)
		cipher = EVP_aes_192_ctr();
	else if (m_aesKeyLength == AesKeyLength::kAesKeyLength256)
		cipher = EVP_aes_256_ctr();
	else
		Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown value of AesKeyLength");

	// create new context
	auto ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_init(ctx);
	EVP_EncryptInit_ex(ctx, cipher, nullptr, nullptr, nullptr);
	int keyLength = EVP_CIPHER_CTX_key_length(ctx);
	int ivLength = EVP_CIPHER_CTX_iv_length(ctx);

	QVector<unsigned char> key(keyLength);
	QVector<unsigned char> iv(ivLength);

	int ok = EVP_BytesToKey(
		cipher, EVP_sha256(),
		m_salt.isEmpty() ? nullptr : reinterpret_cast<unsigned char *>(m_salt.data()),
		reinterpret_cast<unsigned char *>(m_password.data()), m_password.length(),
		m_numRounds, key.data(), iv.data());

	EVP_CIPHER_CTX_free(ctx);

	if (ok == 0) return false;

	int res = AES_set_encrypt_key(key.data(), keyLength * 8, &m_aesKey);
	if (res != 0) return false;

	// initCtr 
	m_ctrState.num = 0;
	memset(m_ctrState.ecount, 0, sizeof(m_ctrState.ecount));
	int sizeOfIv = sizeof(m_ctrState.ivec) - sizeof(qint64);
	qint64 newCount = 0;
	memcpy(m_ctrState.ivec + sizeOfIv, &newCount, sizeof(newCount));
	memcpy(m_ctrState.ivec, iv.data(), sizeOfIv);

	return true;
}

void Cryptor::encrypt(const QByteArray &plainText, QByteArray &cipherText)
{
	qint64 len = plainText.size();
	cipherText.resize(len);

	// block encryption
	qint64 processLen = 0;
	do {
		int maxCipherLen = len > std::numeric_limits<int>::max()
			? std::numeric_limits<int>::max()
			: len;
		CRYPTO_ctr128_encrypt(
			reinterpret_cast<const unsigned char *>(plainText.constData()) + processLen,
			reinterpret_cast<unsigned char *>(cipherText.data()) + processLen, 
			maxCipherLen, &m_aesKey, m_ctrState.ivec,
			m_ctrState.ecount, &m_ctrState.num, (block128_f)AES_encrypt);

		processLen += maxCipherLen;
		len -= maxCipherLen;
	} while (len > 0);
}

void Cryptor::decrypt(const QByteArray &cipherText, QByteArray &plainText)
{
	qint64 len = cipherText.size();
	plainText.resize(len);

	qint64 processLen = 0;
	do {
		int maxPlainLen = len > std::numeric_limits<int>::max()
			? std::numeric_limits<int>::max()
			: len;
		CRYPTO_ctr128_encrypt(
			reinterpret_cast<const unsigned char *>(cipherText.constData()) + processLen,
			reinterpret_cast<unsigned char *>(plainText.data()) + processLen, 
			maxPlainLen, &m_aesKey, m_ctrState.ivec,
			m_ctrState.ecount, &m_ctrState.num, (block128_f)AES_encrypt);

		processLen += maxPlainLen;
		len -= maxPlainLen;
	} while (len > 0);
}

