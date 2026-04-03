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
#include "fs.h"

/*────────────────────────────────────────────────────────────────────────────*/

#include <openssl/ripemd.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
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
       ~NODE() { if( ctx ){ EVP_MD_CTX_free( ctx ); } }
    };  ptr_t<NODE> obj;

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

   ~hash_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_MD_CTX* get_fd() const noexcept { return obj->ctx; }

    void update( string_t msg ) const noexcept { 
        if( !obj->state ){ return; } ulong chunk=0, base=(ulong)( obj->bff.size() );
        while( chunk < msg.size() ){ 
            string_t tmp = msg.slice_view( chunk, chunk + base );
            EVP_DigestUpdate( obj->ctx, (uchar*) tmp.data(), tmp.size() );
        chunk += base; }
    }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        EVP_DigestFinal_ex( obj->ctx, &obj->bff, &obj->length );
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
    :  obj( new NODE() ) { if( key.empty() ){ throw except_t("can't initializate hmac_t"); }
        obj->bff   = ptr_t<uchar>( length ); 
        obj->ctx   = HMAC_CTX_new(); 
        obj->state = 1;
        if ( !obj->ctx || !HMAC_Init_ex( obj->ctx, key.data(), key.size(), type, nullptr ) )
           { throw except_t("can't initializate hmac_t"); }
    }
    
   ~hmac_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    HMAC_CTX* get_fd() const noexcept { return obj->ctx; }

    void update( string_t msg ) const noexcept { 
        if( !obj->state ){ return; } ulong chunk=0, base=(ulong)( obj->bff.size() );
        while( chunk < msg.size() ){ 
            string_t tmp = msg.slice_view( chunk, chunk + base );
            HMAC_Update( obj->ctx, (uchar*) tmp.data(), tmp.size() );
        chunk += base; }
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
        string_t    bff;
        bool    state=0;
    };  ptr_t<NODE> obj;

public:

    event_t<>         onClose;
    event_t<string_t> onData;

    xor_t( const string_t& key ) : obj( new NODE() ) {
        if( key.empty() ){ throw except_t("can't initializate xor_t"); }
    
        CTX item1; //memset( &item1, 0, sizeof(CTX) );
            item1.key  = key; item1.pos = 0;
            obj->state = 1;

        obj->ctx = ptr_t<CTX> ({ item1 });
    }

    xor_t() noexcept : obj( new NODE() ) { obj->state = 0; }
    
   ~xor_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    void update( string_t msg ) const noexcept { 
        if( !obj->state ){ return; } ulong chunk=0, /*-------------*/ base=CHUNK_SIZE;
        while( chunk < msg.size() ){ string_t tmp = msg.slice_view( chunk, chunk + base );
            forEach( y, obj->ctx ){ forEach( x, tmp ){ 
                x = x ^ y.key[ y.pos % y.key.size() ]; ++y.pos; 
            }} if ( tmp.empty() )     { return; }
             elif ( onData.empty() )  { obj->bff +=tmp; }
             else { onData.emit(tmp); }
        chunk += base; }
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    string_t get() const noexcept { free(); return obj->bff; }

    void free() const noexcept { 
        if ( obj->state == 0 ){ return; } 
             obj->state = 0; 
        onClose.emit(); onData.clear();
    }

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
       ~NODE() { if( ctx ){ EVP_CIPHER_CTX_free( ctx ); } }
    };  ptr_t<NODE> obj;

    void _init_( const EVP_CIPHER* type, const string_t& key, const string_t& iv ){
        if( key.empty() ){ throw except_t("can't initializate encrypt_t"); }

        obj->bff        = ptr_t<uchar>(CHUNK_SIZE,'\0');
        int req_key_len = EVP_CIPHER_key_length( type );
        int req_iv_len  = EVP_CIPHER_iv_length ( type );
        obj->ctx        = EVP_CIPHER_CTX_new(); 
        obj->state      = 1;

        ptr_t<uchar> nkey( (ulong) ( req_key_len ), 0x00 );
        ptr_t<uchar> niv ( (ulong) ( req_iv_len  ), 0x00 );
        memcpy( nkey.get(), key.get(), min( nkey.size(), key.size() ) );
        memcpy( niv .get(), iv .get(), min( niv .size(), iv .size() ) );

        if ( !obj->ctx || !EVP_EncryptInit_ex( obj->ctx, type, NULL, nkey.data(), niv.data() ) )
           { throw except_t("can't initializate encrypt_t"); }

    }

