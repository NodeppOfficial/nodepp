/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_CRYPTO
#define NODEPP_CRYPTO
#define OPENSSL_API_COMPAT 0x10100000L

/*────────────────────────────────────────────────────────────────────────────*/

#define CRYPTO_BASE64 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#include "encoder.h"

/*────────────────────────────────────────────────────────────────────────────*/

#include <openssl/ripemd.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/des.h>

/*────────────────────────────────────────────────────────────────────────────*/

#include <openssl/dh.h>
#include <openssl/dsa.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509.h>

/*────────────────────────────────────────────────────────────────────────────*/

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

class hash_t {
protected:

    struct NODE {
        EVP_MD_CTX* ctx = nullptr;
        ptr_t<uchar> bff;
        uint length= 0;
        bool state = 0;
    };  ptr_t<NODE> obj = new NODE();

    string_t hex() const noexcept { 
        free(); return { (char*) &obj->bff, obj->length }; 
    }

public:

    template< class T >
    hash_t( const T& type, ulong length ) : obj( new NODE() ) { 
         
        obj->bff   = ptr_t<uchar>( length );
        obj->ctx   = EVP_MD_CTX_new();
        obj->state = 1;
        if ( !obj->ctx || !EVP_DigestInit_ex( obj->ctx, type, NULL ) )
           { throw except_t("can't initializate hash_t"); }
    }

    virtual ~hash_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_MD_CTX* get_fd() const noexcept { return obj->ctx; }

