/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SSL
#define NODEPP_SSL
#define OPENSSL_API_COMPAT 0x10100000L

/*────────────────────────────────────────────────────────────────────────────*/

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "generator.h"
#include "crypto.h"
#include "fs.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class ssl_t { 
protected:
    
    using onSNI = function_t<ssl_t*,string_t>;
    struct NODE {
        int          tpy = SSL_FILETYPE_PEM;
        generator::ssl::pipe is_blocked;
        string_t     key, crt, cha;
        SSL_CTX*     ctx = nullptr;
        SSL*         ssl = nullptr;
        BIO*         rbio= nullptr;
        BIO*         wbio= nullptr;
        bool         srv = 0;
        bool         cnn = 0;
        bool         stt = 1;
        ptr_t<X509_t>cert;
        ptr_t<onSNI> fnc;
    };  ptr_t<NODE>  obj;
    
    /*─······································································─*/

    void set_ctx_options( SSL_CTX* ctx ) const noexcept {
         SSL_CTX_set_options( ctx, SSL_OP_ALL | SSL_OP_NO_RENEGOTIATION | SSL_OP_IGNORE_UNEXPECTED_EOF );
    }

    SSL_CTX* create_server_context() const noexcept {
        const SSL_METHOD *method; method = TLS_server_method();
        SSL_CTX* ctx = SSL_CTX_new( method );
        SSL_CTX_set_read_ahead( ctx, 1 );
        SSL_CTX_set_timeout( ctx, 0 );
        set_ctx_options( ctx );
        return ctx;
    }
    
    SSL_CTX* create_client_context() const noexcept {
        const SSL_METHOD *method; method = TLS_client_method();
        SSL_CTX* ctx = SSL_CTX_new( method ); 
        SSL_CTX_set_read_ahead( ctx, 1 );
        SSL_CTX_set_timeout( ctx, 0 );
        set_ctx_options( ctx );
        return ctx;
    }

    static int SNI_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size ); buf[ size -1 ] = '\0';
        return strlen(buf);
    }
    
    /*─······································································─*/

    int configure_context( SSL_CTX* ctx, const string_t& key, const string_t& crt, const string_t& cha ) const noexcept { 
    int x = 1; 

        if( !cha.empty() && x==1 ){ x=SSL_CTX_use_certificate_chain_file( ctx, (char*)cha ); }
        if( !crt.empty() && x==1 ){ x=SSL_CTX_use_certificate_file      ( ctx, (char*)crt, obj->tpy ); }
        if( !key.empty() && x==1 ){ x=SSL_CTX_use_PrivateKey_file       ( ctx, (char*)key, obj->tpy ); }

        if( obj->cert != nullptr && x==1 ){
        if( !SSL_CTX_use_certificate(ctx,obj->cert->get_cert()) || !ctx ){ x = 0; goto DONE; }
        if( !SSL_CTX_use_RSAPrivateKey(ctx,obj->cert->get_prv()) )       { x = 0; goto DONE; } 
        if( !SSL_CTX_check_private_key(ctx) )                            { x = 0; goto DONE; }} else { x = 0; }
        
        DONE:; return x==1 ? 1 : -1;
    }
    
    /*─······································································─*/

    static int servername_handler( SSL* ssl, int* ad, void* arg ) {
        const char* servername = SSL_get_servername( ssl, TLSEXT_NAMETYPE_host_name );
        onSNI func = *((onSNI*)arg); 
        
        if( servername ){ 
            ssl_t* xtc = func(servername); 
        if( xtc != nullptr ){ 
            SSL_set_SSL_CTX( ssl, xtc->get_ctx() );
        } else { 
            *ad  = SSL_AD_UNRECOGNIZED_NAME;
            return SSL_TLSEXT_ERR_ALERT_FATAL;
        }}  

        return SSL_TLSEXT_ERR_OK;
    }
    
    /*─······································································─*/

    template< class T >
    int connect( T* stream ) const noexcept {
    int c =obj->srv ? SSL_accept( obj->ssl ) : SSL_connect( obj->ssl );
        c = is_blocked( stream, c ) ? -2 : c; 
        if( c== 1 ){ obj->cnn =1; } return c;
    }

    template< class T >
    bool is_blocked( T* stream, int& c ) const noexcept { 
    return stream->is_closed() ? -1 : obj->is_blocked( obj, stream, c )==-1 ? 0 : 1; }
    
    /*─······································································─*/

    void set_ctx_sni( SSL_CTX* ctx, onSNI* func ) const noexcept {
         SSL_CTX_set_tlsext_servername_callback( ctx, servername_handler );
         SSL_CTX_set_tlsext_servername_arg( ctx, func );
    }
    
    /*─······································································─*/

    void set_nonbloking_mode() const noexcept { 
         SSL_set_quiet_shutdown( obj->ssl, 1 ); SSL_set_mode( obj->ssl, 
         SSL_MODE_ASYNC | SSL_MODE_AUTO_RETRY |
         SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER  |
         SSL_MODE_ENABLE_PARTIAL_WRITE        |
         SSL_MODE_RELEASE_BUFFERS            );
    }