public:

    event_t<string_t> onData;
    event_t<>         onClose;

    encrypt_t( const string_t& iv, const string_t& key, const EVP_CIPHER* type ) : obj( new NODE() ) { _init_( type, key, iv ); }

    encrypt_t( const string_t& key, const EVP_CIPHER* type ) : obj( new NODE() ) { _init_( type, key, nullptr ); }

    void update( string_t msg ) const noexcept { 
        if( !obj->state ){ return; } ulong chunk=0, base=(ulong)( obj->bff.size() );
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
            EVP_EncryptUpdate( obj->ctx, &obj->bff, &obj->len, (uchar*)tmp.get(), tmp.size() );
            if ( obj->len > 0 ) { if ( onData.empty() ) {
                     obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
            } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); }}
        chunk += base; }
    }
    
   ~encrypt_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_CIPHER_CTX* get_fd() const noexcept { return obj->ctx; }

    string_t get() const noexcept { free(); return obj->buff; }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        EVP_EncryptFinal( obj->ctx, &obj->bff, &obj->len );
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
       ~NODE() { if( ctx ){ EVP_CIPHER_CTX_free( ctx ); } }
    };  ptr_t<NODE> obj;

    void _init_( const EVP_CIPHER* type, const string_t& key, const string_t& iv ){
        if( key.empty() ){ throw except_t("can't initializate decrypt_t"); }

        obj->bff        = ptr_t<uchar>(CHUNK_SIZE,'\0');
        int req_key_len = EVP_CIPHER_key_length( type );
        int req_iv_len  = EVP_CIPHER_iv_length ( type );
        obj->ctx        = EVP_CIPHER_CTX_new(); 
        obj->state      = 1;

        ptr_t<uchar> nkey( (ulong) ( req_key_len ), 0x00 );
        ptr_t<uchar> niv ( (ulong) ( req_iv_len  ), 0x00 );
        memcpy( nkey.get(), key.get(), min( nkey.size(), key.size() ) );
        memcpy( niv .get(), iv .get(), min( niv .size(), iv .size() ) );

        if ( !obj->ctx || !EVP_DecryptInit_ex( obj->ctx, type, NULL, nkey.data(), niv.data() ) )
           { throw except_t("can't initializate encrypt_t"); }

    }
    
public:

    event_t<string_t> onData;
    event_t<>         onClose;

    decrypt_t( const string_t& iv, const string_t& key, const EVP_CIPHER* type ) : obj( new NODE() ) { _init_( type, key, iv ); }

    decrypt_t( const string_t& key, const EVP_CIPHER* type ) : obj( new NODE() ) { _init_( type, key, nullptr ); }

    void update( string_t msg ) const noexcept { 
        if( !obj->state ){ return; } ulong chunk=0, base=(ulong)( obj->bff.size() );
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
            EVP_DecryptUpdate( obj->ctx, &obj->bff, &obj->len, (uchar*)tmp.get(), tmp.size());
            if ( obj->len > 0 ) { if ( onData.empty() ) {
                     obj->buff += string_t( (char*)&obj->bff, (ulong) obj->len );
            } else { onData.emit( string_t( (char*)&obj->bff, (ulong) obj->len ) ); }}
        chunk += base; }
    }
    
   ~decrypt_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    EVP_CIPHER_CTX* get_fd() const noexcept { return obj->ctx; }

    string_t get() const noexcept { free(); return obj->buff; }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        EVP_DecryptFinal( obj->ctx, &obj->bff, &obj->len ); 
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

class encoder_t {
protected:

    struct NODE {
        string_t chr; bool state =0;
        queue_t<string_t> bff;
        BIGNUM* bn = nullptr;
       ~NODE() { if( bn ){ BN_clear_free( bn ); } }
    };  ptr_t<NODE> obj;

    string_t encode( string_t msg ) const noexcept {
        if( msg.empty() ){ return nullptr; }

        BN_zero(obj->bn); BN_bin2bn((uchar*)msg.data(), msg.size(), obj->bn);

        string_t result; while(!BN_is_zero(obj->bn)) {
            int rem = BN_div_word(obj->bn, obj->chr.size());
            result.unshift(obj->chr[rem]);
        }

        for( auto& byte : msg ) {
        if ( byte != 0x00 ){ break; }
             result.unshift(obj->chr[0]);
        }

        if( !onData.empty() ){ onData.emit(result); }

    return result; }

public:

    event_t<string_t> onData;
    event_t<>         onClose;

    encoder_t( const string_t& chr ) : obj( new NODE() ) { 
        obj->state = 1; obj->chr = chr; obj->bn = (BIGNUM*) BN_new();
        if( !obj->bn ){ throw except_t("can't initializate encoder"); }
    }
    
   ~encoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    string_t get() const noexcept { if( obj->state == 0 ){ return nullptr; }
        auto raw = array_t<string_t>( obj->bff.data() ).join(nullptr);
        auto data= encode( raw ); free(); return data; 
    }

    void update( const string_t& msg ) const noexcept { 
         if( obj->state!=1 ){ return; } obj->bff.push( msg );
    }

    void free() const noexcept { 
        if( obj->state == 1 ){ return; } 
            obj->state =  0;
        onClose.emit(); onData.clear();
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); }

};

/*────────────────────────────────────────────────────────────────────────────*/

class decoder_t {
protected:

    struct NODE {
        string_t chr; bool state=0;
        queue_t<string_t> bff;
        BIGNUM* bn =nullptr;
       ~NODE() { if( bn ){ BN_clear_free( bn ); } }
    };  ptr_t<NODE> obj;

    string_t decode( string_t msg ) const noexcept {
        if( msg.empty() ){ return nullptr; }

        BN_zero(obj->bn); ulong lz = 0; ulong ch = true;

        for ( const auto& c : msg ){
        if  ( ch && c == obj->chr[0] ){ lz++; } 
        else{ ch = false; }

            const char* pos = strchr(obj->chr.data(), c);
            if( pos == nullptr ){ return nullptr; }
            
            BN_mul_word(obj->bn, obj->chr.size());
            BN_add_word(obj->bn, pos- obj->chr.data());
        }

        int num_bytes = BN_num_bytes(obj->bn); 
        ptr_t<uchar> tmp ( lz + num_bytes, '\0' );
        BN_bn2bin( obj->bn, tmp.data() + lz );

        string_t out( (char*)tmp.data(),tmp.size() );
        if( !onData.empty() ){ onData.emit( out ); }

    return out; }

public:

    event_t<string_t> onData;
    event_t<>         onClose;

    decoder_t( const string_t& chr ) : obj( new NODE() ) { 
        obj->state = 1; obj->chr = chr; obj->bn = (BIGNUM*) BN_new();
        if( !obj->bn ){ throw except_t("can't initializate decoder"); }
    }
    
   ~decoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    void update( const string_t& msg ) const noexcept { 
         if( obj->state!=1 ){ return; } obj->bff.push( msg );
    }

    string_t get() const noexcept { if( obj->state == 0 ){ return nullptr; }
        auto raw = array_t<string_t>( obj->bff.data() ).join(nullptr);
        auto data= decode( raw ); free(); return data; 
    }

    void free() const noexcept { 
        if( obj->state == 1 ){ return; } 
            obj->state = 0;
        onClose.emit(); onData.clear();
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
        queue_t<string_t> buff;
        ptr_t<char> bff;
        ptr_t<CTX>  ctx;
        bool    state=0;
    };  ptr_t<NODE> obj;

public:

    event_t<>         onClose;
    event_t<string_t> onData;

   ~base64_encoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    base64_encoder_t() noexcept : obj( new NODE() ) {
        obj->state = 1; obj->bff = ptr_t<char>( CHUNK_SIZE, '\0' );

        CTX item1; memset( &item1, 0, sizeof(CTX) );
            item1.pos1 = 0; item1.pos2 =-6; 
            item1.size = 0; item1.len  = 0;

        obj->ctx = type::bind( item1 );
    }