    void update( string_t msg ) const noexcept { 
        if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ 
            string_t tmp = msg.splice( 0, obj->bff.size()/2 );
            EVP_DigestUpdate( obj->ctx, (uchar*) tmp.data(), tmp.size() );
        }
    }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        EVP_DigestFinal_ex( obj->ctx, &obj->bff, &obj->length );
        EVP_MD_CTX_free( obj->ctx ); 
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    string_t get() const noexcept { 
        return encoder::buffer::buff2hex( this->hex() );
    }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class hmac_t {
protected:

    struct NODE {
        HMAC_CTX* ctx = nullptr;
        ptr_t<uchar> bff; 
        uint length= 0;
        bool state = 0;
    };  ptr_t<NODE> obj;

    string_t hex() const noexcept { 
        free(); return { (char*) &obj->bff, obj->length }; 
    }

public:

    template< class T >
    hmac_t( const string_t& key, const T& type, ulong length ) 
    :  obj( new NODE() ) { if( key.empty() ){ return; }
        obj->bff   = ptr_t<uchar>( length ); 
        obj->ctx   = HMAC_CTX_new(); 
        obj->state = 1;
        if ( !obj->ctx || !HMAC_Init_ex( obj->ctx, key.data(), key.size(), type, nullptr ) )
           { throw except_t("can't initializate hmac_t"); }
    }
    
    virtual ~hmac_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    HMAC_CTX* get_fd() const noexcept { return obj->ctx; }

    void update( string_t msg ) const noexcept { 
        if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ 
            string_t tmp = msg.splice( 0, obj->bff.size()/2 );
            HMAC_Update( obj->ctx, (uchar*) tmp.data(), tmp.size() ); 
        }
    }

    void free() const noexcept {
        if( obj->state == 0 ){ return; } obj->state = 0;
        HMAC_Final( obj->ctx, &obj->bff, &obj->length ); 
        HMAC_CTX_free( obj->ctx ); 
    }

    string_t get() const noexcept { 
        return encoder::buffer::buff2hex( this->hex() );
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class xor_t {
protected:

    struct CTX {
        string_t key;
        ulong    pos;
    };

    struct NODE {
        ptr_t<CTX>  ctx;
        string_t   buff;
        bool    state=0;
    };  ptr_t<NODE> obj;

public:

    event_t<>         onClose;
    event_t<string_t> onData;

    xor_t( const string_t& key ) noexcept: obj( new NODE() ) {
        if( key.empty() ){ return; } obj->state = 1;

        CTX item1; //memset( &item1, 0, sizeof(CTX) );
            item1.key = key; item1.pos = 0;

        obj->ctx = ptr_t<CTX> ({ item1 });
    }

    xor_t() noexcept : obj( new NODE() ) { obj->state = 0; }
    
    virtual ~xor_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    void update( string_t msg ) const noexcept { if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ string_t tmp = msg.splice( 0, CHUNK_SIZE );
            forEach( y, obj->ctx ){ forEach( x, tmp ){ 
                x = x ^ y.key[ y.pos % y.key.size() ]; ++y.pos; 
            }} if ( tmp.empty() )     { return; }
             elif ( onData.empty() )  { obj->buff +=tmp; }
             else { onData.emit(tmp); }
        }
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    string_t get() const noexcept { free(); return obj->buff; }

    void free() const noexcept { 
        if ( obj->state == 0 ){ return; } 
             obj->state = 0; onClose.emit(); 
             onData.clear();
    }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class cipher_t {
protected:

    struct NODE {
        EVP_CIPHER_CTX* ctx = nullptr;
        ptr_t<uchar> bff;
        string_t buff;
        bool state =0;
        int    len =0;
    };  ptr_t<NODE> obj;

public:

    event_t<string_t> onData;
    event_t<>         onClose;

    template< class T >
    cipher_t( const string_t& key, int mode, const T& type ) 
    :    obj( new NODE() ) { if( key.empty() ){ return; }
          uchar iv[EVP_MAX_IV_LENGTH] = {0};
        obj->bff = ptr_t<uchar>(CHUNK_SIZE,'\0');
        obj->ctx = EVP_CIPHER_CTX_new(); obj->state = 1; 
        if ( !obj->ctx || !EVP_CipherInit_ex( obj->ctx, type, nullptr, (uchar*)key.data(), iv, mode ) )
           { throw except_t("can't initializate cipher_t"); }
    }

    template< class T >
    cipher_t( const string_t& iv, const string_t& key, int mode, const T& type ) 
    :   obj( new NODE() ) { if( key.empty() || iv.empty() ){ return; }
        obj->bff = ptr_t<uchar>(CHUNK_SIZE,'\0');
        obj->ctx = EVP_CIPHER_CTX_new(); obj->state = 1; 
        if ( !obj->ctx || !EVP_CipherInit_ex( obj->ctx, type, nullptr, (uchar*)key.data(), (uchar*)iv.data(), mode ) )
           { throw except_t("can't initializate cipher_t"); }
    }

    template< class T >
    cipher_t( int mode, const T& type ) : obj( new NODE() ) { 
        obj->bff = ptr_t<uchar>(CHUNK_SIZE,'\0');
        obj->ctx = EVP_CIPHER_CTX_new(); obj->state = 1; 
        if ( !obj->ctx || !EVP_CipherInit_ex( obj->ctx, type, nullptr, (uchar*)"\0", (uchar*)"\0", mode ) )
           { throw except_t("can't initializate cipher_t"); }
    }
    
    virtual ~cipher_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_CIPHER_CTX* get_fd() const noexcept { return obj->ctx; }

    void update( string_t msg ) const noexcept { if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ string_t tmp = msg.splice( 0, obj->bff.size()/2 );
            EVP_CipherUpdate( obj->ctx, &obj->bff, &obj->len, (uchar*)tmp.get(), tmp.size() );
            if ( obj->len > 0 ) { if ( onData.empty() ) {
                     obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
            } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); }}
        }
    }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        EVP_CipherFinal_ex( obj->ctx, &obj->bff, &obj->len );
        EVP_CIPHER_CTX_free( obj->ctx ); 
        if ( obj->len > 0 ) { if ( onData.empty() ) {
                 obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
        } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); 
        } onClose.emit(); } onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    string_t get() const noexcept { free(); return obj->buff; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class encrypt_t {
protected:

    struct NODE {
        EVP_CIPHER_CTX* ctx = nullptr;
        ptr_t<uchar> bff;
        string_t buff;
        bool state =0;
        int    len =0;
    };  ptr_t<NODE> obj;

public:

    event_t<string_t> onData;
    event_t<>         onClose;

    template< class T >
    encrypt_t( const string_t& key, const T& type )
    :   obj( new NODE() ) { if( key.empty() ){ return; }
        uchar iv[EVP_MAX_IV_LENGTH] = {0};
        obj->bff = ptr_t<uchar>(CHUNK_SIZE,'\0');
        obj->ctx = EVP_CIPHER_CTX_new(); obj->state = 1;
        if ( !obj->ctx || !EVP_EncryptInit_ex( obj->ctx, type, NULL, (uchar*)key.data(), iv ) )
           { throw except_t("can't initializate encrypt_t"); }
    }

    template< class T >
    encrypt_t( const string_t& iv, const string_t& key, const T& type )
    :   obj( new NODE() ) { if( key.empty() || iv.empty() ){ return; }
        obj->bff = ptr_t<uchar>(CHUNK_SIZE,'\0');
        obj->ctx = EVP_CIPHER_CTX_new(); obj->state = 1;
        if ( !obj->ctx || !EVP_EncryptInit_ex( obj->ctx, type, NULL, (uchar*)key.data(), (uchar*)iv.data() ) )
           { throw except_t("can't initializate encrypt_t"); }
    }

    void update( string_t msg ) const noexcept { if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ string_t tmp = msg.splice( 0, obj->bff.size()/2 );
            EVP_EncryptUpdate( obj->ctx, &obj->bff, &obj->len, (uchar*)tmp.get(), tmp.size() );
            if ( obj->len > 0 ) { if ( onData.empty() ) {
                     obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
            } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); }}
        }
    }
    
    virtual ~encrypt_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_CIPHER_CTX* get_fd() const noexcept { return obj->ctx; }

    string_t get() const noexcept { free(); return obj->buff; }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        EVP_EncryptFinal( obj->ctx, &obj->bff, &obj->len );
        EVP_CIPHER_CTX_free( obj->ctx ); 
        if ( obj->len > 0 ) { if ( onData.empty() ) {
                 obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
        } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); 
        } onClose.emit(); } onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class decrypt_t {
protected:

    struct NODE {
        EVP_CIPHER_CTX* ctx = nullptr; 
        ptr_t<uchar> bff;
        string_t buff;
        bool state =0; 
        int    len =0;
    };  ptr_t<NODE> obj;
    
public:

    event_t<string_t> onData;
    event_t<>         onClose;

    template< class T >
    decrypt_t( const string_t& key, const T& type )
    :   obj( new NODE() ) { if( key.empty() ){ return; }
        uchar iv[EVP_MAX_IV_LENGTH] = {0};
        obj->bff = ptr_t<uchar>(CHUNK_SIZE,'\0');
        obj->ctx = EVP_CIPHER_CTX_new(); obj->state = 1;
        if ( !obj->ctx || !EVP_DecryptInit_ex( obj->ctx, type, NULL, (uchar*)key.data(), iv ) )
           { throw except_t("can't initializate decrypt_t"); }
    }

    template< class T >
    decrypt_t( const string_t& iv, const string_t& key, const T& type )
    :   obj( new NODE() ) { if( key.empty() || iv.empty() ){ return; }
        obj->bff = ptr_t<uchar>(CHUNK_SIZE,'\0');
        obj->ctx = EVP_CIPHER_CTX_new(); obj->state = 1;
        if ( !obj->ctx || !EVP_DecryptInit_ex( obj->ctx, type, NULL, (uchar*)key.data(), (uchar*)iv.data() ) )
           { throw except_t("can't initializate decrypt_t"); }
    }

    void update( string_t msg ) const noexcept { if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ auto tmp = msg.splice( 0, obj->bff.size()/2 );
            EVP_DecryptUpdate( obj->ctx, &obj->bff, &obj->len, (uchar*)tmp.get(), tmp.size());
            if ( obj->len > 0 ) { if ( onData.empty() ) {
                     obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
            } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); }}
        }
    }
    
    virtual ~decrypt_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_CIPHER_CTX* get_fd() const noexcept { return obj->ctx; }

    string_t get() const noexcept { free(); return obj->buff; }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        EVP_DecryptFinal( obj->ctx, &obj->bff, &obj->len ); 
        EVP_CIPHER_CTX_free( obj->ctx ); 
        if ( obj->len > 0 ) { if ( onData.empty() ) {
                 obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
        } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); 
        } onClose.emit(); } onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class base64_encoder_t {
protected:

    struct CTX {
        int pos1, pos2;
        ulong     size;
        ulong     len;
    };

    struct NODE {
        ptr_t<char> bff;
        ptr_t<CTX>  ctx;
        string_t   buff;
        bool    state=0;
    };  ptr_t<NODE> obj;

public:

    event_t<>         onClose;
    event_t<string_t> onData;

    virtual ~base64_encoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    base64_encoder_t() noexcept : obj( new NODE() ) {
        obj->state = 1; obj->bff = ptr_t<char>( CHUNK_SIZE, '\0' );

        CTX item1; memset( &item1, 0, sizeof(CTX) );
            item1.pos1 = 0; item1.pos2 =-6; 
            item1.size = 0; item1.len  = 0;

        obj->ctx = type::bind( item1 );
    }

    void update( string_t msg ) const noexcept { if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ string_t tmp = msg.splice( 0, obj->bff.size()/2 );
            string_t out; obj->ctx->len = 0; forEach ( x, tmp ) {

                obj->ctx->pos1 = ( obj->ctx->pos1 << 8 ) + x; obj->ctx->pos2 += 8;

                while ( obj->ctx->pos2 >= 0 ) { 
                    obj->bff[obj->ctx->len] = CRYPTO_BASE64[(obj->ctx->pos1>>obj->ctx->pos2)&0x3F];
                    obj->ctx->pos2 -= 6; ++obj->ctx->len;
                }

            }   obj->ctx->size += obj->ctx->len; out = string_t( &obj->bff, obj->ctx->len );

            if ( obj->ctx->len == 0 ){ return; }
            if ( onData.empty() ) { obj->buff += out; } else { onData.emit( out ); }
        }
    }

    void free() const noexcept { if ( obj->state == 0 ){ return; } 
        string_t out; obj->state = 0; obj->ctx->len = 0;

        if ( obj->ctx->pos2 > -6 ){ 
            obj->bff[obj->ctx->len] = CRYPTO_BASE64[((obj->ctx->pos1<<8)>>(obj->ctx->pos2+8))&0x3F];
            obj->ctx->len++; 
        } while ( ( obj->ctx->len + obj->ctx->size ) % 4 ){ 
            obj->bff[obj->ctx->len] = '='; 
            obj->ctx->len++;
        } 

        obj->ctx->size += obj->ctx->len; out = string_t( &obj->bff, obj->ctx->len );
        if ( onData.empty() ) { obj->buff += out; } else { onData.emit( out ); }
             onClose.emit(); onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    string_t get() const noexcept { free(); return obj->buff; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class encoder_t {
protected:

    struct NODE {
        string_t chr;
        string_t buff;
        bool    state =0;
        BIGNUM* bn = nullptr;
    };  ptr_t<NODE> obj;

public:

    event_t<string_t> onData;
    event_t<>         onClose;

    encoder_t( const string_t& chr ) : obj( new NODE() ) { 
        obj->state = 1; obj->chr = chr; 
        obj->bn = (BIGNUM*) BN_new();
        if ( !obj->bn )
           { throw except_t("can't initializate encoder"); }
    }
    
    virtual ~encoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    string_t get() const noexcept { free(); return obj->buff; }

    void update( const string_t& msg ) const noexcept { 
        if( obj->state != 1 ){ return; }

        BN_bin2bn( (uchar*)msg.data(), msg.size(), obj->bn );

        while( BN_cmp( obj->bn, BN_value_one() ) > 0 ) {
            int rem = BN_div_word( obj->bn, obj->chr.size() );
                obj->buff.unshift( obj->chr[rem] );
        }

        for( const auto& byte : msg ) {
            if( byte != 0x00 ){ break; }
                obj->buff.unshift( obj->chr[0] );
        }

        if(!onData.empty() ) 
          { onData.emit( obj->buff ); obj->buff.clear(); }

    }

    void free() const noexcept { 
        if( obj->state == 1 ){ return; } obj->state = 0;
        if( obj->bn != nullptr ){ BN_clear_free( obj->bn ); }
            onClose.emit(); onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); }

};

/*────────────────────────────────────────────────────────────────────────────*/

class base64_decoder_t {
protected:

    struct CTX {
        int pos1, pos2;
        ulong     size;
        ulong      len;
        int    T [255];
    };

    struct NODE {
        ptr_t<char> bff;
        ptr_t<CTX>  ctx;
        string_t   buff;
        bool    state=0;
    };  ptr_t<NODE> obj;

public:

    event_t<>         onClose;
    event_t<string_t> onData;

    virtual ~base64_decoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    base64_decoder_t() noexcept : obj( new NODE() ) {
        obj->state = 1; obj->bff = ptr_t<char>( CHUNK_SIZE, '\0' );

        CTX item1; memset( &item1, 0, sizeof(CTX) );
            item1.pos1 = 0; item1.pos2 =-8; 
            item1.size = 0; item1.len  = 0;

        obj->ctx = type::bind( item1 );
    }

    void update( string_t msg ) const noexcept { if( obj->state != 1 ){ return; }
        while( !msg.empty() ){ string_t tmp = msg.splice( 0, obj->bff.size()/2 ); 
            for ( int x=0; x<64; x++ ){ obj->ctx->T[type::cast<int>(CRYPTO_BASE64[x])]=x; }

            string_t out; obj->ctx->len = 0; forEach ( x, tmp ) {
                uint   y = type::cast<uint>(x);

                if ( obj->ctx->T[y]==-1 ){ break; }

                obj->ctx->pos1 = ( obj->ctx->pos1 << 6 ) + obj->ctx->T[y]; obj->ctx->pos2 += 6;

                if ( obj->ctx->pos2 >= 0 ) {
                    obj->bff[obj->ctx->len] = char((obj->ctx->pos1>>obj->ctx->pos2)&0xFF);
                    obj->ctx->pos2 -= 8; ++obj->ctx->len;
                }

            }   obj->ctx->size += obj->ctx->len; out = string_t( &obj->bff, obj->ctx->len );

            if ( obj->ctx->len == 0 ){ return; }
            if ( onData.empty() ) { obj->buff += out; } else { onData.emit( out ); }
        }
    }

    void free() const noexcept { 
        if ( obj->state == 0 ){ return; } 
             obj->state =  0; onClose.emit();
             onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    string_t get() const noexcept { free(); return obj->buff; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class decoder_t {
protected:

    struct NODE {
        string_t chr;
        string_t buff;
        bool     state =0;
        BIGNUM* bn =nullptr;
    };  ptr_t<NODE> obj;

public:

    event_t<string_t> onData;
    event_t<>         onClose;

    decoder_t( const string_t& chr ) : obj( new NODE() ) { 
        obj->state = 1; obj->chr = chr; obj->bn = (BIGNUM*) BN_new();
        if ( !obj->bn )
           { throw except_t("can't initializate decoder"); }
    }
    
    virtual ~decoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    void update( const string_t& msg ) const { 
        if( obj->state != 1 ){ return; }

        for( const auto& c : msg ) {
             const char* pos = strchr( obj->chr.data(), c );
             if( pos == nullptr ) throw except_t("Invalid BaseX character");
             BN_mul_word( obj->bn, obj->chr.size() );
             BN_add_word( obj->bn, pos - obj->chr.data() );
        }

        ptr_t<uchar> out ( BN_num_bytes(obj->bn) );
        BN_bn2bin( obj->bn, out.data() ); if ( onData.empty() ) {
                 obj->buff += string_t( (char*) &out, out.size() );
        } else { onData.emit( string_t( (char*) &out, out.size() ) ); }
    }

    string_t get() const noexcept { free(); return obj->buff; }

    void free() const noexcept { 
        if( obj->state == 1 ){ return; } obj->state = 0;
        if( obj->bn != nullptr ){ BN_clear_free( obj->bn ); }
            onClose.emit(); onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class X509_t {
protected:

    struct NODE {
        X509_NAME* name = nullptr;
        EVP_PKEY*  pkey = nullptr;
        BIGNUM* num= nullptr;
        X509*  ctx = nullptr;
        RSA*   rsa = nullptr;
        bool  state= 0;
    };  ptr_t<NODE> obj;

    static int PASS_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size );
                buf[ size - 1 ] = '\0';
        return strlen(buf);
    }

public:

    X509_t( uint rsa_size=2048 ) : obj( new NODE() ) {
        
        obj->ctx = X509_new(); obj->name= X509_NAME_new();  
        obj->rsa = RSA_new();  obj->num = BN_new();
        obj->pkey= EVP_PKEY_new();
        
        BN_set_word( obj->num, RSA_F4 );
        RSA_generate_key_ex( obj->rsa, rsa_size, obj->num, NULL ); 

        obj->state = 1; if( !obj->ctx || !obj->rsa ) 
        { throw except_t("can't initializate X509_t"); }

    }

    virtual ~X509_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_PKEY* get_pub()  const noexcept { return obj->pkey; }

    X509*     get_cert() const noexcept { return obj->ctx; }

    RSA*      get_prv()  const noexcept { return obj->rsa; }

    void generate( string_t _name, string_t _contry, string_t _organization, ulong _time=31536000L ) const {

        X509_set_version( obj->ctx, 2 ); ASN1_INTEGER_set( X509_get_serialNumber(obj->ctx), 1 );
        
        X509_NAME_add_entry_by_txt( obj->name, "O",  MBSTRING_ASC, (uchar*) _organization.get(), -1, -1, 0);
        X509_NAME_add_entry_by_txt( obj->name, "C",  MBSTRING_ASC, (uchar*) _contry.get(), -1, -1, 0);
        X509_NAME_add_entry_by_txt( obj->name, "CN", MBSTRING_ASC, (uchar*) _name.get(), -1, -1, 0);
        X509_set_subject_name( obj->ctx, obj->name ); X509_set_issuer_name( obj->ctx, obj->name );

        if( _time != 0 ){
            X509_gmtime_adj( X509_getm_notBefore(obj->ctx), 0 );
            X509_gmtime_adj( X509_getm_notAfter(obj->ctx), _time );
        }

        EVP_PKEY_assign_RSA( obj->pkey, obj->rsa ); X509_set_pubkey( obj->ctx, obj->pkey );

        if( !X509_sign( obj->ctx, obj->pkey, EVP_sha256() ) )
          { throw except_t("can't generate X509 certificates"); }

    }

    string_t write_private_key_to_memory( const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_RSAPrivateKey( bo, obj->rsa, NULL, NULL, 0, &PASS_CLB, (void*)pass );
        long len = BIO_get_mem_data( bo, &data ); string_t res ( data, len );
        BIO_free(bo); return res;
    }

    string_t write_certificate_to_memory() const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_X509( bo, obj->ctx );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    void write_private_key( const string_t& path, const char* pass=NULL ) const {
        auto fp = fopen( path.get(), "w"); PEM_write_RSAPrivateKey( 
             fp, obj->rsa, NULL, NULL, 0, &PASS_CLB, (void*)pass 
        ); fclose( fp );
    }

    void write_certificate( const string_t& path ) const {
        auto fp = fopen( path.get(), "w");
        PEM_write_X509( fp, obj->ctx );
        fclose( fp );
    }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0; 
        if( obj->num != nullptr ){ BN_free( obj->num ); }
        if( obj->ctx != nullptr ){ X509_free( obj->ctx ); }
        if( obj->pkey!= nullptr ){ EVP_PKEY_free( obj->pkey ); }
        if( obj->name!= nullptr ){ X509_NAME_free( obj->name ); }
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

class rsa_t {
protected:

    struct NODE {
        RSA*    rsa = nullptr;
        BIGNUM* num = nullptr;
        ptr_t<uchar> bff;
        bool  state = 0;
    };  ptr_t<NODE> obj;

    static int PASS_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size );
                buf[ size - 1 ] = '\0';
        return strlen(buf);
    }
    
public:

    rsa_t() : obj( new NODE() ) {
        
        obj->rsa   = RSA_new();
        obj->num   =  BN_new();
        obj->state = 1;
        if ( !obj->num || !obj->rsa )
           { throw except_t("creating rsa object"); }
    }

    virtual ~rsa_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    RSA* get_fd() const noexcept { return obj->rsa; }

    int generate_keys( uint len=2048 ) const noexcept {
        len = clamp( len, 1024u, 4098u ); BN_set_word( obj->num, RSA_F4 );
        int c = RSA_generate_key_ex( obj->rsa, len, obj->num, NULL );
        obj->bff.resize( RSA_size(obj->rsa) ); return c;
    }

    void read_private_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_RSAPrivateKey( bo, &obj->rsa, &PASS_CLB, (void*)pass ) ){
            BIO_free(bo); throw except_t( "Invalid RSA Key" );
        }   BIO_free(bo); obj->bff.resize(RSA_size(obj->rsa));
    }

    void read_public_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_RSAPublicKey( bo, &obj->rsa, &PASS_CLB, (void*)pass ) ){
            BIO_free(bo); throw except_t( "Invalid RSA Key" );
        }   BIO_free(bo); obj->bff.resize(RSA_size(obj->rsa));
    }

    string_t write_private_key_to_memory( const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_RSAPrivateKey( bo, obj->rsa, NULL, NULL, 0, &PASS_CLB, (void*)pass );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    string_t write_public_key_to_memory() const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_RSAPublicKey( bo, obj->rsa );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    int write_private_key( const string_t& path, const char* pass=NULL ) const {
        FILE* fp = fopen( path.data() , "w"); int res = 0;
        if ( fp == nullptr ){ throw except_t("while writing private key"); }
        res = PEM_write_RSAPrivateKey( fp, obj->rsa, NULL, NULL, 0, &PASS_CLB, (void*)pass );
        fclose( fp ); return res;
    }

    int write_public_key( const string_t& path ) const {
        FILE* fp = fopen( path.data() , "w"); int res = 0;
        if ( fp == nullptr ){ throw except_t("while writing public key"); }
        res = PEM_write_RSAPublicKey( fp, obj->rsa );
        fclose( fp ); return res;
    }

    void read_public_key( const string_t& path, const char* pass=NULL ) const {
        FILE* fp = fopen( path.data(), "r" );
        if( fp == nullptr ){ throw except_t("while reading public key"); }
        if( !PEM_read_RSAPublicKey( fp, &obj->rsa, &PASS_CLB, (void*)pass ) ){
            fclose( fp ); throw except_t( "Invalid RSA Key" );
        }   fclose( fp ); obj->bff.resize(RSA_size(obj->rsa));
    }

    void read_private_key( const string_t& path, const char* pass=NULL ) const {
        FILE* fp = fopen( path.data(), "r" );
        if( fp == nullptr ){ throw except_t("while reading private key"); }
        if( !PEM_read_RSAPrivateKey( fp, &obj->rsa, &PASS_CLB, (void*)pass ) ){
            fclose( fp ); throw except_t( "Invalid RSA Key" );
        }   fclose( fp ); obj->bff.resize(RSA_size(obj->rsa));
    }

    string_t public_encrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if ( msg.empty() || obj->state ==0 || obj->rsa == nullptr ){ return nullptr; }
        string_t data; while( !msg.empty() ){ auto tmp = msg.splice( 0, obj->bff.size()-42 );
            int c = RSA_public_encrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        }   return data;
    }

    string_t private_encrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if( msg.empty() || obj->state ==0 || obj->rsa == nullptr ){ return nullptr; }
        string_t data; while( !msg.empty() ){ auto tmp = msg.splice( 0, obj->bff.size()-42 );
            int c = RSA_private_encrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        }   return data;
    }

    string_t public_decrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if( msg.empty() || obj->state ==0 || obj->rsa == nullptr ){ return nullptr; }
        string_t data; while( !msg.empty() ){ auto tmp = msg.splice( 0, obj->bff.size() );
            int c = RSA_public_decrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        }   return data;
    }

    string_t private_decrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if( msg.empty() || obj->state ==0 || obj->rsa == nullptr ){ return nullptr; }
        string_t data; while( !msg.empty() ){ auto tmp = msg.splice( 0, obj->bff.size() );
            int c = RSA_private_decrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        }   return data;
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state =0;
        if( obj->num != nullptr ){ BN_free( obj->num ); }
        if( obj->rsa != nullptr ){ RSA_free( obj->rsa ); }
    }
    
};

