//#include "qubit_mvc_framework/utilities/secure.h"
//#include "qubit_mvc_framework/utilities/common_p.h"
//
//#include <QNetworkAccessManager>
//#include <QNetworkReply>
//#include <QJsonObject>
//#include <QJsonDocument>
//#include <QJsonArray>
//#include <QReadWriteLock>
//#include <QEventLoop>
//#include <QTimer>
//
//#include <iostream>
//#include <openssl/rsa.h>
//#include <openssl/engine.h>
//#include <openssl/pem.h>
//#include <openssl/conf.h>
//#include <openssl/evp.h>
//#include <openssl/err.h>
//#include <openssl/aes.h>
//#include <openssl/rand.h>
//#include <openssl/ec.h>
//#include <openssl/ecdsa.h>
//
//
//#include <openssl/core_names.h>
//#include <openssl/ec.h>
//#include <openssl/evp.h>
//#include <openssl/kdf.h>
//#include <openssl/sha.h>
//#include <openssl/core.h>
//
//
//#include <openssl/core_names.h>
//#include <openssl/core.h>
//#include <openssl/core_dispatch.h>
//#include <openssl/ec.h>
//// #include <openssl/ec_key.h>
//#include <openssl/evp.h>
//
//
//Secure::Secure(const unsigned int RSA_KEY_BITS){
//    pctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);
//    EVP_PKEY_keygen_init(pctx);
//
//    if(private_key) EVP_PKEY_free(private_key);
//    EVP_PKEY_generate(pctx, &private_key);
//    unsigned int primes = 3;
//    unsigned int bits = RSA_KEY_BITS;
//    OSSL_PARAM params[3];
//
//    params[0] = OSSL_PARAM_construct_uint("bits", &bits);
//    params[1] = OSSL_PARAM_construct_uint("primes", &primes);
//    params[2] = OSSL_PARAM_construct_end();
//    EVP_PKEY_CTX_set_params(pctx, params);
//
//    EVP_PKEY_generate(pctx, &private_key);
//    evp_puk = generatePubKeys();
//
//
//    // BIO *bio_out = BIO_new_fp(stdout, BIO_NOCLOSE);
//    // std::cout<<"\nPrinting Server Private Key\n";
//    // EVP_PKEY_print_private(bio_out, private_key, 0, NULL);
//    // std::cout<<"\nPrinting Session Verification/Public Key\n";
//    // EVP_PKEY_print_public(bio_out, private_key, 0, NULL);
//    // BIO_free(bio_out);
//}
//
//Secure::~Secure(){
//    EVP_PKEY_CTX_free(pctx);
//    ONLY_DEBUG()<<this<<"is destory";
//}
//
//const Integrity Secure::doIntegrityCheck(QByteArray token, QByteArray jwe_decryption_key,EVP_PKEY *ec_key){
//    QList<QByteArray> jweParts = token.split('.');
//    if(jweParts.size() < 5){
//        return Integrity();
//    }
//
//    QByteArray encoded_header = jweParts[0];
//    QByteArray header = QByteArray::fromBase64(encoded_header,QByteArray::Base64UrlEncoding);
//    auto decoded_header_object = QJsonDocument::fromJson(header).object();
//    auto alg=decoded_header_object.value("alg").toString();
//    auto enc=decoded_header_object.value("enc").toString();
//    if(alg=="A256KW"){
//        if(enc=="A256GCM"){
//            QByteArray encoded_jwe_enc_key = jweParts[1];
//            QByteArray jwe_enc_key = QByteArray::fromBase64(encoded_jwe_enc_key,QByteArray::Base64UrlEncoding);
//
//            int key_bits_len=256;
//            QByteArray jwe_key(key_bits_len/8,Qt::Uninitialized);
//
//            if(!keyUnwrapping(jwe_decryption_key,jwe_enc_key,jwe_key)){
//                return Integrity();
//            }
//
//            QByteArray encoded_jwe_iv = jweParts[2];
//            QByteArray jwe_iv = QByteArray::fromBase64(encoded_jwe_iv,QByteArray::Base64UrlEncoding);
//
//            QByteArray encoded_jwe_cipherText = jweParts[3];
//            QByteArray jwe_cipherText = QByteArray::fromBase64(encoded_jwe_cipherText,QByteArray::Base64UrlEncoding);
//
//            QByteArray encoded_jwe_tag = jweParts[4];
//            QByteArray jwe_tag = QByteArray::fromBase64(encoded_jwe_tag,QByteArray::Base64UrlEncoding);
//
//            QByteArray aad;
//
//            QByteArray jws(jwe_cipherText.size() + AES_BLOCK_SIZE,Qt::Uninitialized);
//
//            if(!gcm_decrypt(jwe_cipherText,aad,jwe_tag,jwe_key,jwe_iv,jws)){
//                return Integrity();
//            }
//            QList<QByteArray> jwsParts = jws.split('.');
//            if(jwsParts.size() < 3){
//                return Integrity();
//            }
//
//            QByteArray jws_encoded_header = jwsParts[0];
//            QByteArray jws_header = QByteArray::fromBase64(jws_encoded_header,QByteArray::Base64UrlEncoding);
//            Q_UNUSED(jws_header)
//
//            QByteArray jws_encoded_payload = jwsParts[1];
//            QByteArray jws_payload = QByteArray::fromBase64(jws_encoded_payload,QByteArray::Base64UrlEncoding);
//
//            QByteArray jws_encoded_signature = jwsParts[2];
//            QByteArray jws_signature  = QByteArray::fromBase64(jws_encoded_signature,QByteArray::Base64UrlEncoding);
//
//            Q_UNUSED(jwsParts);Q_UNUSED(jws_signature);Q_UNUSED(jws_payload);Q_UNUSED(ec_key);
//            Q_ASSERT_X(false,"verifySignature(jwsParts[0]+\".\"+jwsParts[1], jws_signature, ec_key)","verifySignature(jwsParts[0]+\".\"+jwsParts[1], jws_signature, ec_key)");
//            // if(verifySignature(jwsParts[0]+"."+jwsParts[1], jws_signature, ec_key)){
//            return Integrity(jws_payload,true);
//            // }
//        }
//    }
//    return Integrity();
//}
//
//bool setPadding(EVP_MD_CTX *ctx,EVP_PKEY *key){
//    // Set padding (optional)
//    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new(key, nullptr);
//    if (pctx == nullptr) {
//        qDebug("Cannot allocate PKEY CTX");
//        ERR_print_errors_fp(stdout);
//        EVP_MD_CTX_free(ctx);
//        return false;
//    }
//    if (1 != EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PADDING)) {
//        qDebug("EVP_PKEY_CTX_set_rsa_padding failure");
//        ERR_print_errors_fp(stdout);
//        EVP_MD_CTX_free(ctx);
//        EVP_PKEY_CTX_free(pctx);
//        return false;
//    }
//    EVP_PKEY_CTX_free(pctx);
//    return true;
//}
//
//bool Secure::signBytes(QByteArray &token, const QByteArray &body_ba){
//    return signBytes(token,body_ba,private_key);
//}
//
//bool Secure::signBytes(QByteArray &token, const QByteArray &body_ba, EVP_PKEY *private_key)
//{
//    bool success=false;
//    QByteArray  sig(EVP_PKEY_size(private_key),Qt::Uninitialized);
//    size_t sig_len = sig.size();
//    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
//    if (ctx == nullptr){
//        qDebug("cannot allocate MD CTX");
//        goto FINISH;
//    }
//
//    if (1 != EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, private_key)){
//        ERR_print_errors_fp(stdout);
//        goto FINISH;
//    }
//
//    if (1 != EVP_DigestSignUpdate(ctx,(uchar *)body_ba.data(), body_ba.length())){
//        ERR_print_errors_fp(stdout);
//        goto FINISH;
//    }
//
//    if (1 != EVP_DigestSignFinal(ctx,(uchar *)sig.data(), &sig_len)){
//        qDebug("EVP_DigestSignFinal failure");
//        ERR_print_errors_fp(stdout);
//        goto FINISH;
//    }
//
//    sig.resize(sig_len);
//    token= body_ba+"."+sig.toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals);
//    success=true;
//FINISH:
//    EVP_MD_CTX_free(ctx);
//    return success;
//}
//
//bool Secure::verifySignature(const QByteArray &token){
//    return verifySignature(token,evp_puk);
//}
//
//bool Secure::verifySignature(const QByteArray &token,EVP_PKEY *public_key)
//{
//    QByteArray signature,cc;
//    QByteArray decodedSignature;
//    EVP_MD_CTX *ctx = nullptr;
//    bool success = false;
//
//    // Extract signature from the token
//    int dotIndex = token.lastIndexOf('.');
//    if (dotIndex == -1) {
//        qDebug("Invalid token format");
//        goto FINISH;
//    }
//    signature = token.mid(dotIndex + 1);
//    decodedSignature = QByteArray::fromBase64(signature, QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals);
//    cc = QByteArray::fromRawData(token.constData(), dotIndex);
//    // Initialize digest context
//    ctx = EVP_MD_CTX_new();
//    if (ctx == nullptr) {
//        qDebug("Cannot allocate MD CTX");
//        goto FINISH;
//    }
//
//    // Initialize verification operation
//    if (1 != EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, public_key)) {
//        ERR_print_errors_fp(stdout);
//        goto FINISH;
//    }
//
//    // Add data to be verified
//    if (1 != EVP_DigestVerifyUpdate(ctx, (const uchar *)token.constData(), dotIndex)) {
//        ERR_print_errors_fp(stdout);
//        goto FINISH;
//    }
//
//    // Perform verification
//    if (1 != EVP_DigestVerifyFinal(ctx, (const uchar *)decodedSignature.constData(), decodedSignature.length())) {
//        // qDebug("Signature verification failed");
//        ERR_print_errors_fp(stdout);
//        goto FINISH;
//    }
//
//    // Verification successful
//    success = true;
//FINISH:
//    EVP_MD_CTX_free(ctx);
//    return success;
//}
//
//bool Secure::rsa_decrypt(EVP_PKEY *private_key, const QByteArray &in, QByteArray &out, int padding){
//
//    bool success = false;
//    size_t encrypted_data_length = in.size();
//    size_t decrypted_data_length = encrypted_data_length;
//    out.resize(encrypted_data_length);
//
//    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(private_key, NULL);
//    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
//        goto FINISH;
//    }
//
//    if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0) {
//        goto FINISH;
//    }
//
//    if (EVP_PKEY_decrypt(ctx, (uchar *)out.data(), &decrypted_data_length, (uchar *)in.data(), encrypted_data_length) <= 0) {
//        qWarning() << "Could not decrypt: " << ERR_error_string(ERR_get_error(),NULL);
//        goto FINISH;
//    }
//
//    out.resize(decrypted_data_length);
//    success = true;
//FINISH:
//    EVP_PKEY_CTX_free(ctx);
//    return success;
//}
//
//bool Secure::gcm_decrypt(const QByteArray &in, const QByteArray &aad, const QByteArray &tag, const QByteArray &key, const QByteArray &iv, QByteArray &out){
//
//    int len;
//    int p_len = 0, f_len = 0;
//    bool success =false;
//    EVP_CIPHER_CTX  *ctx = EVP_CIPHER_CTX_new();
//
//    /* Initialise the decryption operation. */
//    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)){
//        qCritical() <<"Function :"<<__FUNCTION__<<"Line :"<<__LINE__<< ERR_error_string(ERR_get_error(),NULL);
//        goto FINISH;
//    }
//
//    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL)){
//        qCritical() <<"Function :"<<__FUNCTION__<<"Line :"<<__LINE__<< ERR_error_string(ERR_get_error(),NULL);
//        goto FINISH;
//    }
//
//
//    if(!EVP_DecryptInit_ex(ctx, NULL, NULL,  (const uchar *)key.constData(),(const uchar *) iv.constData())){
//        qCritical() <<"Function :"<<__FUNCTION__<<"Line :"<<__LINE__<< ERR_error_string(ERR_get_error(),NULL);
//        goto FINISH;
//    }
//
//    //    //Provide any AAD data. This can be called zero or more times as required
//    if(!EVP_DecryptUpdate(ctx, (uchar *)out.data(), &len, (uchar *)aad.data(), aad.size())){
//        qCritical() <<"Function :"<<__FUNCTION__<<"Line :"<<__LINE__<< ERR_error_string(ERR_get_error(),NULL);
//        goto FINISH;
//    }
//
//    //Provide the message to be decrypted, and obtain the plaintext output. EVP_DecryptUpdate can be called multiple times if necessary
//    if(!EVP_DecryptUpdate(ctx, (uchar *)out.data(), &len, (const uchar *)in.constData(), in.size())){
//        qCritical() <<"Function :"<<__FUNCTION__<<"Line :"<<__LINE__<<ERR_error_string(ERR_get_error(),NULL);
//        goto FINISH;
//    }
//    p_len = len ;
//    /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
//    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (uchar *)tag.constData())){
//        qCritical() <<"Function :"<<__FUNCTION__<<"Line :"<<__LINE__<< ERR_error_string(ERR_get_error(),NULL);
//        goto FINISH;
//    }
//
//    if(!EVP_DecryptFinal_ex(ctx,(uchar *)out.data()+p_len,&f_len)){
//        //            qCritical() <<"Function :"<<__FUNCTION__<<"Line :"<<__LINE__<<"EVP_DecryptFinal_ex() failed " <<  ERR_error_string(ERR_get_error(), NULL);
//        //        goto FINISH;
//    }
//
//    out.resize(p_len + f_len);
//    success = true;
//
//FINISH:
//    EVP_CIPHER_CTX_free(ctx);
//    return success;
//}
//
//bool Secure::keyUnwrapping(const QByteArray &aes_key, const QByteArray &wrapped_key, QByteArray &out){
//
//    bool success =false;
//    int p_len = 0, f_len = 0;
//    EVP_CIPHER_CTX  *ctx = EVP_CIPHER_CTX_new();
//
//    EVP_CIPHER_CTX_set_flags(ctx, EVP_CIPHER_CTX_FLAG_WRAP_ALLOW);
//
//    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_wrap(), NULL, (uchar *)aes_key.constData(),NULL)) {
//        ONLY_DEBUG() << "EVP_CipherInit_ex() failed" << ERR_error_string(ERR_get_error(), NULL);
//        goto FINISH;
//    }
//
//    out.resize(p_len + AES_BLOCK_SIZE);
//    if(!EVP_DecryptUpdate(ctx, (uchar *)out.data(), &p_len, (const uchar *)wrapped_key.constData(), wrapped_key.size())){
//        ONLY_DEBUG() << "EVP_DecryptUpdate() failed " <<  ERR_error_string(ERR_get_error(), NULL);
//        goto FINISH;
//    }
//
//    if(!EVP_DecryptFinal_ex(ctx,(uchar *)out.data()+p_len,&f_len)){
//        ONLY_DEBUG()<< "EVP_DecryptFinal_ex() failed " <<  ERR_error_string(ERR_get_error(), NULL);
//        goto FINISH;
//    }
//    out.resize(p_len + f_len);
//    success = true;
//FINISH:
//    EVP_CIPHER_CTX_free(ctx);
//
//    return success;
//
//}
//
//bool Secure::isValid(){
//    return  (private_key != nullptr);
//}
//
//EVP_PKEY *Secure::generatePubKeys(){
//    EVP_PKEY* public_key = nullptr;
//    RSA* rsa_private = nullptr;
//    RSA* rsa_public = nullptr;
//    if (!private_key) {
//        goto FINISH;
//    }
//
//    rsa_private = EVP_PKEY_get1_RSA(private_key);
//    if (!rsa_private) {
//        goto FINISH;
//    }
//
//    rsa_public = RSAPublicKey_dup(rsa_private);
//    if (!rsa_public) {
//        goto FINISH;
//    }
//
//    public_key = EVP_PKEY_new();
//    if (!public_key) {
//        goto FINISH;
//    }
//
//    if (EVP_PKEY_assign_RSA(public_key, rsa_public) != 1) {
//        goto FINISH;
//    }
//
//FINISH:
//    if(rsa_private)
//        RSA_free(rsa_private);
//    if(rsa_public && !public_key)
//        RSA_free(rsa_public);
//    return public_key;
//}
//