    void update( string_t msg ) const noexcept { 
        if( !obj->state ){ return; } ulong chunk=0, /*--------*/ base=obj->bff.size();
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
            string_t out; obj->ctx->len = 0; forEach ( x, tmp ) {

                obj->ctx->pos1 = ( obj->ctx->pos1 << 8 ) + x; obj->ctx->pos2 += 8;

                while ( obj->ctx->pos2 >= 0 ) { 
                    obj->bff[obj->ctx->len] = CRYPTO_BASE64[(obj->ctx->pos1>>obj->ctx->pos2)&0x3F];
                    obj->ctx->pos2 -= 6; ++obj->ctx->len;
                }

            }   obj->ctx->size += obj->ctx->len; out = string_t( &obj->bff, obj->ctx->len );

            if ( obj->ctx->len == 0 ){ return; }
            if ( onData.empty()     ){ obj->buff.push( out ); } else { onData.emit( out ); }
        chunk += base; }
    }

    void free() const noexcept { if ( obj->state == 0 ){ return; } 
        string_t out; obj->state = 0; obj->ctx->len = 0;

        if( obj->ctx->pos2 > -6 ){ 
            obj->bff[obj->ctx->len] = CRYPTO_BASE64[((obj->ctx->pos1<<8)>>(obj->ctx->pos2+8))&0x3F];
            obj->ctx->len++; 
        } while ( ( obj->ctx->len + obj->ctx->size ) % 4 ){ 
            obj->bff[obj->ctx->len] = '='; 
            obj->ctx->len++;
        } 

        obj->ctx->size += obj->ctx->len; out = string_t( &obj->bff, obj->ctx->len );
        if ( onData.empty() ) { obj->buff.push( out ); } else { onData.emit(out); }
             onClose.emit(); onData.clear();
    }

    string_t get() const noexcept { free(); return array_t<string_t>( obj->buff.data() ).join(nullptr); }

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
        queue_t<string_t> buff;
        ptr_t<char> bff;
        ptr_t<CTX>  ctx;
        bool    state=0;
    };  ptr_t<NODE> obj;

public:

    event_t<>         onClose;
    event_t<string_t> onData;

   ~base64_decoder_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    base64_decoder_t() noexcept : obj( new NODE() ) {
        obj->state = 1; obj->bff = ptr_t<char>( CHUNK_SIZE, '\0' );

        CTX item1; memset( &item1, 0, sizeof(CTX) );
            item1.pos1 = 0; item1.pos2 =-8; 
            item1.size = 0; item1.len  = 0;

        obj->ctx = type::bind( item1 );
    }

    void update( string_t msg ) const noexcept { 
        if( !obj->state ){ return; } ulong chunk=0, /*--------*/ base=obj->bff.size();
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
        for  ( int x=0; x<64; x++ ){ obj->ctx->T[type::cast<int>(CRYPTO_BASE64[x])] =x; }

            string_t out; obj->ctx->len = 0; forEach ( x, tmp ) {
                uint   y = type::cast<uint>(x);

                if( obj->ctx->T[y]==-1 ){ break; }

                obj->ctx->pos1 = ( obj->ctx->pos1 << 6 ) + obj->ctx->T[y]; obj->ctx->pos2 += 6;

                if( obj->ctx->pos2 >= 0 ) {
                    obj->bff[obj->ctx->len] = char((obj->ctx->pos1>>obj->ctx->pos2)&0xFF);
                    obj->ctx->pos2 -= 8; ++obj->ctx->len;
                }

            }   obj->ctx->size += obj->ctx->len; out = string_t( &obj->bff, obj->ctx->len );

            if ( obj->ctx->len == 0 ){ return; }
            if ( onData.empty()     ){ obj->buff.push(out); } else { onData.emit( out ); }
        chunk += base; }
    }

    void free() const noexcept { 
    if( obj->state == 0 ){ return; } 
        obj->state =  0; onClose.emit(); onData.clear();
    }

    string_t get() const noexcept { free(); return array_t<string_t>( obj->buff.data() ).join(nullptr); }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

};

/*────────────────────────────────────────────────────────────────────────────*/

class X509_t {
protected:

    static int PASS_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size );
                buf[ size - 1 ] = '\0';
        return strlen(buf);
    }

    struct NODE {
        X509_NAME* name = nullptr;
        EVP_PKEY*  pkey = nullptr;
        X509*       ctx = nullptr;
        bool      state = 0;

       ~NODE() { 
            if( ctx  ){ X509_free     ( ctx  ); }
            if( pkey ){ EVP_PKEY_free ( pkey ); }
            if( name ){ X509_NAME_free( name ); }
        }

    };  ptr_t<NODE> obj;