/*────────────────────────────────────────────────────────────────────────────*/

class ec_t {
protected:

    struct NODE {
        EC_GROUP *key_group = nullptr;
        EC_POINT *pub_key   = nullptr;
        BIGNUM   *priv_key  = nullptr;
        EC_KEY   *key_pair  = nullptr;
        bool      state = 0;
    };  ptr_t<NODE> obj;
    
public:

    template< class T >
    ec_t( const string_t& key, const T& type ) noexcept
    :obj( new NODE() ) { if( key.empty() ){ return; }
        obj->state = 1;

        obj->key_pair  = EC_KEY_new_by_curve_name(type);
        obj->key_group = EC_GROUP_new_by_curve_name(type);

        obj->priv_key = (BIGNUM*) BN_new(); 
        BN_hex2bn( &obj->priv_key, key.data() );
        EC_KEY_set_private_key( obj->key_pair, obj->priv_key );

        obj->pub_key = (EC_POINT*) EC_POINT_new( obj->key_group );
        EC_POINT_mul( obj->key_group, obj->pub_key, obj->priv_key, nullptr, nullptr, nullptr );
        EC_KEY_set_public_key( obj->key_pair, obj->pub_key );
    }

    template< class T >
    ec_t( const T& type ) noexcept 
    :   obj( new NODE() ) { 
        obj->state = 1;

        obj->key_pair  = EC_KEY_new();
        obj->key_group = EC_GROUP_new_by_curve_name( type );

        EC_KEY_set_group( obj->key_pair, obj->key_group );
        EC_KEY_generate_key( obj->key_pair );

        obj->pub_key  = (EC_POINT*) EC_KEY_get0_public_key( obj->key_pair );
        obj->priv_key = (BIGNUM*)  EC_KEY_get0_private_key( obj->key_pair );
    }
    