public:
    
    /*─······································································─*/

    ssl_t( const string_t& _key, const string_t& _cert, const string_t& _chain ) : obj( new NODE() ) {
       if(!fs::exists_file(_key) || !fs::exists_file(_cert) || !fs::exists_file(_chain) )
         { throw except_t("such key, cert or chain does not exist"); } 
           obj->key = _key;  obj->crt = _cert; obj->cha = _chain;
    }
    
   ~ssl_t() { if( obj.count() > 1 ) { return; } free(); }
    
    /*─······································································─*/

    ssl_t( const string_t& _key, const string_t& _cert ) : obj( new NODE() ) { 
       if(!fs::exists_file(_key) || !fs::exists_file(_cert) )
         { throw except_t("such key or cert does not exist"); }
           obj->key = _key; obj->crt = _cert; 
    }

    /*─······································································─*/

    ssl_t( ssl_t xtc, int /*unused*/ ) : obj( new NODE() ) { 
       if( xtc.get_ctx() == nullptr ){ throw except_t("ctx has no context"); }
        
        obj->ctx = xtc.get_ctx(); 
        obj->srv = xtc.is_server(); 
        obj->ssl = SSL_new(obj->ctx);

        SSL_CTX_up_ref( obj->ctx );

        obj->rbio = BIO_new(BIO_s_mem());
        obj->wbio = BIO_new(BIO_s_mem());

        BIO_set_nbio( obj->rbio, 1 );
        BIO_set_nbio( obj->wbio, 1 );
        SSL_set_bio ( obj->ssl, obj->rbio, obj->wbio );

        set_nonbloking_mode(); 
    }

    /*─······································································─*/

    ssl_t() : obj( new NODE() ) {
        thread_local static ptr_t< X509_t > cert; if( cert.null() ){
            cert= type::bind/*-*/( X509_t() ); 
            cert->generate("Node","Node","Node");
        }   /*---------------*/ obj->cert = cert;
    }
    
    /*─······································································─*/

    void set_sni_callback( onSNI callback ){ obj->fnc = type::bind( callback ); }
    
    /*─······································································─*/

    void set_ctx_type( int type ) const noexcept { obj->tpy = type; }
    SSL_CTX*  get_ctx()           const noexcept { return obj->ctx; }
    SSL*      get_ssl()           const noexcept { return obj->ssl; }
    bool    is_server()           const noexcept { return obj->srv; }
    
    /*─······································································─*/

    string_t get_key_path() noexcept { return obj->key; }
    string_t get_crt_path() noexcept { return obj->crt; }
    string_t get_cha_path() noexcept { return obj->cha; }
    
    /*─······································································─*/
    
    int create_client() const noexcept { if( !obj->stt ){ return -1; }
        obj->ctx = create_client_context(); obj->srv = 0; 
        return configure_context( obj->ctx, obj->key, obj->crt, obj->cha );
    }

    int create_server() const noexcept { if( !obj->stt ){ return -1; }
        obj->ctx = create_server_context(); obj->srv = 1;
        int  res = configure_context( obj->ctx, obj->key, obj->crt, obj->cha ); 
        if( obj->fnc != nullptr ){ set_ctx_sni( obj->ctx, &obj->fnc ); } 
    return res; }
    
    /*─······································································─*/

    void set_password( const char* pass ) const noexcept {
        if( !obj->stt ){ return; }
        SSL_CTX_set_default_passwd_cb( obj->ctx, &SNI_CLB );
        SSL_CTX_set_default_passwd_cb_userdata( obj->ctx, (void*)pass );
    }

    int set_hostname( const string_t& name ) const noexcept {
        if( !obj->stt ){ return -1; }
        return SSL_set_tlsext_host_name( obj->ssl, name.data() );
    }

    string_t get_hostname() const noexcept {
        if( !obj->stt ){ return nullptr; }
        int type = SSL_get_servername_type( obj->ssl );
        return SSL_get_servername( obj->ssl, type );
    }
    
    /*─······································································─*/

    template< class T >
    ulong read( T* stream, char* buffer, const ulong& size ) const noexcept { int c = 0;
        while(( c=_read( stream, buffer, size ) )==-2 ){ process::next(); } return c;
    }
    
    template< class T >
    ulong write( T* stream, char* buffer, const ulong& size ) const noexcept { int c = 0;
        while(( c=_write( stream, buffer, size ) )==-2 ){ process::next(); } return c;
    }
    
    /*─······································································─*/

    template< class T >
    int _read ( T* stream, char* bf, ulong sx ) const noexcept { 
        return __read ( stream, bf, sx ); 
    }
    
    template< class T >
    int _write( T* stream, char* bf, ulong sx ) const noexcept { 
        return __write( stream, bf, sx ); 
    }
    
    /*─······································································─*/

    template< class T >
    int __read( T* stream, char* bf, ulong sx ) const noexcept { 
        if( !obj->stt || !obj->ssl || stream->is_closed() ){ return -1; }
        if( !obj->cnn && connect( stream )==-2 ) /*-----*/ { return -2; }

        int c=0; bool blk = false;
        
        do { c=SSL_read( obj->ssl, bf, sx ); if( c > 0 ){ return c; }} 
        while((blk=is_blocked(stream,c)) && SSL_pending(obj->ssl)>0 );

        return stream->is_closed() ? -1 : blk ? -2 : c; 
    }
    
    template< class T >
    int __write( T* stream, char* bf, ulong sx ) const noexcept {
        if( !obj->stt || !obj->ssl || stream->is_closed() ){ return -1; }
        if( !obj->cnn && connect( stream )==-2 ) /*-----*/ { return -2; }
        int c =SSL_write( obj->ssl, bf, sx ); 
        return is_blocked( stream, c ) ? -2 : c;
    }

    /*─······································································─*/

    template< class T >
    bool _write_( T* stream, char* bf, const ulong& sx, ulong* sy ) const noexcept {
    if( !obj->stt || !obj->ssl || stream->is_closed() ){ return -1; } while( *sy<sx ) {
        int c = __write( stream, bf + *sy, sx - *sy );
        if( c <= 0 && c != -2 ) /*----*/ { return 0; }
        if( c >  0 ){ *sy+= c; continue; } return 1;
    }   return 0; }

    template< class T >
    bool _read_( T* stream, char* bf, const ulong& sx, ulong* sy ) const noexcept {
    if( !obj->stt || !obj->ssl || stream->is_closed() ){ return -1; } while( *sy<sx ) {
        int c = __read( stream, bf + *sy, sx - *sy );
        if( c <= 0 && c != -2 ) /*----*/ { return 0; }
        if( c >  0 ){ *sy+= c; continue; } return 1;
    }   return 0; }
    
    /*─······································································─*/

    void free() const noexcept {

        if( obj->ssl != nullptr && obj->stt ){
        if( obj->cnn == 1 ){ SSL_shutdown( obj->ssl ); }   
            /*------------*/ SSL_clear( obj->ssl ); 
            /*------------*/ SSL_free ( obj->ssl ); 
        } 

        if( obj->ctx != nullptr && obj->stt ){
            SSL_CTX_free( obj->ctx );
        }

        obj->stt = false;

    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