public:

    X509_t( int curve_nid = NID_X9_62_prime256v1 ) : obj( new NODE() ) {
        
        obj->ctx = X509_new(); obj->name= X509_NAME_new();  
        /*------------------*/ obj->pkey= EVP_PKEY_new();
        
        EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
        EVP_PKEY_keygen_init(pctx);
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, curve_nid);
        
        EVP_PKEY_keygen(pctx, &obj->pkey);
        EVP_PKEY_CTX_free(pctx);

        obj->state = 1; if( !obj->ctx || !obj->pkey ) 
        { throw except_t("can't initializate X509_t"); }

    }

   ~X509_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    X509*     get_cert() const noexcept { return obj->ctx;  }

    EVP_PKEY* get_pkey() const noexcept { return obj->pkey; }

    int generate( string_t _name, string_t _contry, string_t _organization, ulong _time=31536000L ) const {

        X509_set_version( obj->ctx, 2 ); ASN1_INTEGER_set( X509_get_serialNumber(obj->ctx), 1 );
        
        X509_NAME_add_entry_by_txt( obj->name, "O",  MBSTRING_ASC, (uchar*) _organization.get(), -1, -1, 0);
        X509_NAME_add_entry_by_txt( obj->name, "C",  MBSTRING_ASC, (uchar*) _contry      .get(), -1, -1, 0);
        X509_NAME_add_entry_by_txt( obj->name, "CN", MBSTRING_ASC, (uchar*) _name        .get(), -1, -1, 0);
        
        X509_set_subject_name( obj->ctx, obj->name ); 
        X509_set_issuer_name ( obj->ctx, obj->name );

        if( _time != 0 ){
            X509_gmtime_adj( X509_getm_notBefore(obj->ctx), 0     );
            X509_gmtime_adj( X509_getm_notAfter (obj->ctx), _time );
        }   X509_set_pubkey( obj->ctx, obj->pkey ); 

        if( !X509_sign( obj->ctx, obj->pkey, EVP_sha256() ) ){ return -1; }

    return 1; }

    string_t write_private_key_to_memory( const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        
        PEM_write_bio_PrivateKey( 
            bo, obj->pkey, pass ? EVP_aes_256_cbc(): NULL, 
            (uchar*)pass, pass ? (int)strlen(pass) : 0   , 
            NULL, NULL 
        );

        long len = BIO_get_mem_data( bo, &data ); 
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    string_t write_certificate_to_memory() const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_X509( bo, obj->ctx );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    int write_private_key( const string_t& path, const char* pass=NULL ) const {
        if( fs::write_file( path, write_private_key_to_memory( pass ) ).await() )
          { return -1; } return 1;
    }

    int write_certificate( const string_t& path ) const {
        if( fs::write_file( path, write_certificate_to_memory() ).await() )
          { return -1; } return 1;
    }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } 
            obj->state = 0; 
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

class rsa_t {
protected:

    static int PASS_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size );
                buf[ size - 1 ] = '\0';
        return strlen(buf);
    }

    struct NODE {
        RSA*    rsa = nullptr;
        BIGNUM* num = nullptr;
        ptr_t<uchar> bff;
        bool  state = 0;

       ~NODE() {
            if( num ){ BN_free ( num ); }
            if( rsa ){ RSA_free( rsa ); }
        }

    };  ptr_t<NODE> obj;
    