    virtual ~ec_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    string_t get_public_key( uint x = 0 ) const noexcept { 
        if( obj->state != 1 ){ return nullptr; }
        point_conversion_form_t y; switch( x ){
            case 0:  y = POINT_CONVERSION_HYBRID;       break;
            case 1:  y = POINT_CONVERSION_COMPRESSED;   break;
            default: y = POINT_CONVERSION_UNCOMPRESSED; break;
        }   return EC_POINT_point2hex( obj->key_group, obj->pub_key, y, nullptr );
    }

    string_t get_private_key() const noexcept { return BN_bn2hex( obj->priv_key ); }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        if( obj->priv_key  != nullptr ){ BN_free( obj->priv_key ); }
    //  if( obj->key_pair  != nullptr ){ EC_KEY_free( obj->key_pair ); }
        if( obj->pub_key   != nullptr ){ EC_POINT_free( obj->pub_key ); }
        if( obj->key_group != nullptr ){ EC_GROUP_free( obj->key_group ); }
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class dh_t {
protected:

    struct NODE {
        DH     *dh = nullptr;
        BIGNUM *k  = nullptr;
        bool state = 0;
    };  ptr_t<NODE> obj;

public:

    dh_t() : obj( new NODE() ) {
        
        obj->dh    = DH_new(); 
        obj->k     = BN_new();
        obj->state = 1;
        if( !obj->dh || !obj->k )
          { throw except_t( "creating new dh" ); }
    }

    virtual ~dh_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    int generate_keys( int len=512 ) const noexcept {
        if( !DH_generate_parameters_ex( obj->dh, len, DH_GENERATOR_2, NULL ) )
          { return -1; } if( !DH_generate_key( obj->dh ) )
          { return -1; } return 1;
    }

    int set_public_key( const string_t& key ) const noexcept {
        if( obj->state != 1 ){ return 0; }
               BN_hex2bn( &obj->k, key.data() );
        return DH_set0_key( obj->dh, nullptr, obj->k );
    }

    int set_private_key( const string_t& key ) const noexcept {
        if( obj->state != 1 ){ return 0; }
               BN_hex2bn( &obj->k, key.data() );
        return DH_set0_key( obj->dh, obj->k, nullptr );
    }

    string_t get_private_key() const noexcept {
        if( obj->state != 1 ){ return nullptr; } 
        return BN_bn2hex( DH_get0_priv_key( obj->dh ) );
    }

    string_t get_public_key() const noexcept {
        if( obj->state != 1 ){ return nullptr; } 
        return BN_bn2hex( DH_get0_pub_key( obj->dh ) );
    }

    void free() const noexcept {
        if( obj->state == 0 ){ return; } obj->state = 0;
        if( obj->dh != nullptr ){ DH_free( obj->dh ); }
        if( obj->k  != nullptr ){ BN_free( obj->k ); }
    }

    bool verify( const string_t& hex, const string_t& sgn ) const {
         return sign( hex ) == sgn;
    }

    string_t sign( const string_t& hex ) const {
        if( obj->state != 1 ){ return nullptr; } 
        ptr_t<uchar> shared( DH_size( obj->dh ) );
        if( !BN_hex2bn( &obj->k,hex.data() ) )
          { throw except_t( "invalid key" ); }
        int len = DH_compute_key( &shared, obj->k, obj->dh );
        return encoder::buffer::buff2hex( string_t( (char*) &shared, (ulong) len ) );
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

class dsa_t {
protected:

    struct NODE {
        DSA    *dsa = nullptr;
        bool    state = 0;
    };  ptr_t<NODE> obj;
    
    static int PASS_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size );
                buf[ size - 1 ] = '\0';
        return strlen(buf);
    }
    
public:

    dsa_t(): obj( new NODE() ) { 
        obj->state = 1; obj->dsa = DSA_new(); 
    }

    virtual ~dsa_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    int generate_keys( uint len=512 ) const noexcept {
        if(!DSA_generate_parameters_ex( obj->dsa, len, NULL, 0, NULL, NULL, NULL ) )
          { return -1; } if(!DSA_generate_key( obj->dsa ) )
          { return -1; } return 1;
    }

    bool verify( const string_t& msg, const string_t& sgn ) const noexcept { 
         if( obj->state != 1 || obj->dsa == nullptr ){ return false; } auto ngs = encoder::buffer::hex2buff( sgn ); 
         return DSA_verify( 0, (uchar*)msg.data(), msg.size(), (uchar*)ngs.data(), ngs.size(), obj->dsa )>0;
    }

    string_t sign( const string_t& msg ) const noexcept {
        if( obj->state != 1 || obj->dsa == nullptr ){ return nullptr; }
        ptr_t<uchar> sgn( DSA_size(obj->dsa) ); uint len;
        DSA_sign( 0,(uchar*)msg.data(), msg.size(),&sgn, &len, obj->dsa );
        return encoder::buffer::buff2hex( string_t( (char*) &sgn, (ulong) len ) );
    }

    void read_private_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_DSAPrivateKey( bo, &obj->dsa, &PASS_CLB, (void*)pass ) )
          { BIO_free(bo); throw except_t( "Invalid DSA Key" ); } BIO_free(bo);
    }

    void read_public_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_DSA_PUBKEY( bo, &obj->dsa, &PASS_CLB, (void*)pass ) )
          { BIO_free(bo); throw except_t( "Invalid DSA Key" ); } BIO_free(bo);
    }

    string_t write_private_key_to_memory( const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_DSAPrivateKey( bo, obj->dsa, NULL, NULL, 0, &PASS_CLB, (void*)pass );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    string_t write_public_key_to_memory() const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_DSA_PUBKEY( bo, obj->dsa );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    void read_private_key( const string_t& path, const char* pass=NULL ) const {
        FILE* fp = fopen(path.data(),"r");
        if ( fp == nullptr ){ throw except_t(" while reading private key"); }
        obj->dsa = PEM_read_DSAPrivateKey( fp, &obj->dsa, &PASS_CLB, (void*)pass );
        if ( obj->dsa == nullptr )
           { fclose(fp); throw except_t( "Invalid DSA Key" ); } fclose(fp); 
    }

    void read_public_key( const string_t& path, const char* pass=NULL ) const {
        FILE* fp = fopen(path.data(),"r");
        if ( fp == nullptr ){ throw except_t(" while reading public key"); }
        obj->dsa = PEM_read_DSA_PUBKEY( fp, &obj->dsa, &PASS_CLB, (void*)pass );
        if ( obj->dsa == nullptr )
           { fclose(fp); throw except_t( "Invalid DSA Key" ); } fclose(fp);  
    }

    void write_private_key( const string_t& path ) const {
        if( obj->state != 1 ){ return; } FILE* fp = fopen( path.data(), "w" );
        if ( fp == nullptr ) { throw except_t("while creating file"); }
        if (!PEM_write_DSA_PUBKEY( fp, obj->dsa ) ) 
           { fclose( fp ); throw except_t("while writting the private key"); } fclose( fp );
    }

    void write_public_key( const string_t& path, const char* pass=NULL ) const {
        if( obj->state != 1 ){ return; } FILE* fp = fopen( path.data(), "w" );
        if ( fp == nullptr ) { throw except_t("while creating file"); }
        if (!PEM_write_DSAPrivateKey( fp, obj->dsa, nullptr, nullptr, 0, &PASS_CLB, (void*)pass ) )
           { fclose( fp ); throw except_t("while writting the public key"); } fclose( fp );
    }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        if( obj->dsa != nullptr ) DSA_free( obj->dsa );
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 
    
};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined(GENERATOR_CRYPTO) && defined(NODEPP_CRYPTO)
    #define  GENERATOR_CRYPTO
