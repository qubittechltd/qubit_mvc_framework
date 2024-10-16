#ifndef SECURE_H
#define SECURE_H

#include <QDebug>
#include <QByteArray>
#include <QElapsedTimer>
#include <QVariant>
#include <QJsonDocument>
#include <QSslCertificate>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QBuffer>
#include <QFile>
#include <QSslKey>

#include <openssl/evp.h>
#include <openssl/rsa.h>

#include "integrity.h"

class Secure{

public:
    enum Purpose{
        Core,
        Peer
    };

    static Secure & instance(const unsigned int RSA_KEY_BITS=1024){
        static Secure s(RSA_KEY_BITS);
        return s;
    }

    Secure(const unsigned int RSA_KEY_BITS=1024);

    Secure(Secure const&) = delete;

    Secure& operator=(Secure const&) = delete;

    ~Secure();

    const Integrity doIntegrityCheck(QByteArray token, QByteArray jwe_decryption_key,EVP_PKEY *ec_key);

    bool signBytes(QByteArray &token,const QByteArray &body_ba);

    static bool signBytes(QByteArray &token,const QByteArray &body_ba,EVP_PKEY * private_key);

    bool verifySignature(const QByteArray &token);

    bool verifySignature(const QByteArray &token, EVP_PKEY *public_key);

    static bool rsa_decrypt(EVP_PKEY * private_key,const QByteArray &in , QByteArray &out, int padding=RSA_NO_PADDING);

    static bool gcm_decrypt(const QByteArray & in,const QByteArray & aad,const QByteArray & tag,const QByteArray & key,const QByteArray & iv,QByteArray & out);

    static bool keyUnwrapping(const QByteArray & aes_key,const QByteArray &wrapped_key, QByteArray &out);

    bool  isValid();

    EVP_PKEY * generatePubKeys();
    
    static bool equalHash(const QString&,const QString&){ return true; };
    static bool equalHash(const QByteArray&,QByteArray&){ return true; };
private:
    EVP_PKEY *private_key = nullptr;
    EVP_PKEY *evp_puk = nullptr;//for self verification
    EVP_PKEY_CTX *pctx = nullptr;
};

#endif // SECURE_H