public:

    rsa_t() : obj( new NODE() ) {
        obj->rsa = RSA_new(); obj->num = BN_new (); obj->state = 1;
        if( !obj->num || !obj->rsa ){ throw except_t("creating rsa object"); }
    }

   ~rsa_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    RSA* get_fd() const noexcept { return obj->rsa; }

    int generate_keys( uint len=2048 ) const noexcept {
        len = clamp( len, 1024u, 4098u ); BN_set_word( obj->num, RSA_F4 );
        int c = RSA_generate_key_ex( obj->rsa, len, obj->num, NULL );
        obj->bff.resize( RSA_size(obj->rsa) ); return c;
    }

    int read_private_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_RSAPrivateKey( bo, &obj->rsa, &PASS_CLB, (void*)pass ) ){
            BIO_free(bo); return -1;
        }   BIO_free(bo); obj->bff.resize(RSA_size(obj->rsa));
    return 1; }

    int read_public_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_RSAPublicKey( bo, &obj->rsa, &PASS_CLB, (void*)pass ) ){
            BIO_free(bo); return -1;
        }   BIO_free(bo); obj->bff.resize(RSA_size(obj->rsa));
    return 1; }

    string_t write_private_key_to_memory( const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_RSAPrivateKey( bo, obj->rsa, NULL, NULL, 0, &PASS_CLB, (void*)pass );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    string_t write_public_key_to_memory( const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); char* data;
        PEM_write_bio_RSAPublicKey( bo, obj->rsa );
        long len = BIO_get_mem_data( bo, &data );
        string_t res ( data, len );
        BIO_free(bo); return res;
    }

    int read_private_key( const string_t& path, const char* pass=NULL ) const {
        auto raw = fs::read_file( path ).await(); if( !raw ){ return -1; }
        read_private_key_from_memory( raw.value(), pass );
    return 1; }

    int write_private_key( const string_t& path, const char* pass=NULL ) const {
        if( !fs::write_file( path, write_private_key_to_memory( pass ) ).await() )
          { return -1; } return 1;
    }

    int read_public_key( const string_t& path, const char* pass=NULL ) const {
        auto raw = fs::read_file( path ).await(); if( !raw ){ return -1; }
        read_public_key_from_memory( raw.value(), pass );
    return 1; }

    int write_public_key( const string_t& path, const char* pass=NULL ) const {
        if( !fs::write_file( path, write_public_key_to_memory( pass ) ).await() )
          { return -1; } return 1;
    }

    string_t public_encrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if( msg.empty() || obj->state==0 || obj->rsa == nullptr ){ return nullptr; }

        ulong chunk=0, base=(ulong)( obj->bff.size() ); string_t data;
        
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
            int c = RSA_public_encrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        chunk += base; } return data;
    }

    /*
    string_t private_encrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if( msg.empty() || obj->state==0 || obj->rsa == nullptr ){ return nullptr; }

        ulong chunk=0, base=(ulong)( obj->bff.size() ); string_t data;
        
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
            int c = RSA_private_encrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        chunk += base; } return data;
    }

    string_t public_decrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if( msg.empty() || obj->state==0 || obj->rsa == nullptr ){ return nullptr; }

        ulong chunk=0, base=(ulong)( obj->bff.size() ); string_t data;
        
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
            int c = RSA_public_decrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        chunk += base; } return data;
    }
    */

    string_t private_decrypt( string_t msg, int padding=RSA_PKCS1_PADDING ) const {
        if( msg.empty() || obj->state==0 || obj->rsa == nullptr ){ return nullptr; }

        ulong chunk=0, base=(ulong)( obj->bff.size() ); string_t data;
        
        while( chunk < msg.size() ){ auto tmp = msg.slice_view( chunk, chunk + base );
            int c = RSA_private_decrypt( tmp.size(), (uchar*)tmp.data(), &obj->bff, obj->rsa, padding );
            data += string_t( (char*) &obj->bff, (ulong)c );
        chunk += base; } return data;
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool is_closed() const noexcept { return obj->state == 0; }

    void close() const noexcept { free(); } 

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } 
            obj->state =  0;
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

       ~NODE() {
            if( priv_key  ){ BN_free      ( priv_key ); }
        //  if( key_pair  ){ EC_KEY_free  ( key_pair ); }
            if( pub_key   ){ EC_POINT_free( pub_key ); }
            if( key_group ){ EC_GROUP_free( key_group ); }
        }

    };  ptr_t<NODE> obj;
    
public:

    template< class T >
    ec_t( const string_t& key, const T& type ) noexcept :obj( new NODE() ) {
        if( key.empty() ){ throw except_t("can't initializate ec_t"); }

        obj->state     = 1;
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
    ec_t( const T& type ) noexcept : obj( new NODE() ) { 
        obj->state = 1;

        obj->key_pair  = EC_KEY_new();
        obj->key_group = EC_GROUP_new_by_curve_name( type );

        EC_KEY_set_group( obj->key_pair, obj->key_group );
        EC_KEY_generate_key( obj->key_pair );

        obj->pub_key  = (EC_POINT*) EC_KEY_get0_public_key( obj->key_pair );
        obj->priv_key = (BIGNUM*)  EC_KEY_get0_private_key( obj->key_pair );
    }
    
   ~ec_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    string_t get_public_key( uint x = 0 ) const noexcept { 
        if( !obj->state ){ return nullptr; }
        point_conversion_form_t y; switch( x ){
            case 0:  y = POINT_CONVERSION_HYBRID;       break;
            case 1:  y = POINT_CONVERSION_COMPRESSED;   break;
            default: y = POINT_CONVERSION_UNCOMPRESSED; break;
        }   return EC_POINT_point2hex( obj->key_group, obj->pub_key, y, nullptr );
    }

    string_t get_private_key() const noexcept { return BN_bn2hex( obj->priv_key ); }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } 
            obj->state =  0;
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

       ~NODE() {
            if( dh ){ DH_free( dh ); }
            if( k  ){ BN_free( k  ); }
        }

    };  ptr_t<NODE> obj;