namespace nodepp {
namespace crypto { namespace hash {

    class MD4 : public hash_t { public:
          MD4() : hash_t( EVP_md4(), MD4_DIGEST_LENGTH ) {}
    };

    class MD5 : public hash_t { public:
          MD5() : hash_t( EVP_md5(), MD5_DIGEST_LENGTH ) {}
    };

    class SHA1 : public hash_t { public:
          SHA1() : hash_t( EVP_sha1(), SHA_DIGEST_LENGTH ) {}
    };

    class SHA256 : public hash_t { public:
          SHA256() : hash_t( EVP_sha256(), SHA256_DIGEST_LENGTH ) {}
    };

    class SHA384 : public hash_t { public:
          SHA384() : hash_t( EVP_sha384(), SHA384_DIGEST_LENGTH ) {}
    };

    class SHA512 : public hash_t { public:
          SHA512() : hash_t( EVP_sha512(), SHA512_DIGEST_LENGTH ) {}
    };

    class SHA3_256 : public hash_t { public:
          SHA3_256() : hash_t( EVP_sha3_256(), SHA256_DIGEST_LENGTH ) {}
    };

    class SHA3_384 : public hash_t { public:
          SHA3_384() : hash_t( EVP_sha3_384(), SHA384_DIGEST_LENGTH ) {}
    };