public:

    dh_t() : obj( new NODE() ) {
        obj->dh = DH_new(); obj->k = BN_new(); obj->state = 1;
        if( !obj->dh || !obj->k ){ throw except_t( "creating new dh" ); }
    }

   ~dh_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    int generate_keys( int len=512 ) const noexcept {
        if( !DH_generate_parameters_ex( obj->dh, len, DH_GENERATOR_2, NULL ) )
          { return -1; } if( !DH_generate_key( obj->dh ) )
          { return -1; } return 1;
    }

    int set_public_key( const string_t& key ) const noexcept {
        if( !obj->state ){ return 0; }
               BN_hex2bn( &obj->k, key.data() );
        return DH_set0_key( obj->dh, nullptr, obj->k );
    }

    int set_private_key( const string_t& key ) const noexcept {
        if( !obj->state ){ return 0; }
               BN_hex2bn( &obj->k, key.data() );
        return DH_set0_key( obj->dh, obj->k, nullptr );
    }

    string_t get_private_key() const noexcept {
        if( !obj->state ){ return nullptr; } 
        return BN_bn2hex( DH_get0_priv_key( obj->dh ) );
    }

    string_t get_public_key() const noexcept {
        if( !obj->state ){ return nullptr; } 
        return BN_bn2hex( DH_get0_pub_key( obj->dh ) );
    }

    void free() const noexcept {
        if( obj->state == 0 ){ return; } 
            obj->state =  0;
    }

    bool verify( const string_t& hex, const string_t& sgn ) const {
         return sign( hex ) == sgn;
    }

    string_t sign( const string_t& hex ) const {
        if( !obj->state ) /*----------------*/ { return nullptr; } 
        ptr_t<uchar> shared( DH_size( obj->dh ));
        if( !BN_hex2bn( &obj->k,hex.data() )  ){ return nullptr; }
        int len = DH_compute_key( &shared, obj->k, obj->dh );
        return encoder::buffer::buff2hex( string_t( (char*) &shared, (ulong) len ) );
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

class dsa_t {
protected:

    static int PASS_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size );
                buf[ size - 1 ] = '\0';
        return strlen(buf);
    }

    struct NODE {
        DSA *dsa = nullptr; bool state = 0;
       ~NODE() { if( dsa ) DSA_free( dsa ); }
    };  ptr_t<NODE> obj;
    