    class SHA3_512 : public hash_t { public:
          SHA3_512() : hash_t( EVP_sha3_512(), SHA512_DIGEST_LENGTH ) {}
    };

    class RIPEMD160 : public hash_t { public:
          RIPEMD160() : hash_t( EVP_ripemd160(), RIPEMD160_DIGEST_LENGTH ) {}
    }; 

}}
    
    /*─······································································─*/

namespace crypto { namespace hmac {

    class MD4 : public hmac_t { public:
          MD4 ( const string_t& key ) : hmac_t( key, EVP_md4(), MD4_DIGEST_LENGTH ) {}
    };

    class MD5 : public hmac_t { public:
          MD5 ( const string_t& key ) : hmac_t( key, EVP_md5(), MD5_DIGEST_LENGTH ) {}
    };

    class SHA1 : public hmac_t { public:
          SHA1 ( const string_t& key ) : hmac_t( key, EVP_sha1(), SHA_DIGEST_LENGTH ) {}
    };

    class SHA256 : public hmac_t { public:
          SHA256 ( const string_t& key ) : hmac_t( key, EVP_sha256(), SHA256_DIGEST_LENGTH ) {}
    };

    class SHA384 : public hmac_t { public:
          SHA384 ( const string_t& key ) : hmac_t( key, EVP_sha384(), SHA384_DIGEST_LENGTH ) {}
    };

    class SHA512 : public hmac_t { public:
          SHA512 ( const string_t& key ) : hmac_t( key, EVP_sha512(), SHA512_DIGEST_LENGTH ) {}
    };

    class SHA3_256 : public hmac_t { public:
          SHA3_256 ( const string_t& key ) : hmac_t( key, EVP_sha3_256(), SHA256_DIGEST_LENGTH ) {}
    };

    class SHA3_384 : public hmac_t { public:
          SHA3_384 ( const string_t& key ) : hmac_t( key, EVP_sha3_384(), SHA384_DIGEST_LENGTH ) {}
    };

    class SHA3_512 : public hmac_t { public:
          SHA3_512 ( const string_t& key ) : hmac_t( key, EVP_sha3_512(), SHA512_DIGEST_LENGTH ) {}
    };

    class RIPEMD160 : public hmac_t { public:
          RIPEMD160( const string_t& key ) : hmac_t( key, EVP_ripemd160(), RIPEMD160_DIGEST_LENGTH ) {}
    }; 

}}
    
    /*─······································································─*/

namespace crypto { namespace encrypt {
    
    class AES_128_CBC : public encrypt_t { public: template< class... T >
          AES_128_CBC( const T&... args ) : encrypt_t( args..., EVP_aes_128_cbc() ) {}
    };
    
    class AES_192_CBC : public encrypt_t { public: template< class... T >
          AES_192_CBC( const T&... args ) : encrypt_t( args..., EVP_aes_192_cbc() ) {}
    };
    
    class AES_256_CBC : public encrypt_t { public: template< class... T >
          AES_256_CBC( const T&... args ) : encrypt_t( args..., EVP_aes_256_cbc() ) {}
    };

    /*─······································································─*/
    
    class AES_128_ECB : public encrypt_t { public: template< class... T >
          AES_128_ECB( const T&... args ) : encrypt_t( args..., EVP_aes_128_ecb() ) {}
    };
    
    class AES_192_ECB : public encrypt_t { public: template< class... T >
          AES_192_ECB( const T&... args ) : encrypt_t( args..., EVP_aes_192_ecb() ) {}
    };
    
    class AES_256_ECB : public encrypt_t { public: template< class... T >
          AES_256_ECB( const T&... args ) : encrypt_t( args..., EVP_aes_256_ecb() ) {}
    };

    /*─······································································─*/
    
    class TRIPLE_DES_CFB : public encrypt_t { public: template< class... T >
          TRIPLE_DES_CFB ( const T&... args ) : encrypt_t( args..., EVP_des_ede3_cfb() ) {}
    };
    
    class TRIPLE_DES_CBC : public encrypt_t { public: template< class... T >
          TRIPLE_DES_CBC ( const T&... args ) : encrypt_t( args..., EVP_des_ede3_cbc() ) {}
    };
    
    class TRIPLE_DES_ECB : public encrypt_t { public: template< class... T >
          TRIPLE_DES_ECB ( const T&... args ) : encrypt_t( args..., EVP_des_ede3_ecb() ) {}
    };

    /*─······································································─*/
    
    class DES_CFB : public encrypt_t { public: template< class... T >
          DES_CFB ( const T&... args ) : encrypt_t( args..., EVP_des_ede_cfb() ) {}
    };
    
    class DES_CBC : public encrypt_t { public: template< class... T >
          DES_CBC ( const T&... args ) : encrypt_t( args..., EVP_des_ede_cbc() ) {}
    };
    
    class DES_ECB : public encrypt_t { public: template< class... T >
          DES_ECB ( const T&... args ) : encrypt_t( args..., EVP_des_ede_ecb() ) {}
    };

    /*─······································································─*/

    class RSA : public rsa_t { public: template< class... T > 
          RSA ( const T&... args ) : rsa_t( args... ) {}
    };

    /*─······································································─*/
    
    class XOR : public xor_t { public: template< class... T >
          XOR ( const T&... args ) : xor_t( args... ) {}
    };

}}
    
    /*─······································································─*/

namespace crypto { namespace decrypt {
    