public:

    dsa_t(): obj( new NODE() ) { obj->state = 1; obj->dsa = DSA_new(); }

   ~dsa_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    int generate_keys( uint len=512 ) const noexcept {
        if(!DSA_generate_parameters_ex( obj->dsa, len, NULL, 0, NULL, NULL, NULL ) )
          { return -1; } if(!DSA_generate_key( obj->dsa ) )
          { return -1; } return 1;
    }

    bool verify( const string_t& msg, const string_t& sgn ) const noexcept { 
         if( !obj->state || obj->dsa == nullptr ){ return false; } auto ngs = encoder::buffer::hex2buff( sgn ); 
         return DSA_verify( 0, (uchar*)msg.data(), msg.size(), (uchar*)ngs.data(), ngs.size(), obj->dsa )>0;
    }

    string_t sign( const string_t& msg ) const noexcept {
        if( !obj->state || obj->dsa == nullptr ){ return nullptr; }
        ptr_t<uchar> sgn( DSA_size(obj->dsa) ); uint len;
        DSA_sign( 0,(uchar*)msg.data(), msg.size(),&sgn, &len, obj->dsa );
        return encoder::buffer::buff2hex( string_t( (char*) &sgn, (ulong) len ) );
    }

    int read_private_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_DSAPrivateKey( bo, &obj->dsa, &PASS_CLB, (void*)pass ) )
          { BIO_free(bo); return -1; } BIO_free(bo); return 1;
    }

    int read_public_key_from_memory( const string_t& key, const char* pass=NULL ) const {
        BIO* bo = BIO_new( BIO_s_mem() ); BIO_write( bo, key.get(), key.size() );
        if( !PEM_read_bio_DSA_PUBKEY( bo, &obj->dsa, &PASS_CLB, (void*)pass ) )
          { BIO_free(bo); return -1; } BIO_free(bo); return 1;
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

    int read_private_key( const string_t& path, const char* pass=NULL ) const {
        auto raw = fs::read_file( path ).await(); if( !raw ){ return -1; }
        read_private_key_from_memory( raw.value(), pass );
    return 1; }

    int write_private_key( const string_t& path, const char* pass=NULL ) const {
        if( !fs::write_file( path, write_private_key_to_memory( pass ) ).await() )
          { return -1; } return 1;
    }

    int read_public_key( const string_t& path, const char* pass=NULL ) const {
        auto raw = fs::read_file( path ).await(); if( !raw ){ return -1; }
        read_public_key_from_memory( raw.value(), pass );
    }

    int write_public_key( const string_t& path ) const {
        if( !fs::write_file( path, write_public_key_to_memory() ).await() )
          { return -1; } return 1;
    }

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } 
            obj->state =  0;
    }

    bool is_available() const noexcept { return obj->state == 1; }

    bool    is_closed() const noexcept { return obj->state == 0; }

    void        close() const noexcept { free(); } 
    
};

/*────────────────────────────────────────────────────────────────────────────*/

namespace crypto { namespace hash {

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

    class RSA : public rsa_t { public: template< class... T > 
          RSA ( const T&... args ) : rsa_t( args... ) {}
    };

    /*─······································································─*/
    
    class XOR : public xor_t { public: template< class... T >
          XOR ( const T&... args ) : xor_t( args... ) {}
    };

    /*─······································································─*/
    
    class DES_CBC : public encrypt_t { public: template< class... T >
          DES_CBC ( const T&... args ) : encrypt_t( args..., EVP_des_ede_cbc() ) {}
    };
    
    class DES_ECB : public encrypt_t { public: template< class... T >
          DES_ECB ( const T&... args ) : encrypt_t( args..., EVP_des_ede_ecb() ) {}
    };

    /*─······································································─*/
    
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
    
    class TRIPLE_DES_CBC : public encrypt_t { public: template< class... T >
          TRIPLE_DES_CBC ( const T&... args ) : encrypt_t( args..., EVP_des_ede3_cbc() ) {}
    };
    
    class TRIPLE_DES_ECB : public encrypt_t { public: template< class... T >
          TRIPLE_DES_ECB ( const T&... args ) : encrypt_t( args..., EVP_des_ede3_ecb() ) {}
    };

}}
    
    /*─······································································─*/

namespace crypto { namespace decrypt {

    class RSA : public rsa_t { public: template< class... T > 
          RSA ( const T&... args ) : rsa_t( args... ) {}
    };

    /*─······································································─*/
    
    class XOR : public xor_t { public: template< class... T >
          XOR ( const T&... args ) : xor_t( args... ) {}
    };

    /*─······································································─*/
    
    class DES_CBC : public decrypt_t { public: template< class... T >
          DES_CBC ( const T&... args ) : decrypt_t( args..., EVP_des_ede_cbc() ) {}
    };
    
    class DES_ECB : public decrypt_t { public: template< class... T >
          DES_ECB ( const T&... args ) : decrypt_t( args..., EVP_des_ede_ecb() ) {}
    };

    /*─······································································─*/
    
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
    
    class TRIPLE_DES_CBC : public decrypt_t { public: template< class... T >
          TRIPLE_DES_CBC ( const T&... args ) : decrypt_t( args..., EVP_des_ede3_cbc() ) {}
    };
    
    class TRIPLE_DES_ECB : public decrypt_t { public: template< class... T >
          TRIPLE_DES_ECB ( const T&... args ) : decrypt_t( args..., EVP_des_ede3_ecb() ) {}
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