    class AES_128_CBC : public decrypt_t { public: template< class... T >
          AES_128_CBC( const T&... args ) : decrypt_t( args..., EVP_aes_128_cbc() ) {}
    };
    
    class AES_192_CBC : public decrypt_t { public: template< class... T >
          AES_192_CBC( const T&... args ) : decrypt_t( args..., EVP_aes_192_cbc() ) {}
    };
    
    class AES_256_CBC : public decrypt_t { public: template< class... T >
          AES_256_CBC( const T&... args ) : decrypt_t( args..., EVP_aes_256_cbc() ) {}
    };

    /*─······································································─*/
    
    class AES_128_ECB : public decrypt_t { public: template< class... T >
          AES_128_ECB( const T&... args ) : decrypt_t( args..., EVP_aes_128_ecb() ) {}
    };
    
    class AES_192_ECB : public decrypt_t { public: template< class... T >
          AES_192_ECB( const T&... args ) : decrypt_t( args..., EVP_aes_192_ecb() ) {}
    };
    
    class AES_256_ECB : public decrypt_t { public: template< class... T >
          AES_256_ECB( const T&... args ) : decrypt_t( args..., EVP_aes_256_ecb() ) {}
    };

    /*─······································································─*/
    
    class TRIPLE_DES_CFB : public decrypt_t { public: template< class... T >
          TRIPLE_DES_CFB ( const T&... args ) : decrypt_t( args..., EVP_des_ede3_cfb() ) {}
    };
    
    class TRIPLE_DES_CBC : public decrypt_t { public: template< class... T >
          TRIPLE_DES_CBC ( const T&... args ) : decrypt_t( args..., EVP_des_ede3_cbc() ) {}
    };
    
    class TRIPLE_DES_ECB : public decrypt_t { public: template< class... T >
          TRIPLE_DES_ECB ( const T&... args ) : decrypt_t( args..., EVP_des_ede3_ecb() ) {}
    };

    /*─······································································─*/
    
    class DES_CFB : public decrypt_t { public: template< class... T >
          DES_CFB ( const T&... args ) : decrypt_t( args..., EVP_des_ede_cfb() ) {}
    };
    
    class DES_CBC : public decrypt_t { public: template< class... T >
          DES_CBC ( const T&... args ) : decrypt_t( args..., EVP_des_ede_cbc() ) {}
    };
    
    class DES_ECB : public decrypt_t { public: template< class... T >
          DES_ECB ( const T&... args ) : decrypt_t( args..., EVP_des_ede_ecb() ) {}
    };

    /*─······································································─*/

    class RSA : public rsa_t { public: template< class... T > 
          RSA ( const T&... args ) : rsa_t( args... ) {}
    };

    /*─······································································─*/
    
    class XOR : public xor_t { public: template< class... T >
          XOR ( const T&... args ) : xor_t( args... ) {}
    };

}}
    
    /*─······································································─*/

namespace crypto { namespace encoder {

    class BASE58 : public encoder_t { public:
          BASE58 () : encoder_t( "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz" ) {}
    };

    class BASE16 : public encoder_t { public: 
          BASE16 () : encoder_t( "123456789ABCDEF" ){}
    };

    class BASE8 : public encoder_t { public: 
          BASE8 () : encoder_t( "1234567" ){}
    };

    class BASE4 : public encoder_t { public: 
          BASE4 () : encoder_t( "123" ){}
    };

    class BASE64 : public base64_encoder_t { public:
          BASE64 () : base64_encoder_t() {}
    };

}}
    
    /*─······································································─*/

namespace crypto { namespace decoder {

    class BASE58 : public decoder_t { public:
          BASE58 () : decoder_t( "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz" ) {}
    };

    class BASE16 : public decoder_t { public: 
          BASE16 () : decoder_t( "123456789ABCDEF" ){}
    };

    class BASE8 : public decoder_t { public: 
          BASE8 () : decoder_t( "1234567" ){}
    };

    class BASE4 : public decoder_t { public: 
          BASE4 () : decoder_t( "123" ){}
    };

    class BASE64 : public base64_decoder_t { public:
          BASE64 () : base64_decoder_t() {}
    };

}}
    
    /*─······································································─*/

namespace crypto { namespace curve { //openssl ecparam -list_curves
    
    class PRIME256V1: public ec_t { public: template< class... T >
          PRIME256V1( const T&... args ) noexcept : ec_t( args..., NID_X9_62_prime256v1 ) {}
    };
    
    class PRIME192V1 : public ec_t { public: template< class... T >
          PRIME192V1( const T&... args ) noexcept : ec_t( args..., NID_X9_62_prime192v1 ) {}
    };

    /*─······································································─*/
    
    class SECP128R1 : public ec_t { public: template< class... T >
          SECP128R1( const T&... args ) noexcept : ec_t( args..., NID_secp128r1 ) {}
    };
    
    class SECP128R2 : public ec_t { public: template< class... T >
          SECP128R2( const T&... args ) noexcept : ec_t( args..., NID_secp128r2 ) {}
    };

    /*─······································································─*/
    
    class SECP160R1 : public ec_t { public: template< class... T >
          SECP160R1( const T&... args ) noexcept : ec_t( args..., NID_secp160r1 ) {}
    };
    
    class SECP160R2 : public ec_t { public: template< class... T >
          SECP160R2( const T&... args ) noexcept : ec_t( args..., NID_secp160r2 ) {}
    };
    
    class SECP160K1 : public ec_t { public: template< class... T >
          SECP160K1( const T&... args ) noexcept : ec_t( args..., NID_secp160k1 ) {}
    };

    /*─······································································─*/
    
    class SECP256K1 : public ec_t { public: template< class... T >
          SECP256K1( const T&... args ) noexcept : ec_t( args..., NID_secp256k1 ) {}
    };

}}
    
    /*─······································································─*/

namespace crypto { namespace sign {
    
    class DSA : public dsa_t { public: template< class... T >
          DSA ( const T&... args ) : dsa_t ( args... ) {}
    };
    
    class DH : public dh_t { public: template< class... T >
          DH ( const T&... args ) : dh_t ( args... ) {}
    };

}}
    
    /*─······································································─*/

namespace crypto { namespace certificate {

    class X509 : public X509_t { public: template< class... T > 
          X509 ( const T&... args ) : X509_t ( args... ) {}
    };

}}
  
    /*─······································································─*/

}

#undef CRYPTO_BASE64
#endif
