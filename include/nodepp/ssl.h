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
#include "crypto.h"
#include "fs.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class ssl_t { 
protected:
    
    using onSNI = function_t<ssl_t*,string_t>;

    struct NODE {
        int          tpy = SSL_FILETYPE_PEM;
        string_t     key, crt, cha;
        SSL_CTX*     ctx = nullptr;
        SSL*         ssl = nullptr;
        bool         srv = 0;
        bool         cnn = 0;
        bool         stt = 1;
        ptr_t<X509_t>cert;
        ptr_t<onSNI> fnc;
    };  ptr_t<NODE>  obj;
    
    /*─······································································─*/

    SSL_CTX* create_server_context() const noexcept {
        const SSL_METHOD *method; method = TLS_server_method();
        SSL_CTX* ctx = SSL_CTX_new( method );
        SSL_CTX_set_read_ahead( ctx, 1 );
        SSL_CTX_set_timeout( ctx, 0 );
        return ctx;
    }
    
    SSL_CTX* create_client_context() const noexcept {
        const SSL_METHOD *method; method = TLS_client_method();
        SSL_CTX* ctx = SSL_CTX_new( method ); 
        SSL_CTX_set_read_ahead( ctx, 1 );
        SSL_CTX_set_timeout( ctx, 0 );
        return ctx;
    }


    static int SNI_CLB ( char *buf, int size, int rwflag, void *args ) {
        if( args == nullptr || rwflag != 1 ){ return -1; }
        strncpy( buf, (char*)args, size );
                buf[ size - 1 ] = '\0';
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
        if( !SSL_CTX_check_private_key(ctx) )                            { x = 0; goto DONE; }
          } else { x = 0; }
        
        DONE:; return x==1 ? 1 : -1;
    }
    
    /*─······································································─*/

    static int servername_handler( SSL* ssl, int* /*unused*/, void* arg ) {
        const char* servername = SSL_get_servername( ssl, TLSEXT_NAMETYPE_host_name );
        onSNI func = *((onSNI*)arg); if( servername ){ ssl_t* xtc = func(servername); 
            if( xtc != nullptr ){ xtc->create_server();
                SSL_CTX* v = SSL_set_SSL_CTX( ssl, xtc->get_ctx() );
                if( v != xtc->get_ctx() ){ return SSL_TLSEXT_ERR_NOACK; }
            }
        }   return SSL_TLSEXT_ERR_OK;
    }
    
    /*─······································································─*/

    bool is_blocked( const int& c ) const noexcept { if( c<=0 ){
    int error =  SSL_get_error( obj->ssl, c ); ERR_clear_error(); return ( 
        error == SSL_ERROR_WANT_CLIENT_HELLO_CB ||
        error == SSL_ERROR_WANT_X509_LOOKUP     ||
        error == SSL_ERROR_WANT_ASYNC_JOB       ||
        error == SSL_ERROR_WANT_CONNECT         ||
        error == SSL_ERROR_WANT_ACCEPT          ||
        error == SSL_ERROR_WANT_WRITE           ||
        error == SSL_ERROR_WANT_ASYNC           ||
        error == SSL_ERROR_WANT_READ
    );} return 0; }
    
    /*─······································································─*/

    void set_ctx_sni( SSL_CTX* ctx, onSNI* func ) const noexcept {
         SSL_CTX_set_tlsext_servername_callback( ctx, servername_handler );
         SSL_CTX_set_tlsext_servername_arg( ctx, func );
    }
    
    /*─······································································─*/

    void set_nonbloking_mode() const noexcept { SSL_set_mode( obj->ssl, 
         SSL_MODE_ASYNC | SSL_MODE_AUTO_RETRY |
         SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER  |
         SSL_MODE_ENABLE_PARTIAL_WRITE        |
         SSL_MODE_RELEASE_BUFFERS
    );}

public:
    
    virtual ~ssl_t() { if( obj.count() > 1 ) { return; } free(); }
    
    /*─······································································─*/

    ssl_t( const string_t& _key, const string_t& _cert, const string_t& _chain ) : obj( new NODE() ) {
        if(!fs::exists_file(_key) || !fs::exists_file(_cert) || !fs::exists_file(_chain) )
          { throw except_t("such key, cert or chain does not exist"); } 
            obj->key = _key;  obj->crt = _cert; obj->cha = _chain;
    }
    
    /*─······································································─*/

    ssl_t( const string_t& _key, const string_t& _cert ) : obj( new NODE() ) { 
        if(!fs::exists_file(_key) || !fs::exists_file(_cert) )
          { throw except_t("such key or cert does not exist"); }
            obj->key = _key;  obj->crt = _cert; 
    }

    /*─······································································─*/

    ssl_t( ssl_t xtc, int df ) : obj( new NODE() ) { 
       if( xtc.get_ctx() == nullptr ){ throw except_t("ctx has no context"); }
           obj->ctx = xtc.get_ctx(); obj->ssl = SSL_new(obj->ctx); 
           obj->srv = xtc.is_server(); set_nonbloking_mode(); 
           set_fd( df );
    }

    /*─······································································─*/

    ssl_t() : obj( new NODE() ) {  
        static ptr_t<X509_t> cert; if( cert.null() ){
            cert = type::bind /*-*/ ( new X509_t() ); 
            cert->generate( "Node", "Node", "Node" );
        }   /*-------------------*/ obj->cert = cert;
    }
    
    /*─······································································─*/

    void set_sni_callback( onSNI callback ){ obj->fnc = type::bind( callback ); }
    
    /*─······································································─*/

    int set_fd( int df ) noexcept { return obj->ssl==nullptr ? -1 : SSL_set_fd ( obj->ssl, df ); }
    int get_rfd()  const noexcept { return obj->ssl==nullptr ? -1 : SSL_get_rfd( obj->ssl ); }
    int get_wfd()  const noexcept { return obj->ssl==nullptr ? -1 : SSL_get_wfd( obj->ssl ); }
    int  get_fd()  const noexcept { return obj->ssl==nullptr ? -1 : SSL_get_fd ( obj->ssl ); }
    
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
        if( obj->fnc != nullptr ){ set_ctx_sni( obj->ctx, &obj->fnc ); } return res;
    }
    
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

    inline int _accept() const noexcept { if( !obj->stt ){ return -1; }
        if( obj->ssl == nullptr ){ return -1; }
        int c = SSL_accept( obj->ssl );
        if( c > 0 ){ obj->cnn = 1; }
        return is_blocked(c) ? -2 : c;
    }
    
    /*─······································································─*/

    inline int _connect() const noexcept { if( !obj->stt ){ return -1; }
        if( obj->ssl == nullptr ){ return -1; } 
        int c = SSL_connect( obj->ssl );
        if( c > 0 ){ obj->cnn = 1; }
        return is_blocked(c) ? -2 : c;
    }
    
    /*─······································································─*/

    int accept() const noexcept { int c = 0;
        while(( c=_accept() )==-2 ){ process::next(); } return c;
    }
    
    /*─······································································─*/

    int connect() const noexcept { int c = 0;
        while(( c=_connect() )==-2 ){ process::next(); } return c;
    }
    
    /*─······································································─*/

    ulong read( char* buffer, const ulong& size ) const noexcept { int c = 0;
        while(( c=_read(buffer,size) )==-2 ){ process::next(); } return c;
    }
    
    ulong write( char* buffer, const ulong& size ) const noexcept { int c = 0;
        while(( c=_write(buffer,size) )==-2 ){ process::next(); } return c;
    }
    
    /*─······································································─*/

    virtual int _read ( char* bf, ulong sx ) const noexcept { return __read ( bf, sx ); }
    
    virtual int _write( char* bf, ulong sx ) const noexcept { return __write( bf, sx ); }
    
    /*─······································································─*/

    virtual int __read( char* bf, ulong sx ) const noexcept { 
        if( !obj->stt || obj->ssl == nullptr ){ return -1; } int c = 0;
        if( obj->cnn == 0 ){ while( _accept()==-2 ){ return -2; } }
        return is_blocked( c=SSL_read( obj->ssl, bf, sx ) ) ? -2 : c;
    }
    
    virtual int __write( char* bf, ulong sx ) const noexcept {
        if( !obj->stt || obj->ssl == nullptr ){ return -1; } int c = 0;
        if( obj->cnn == 0 ){ while( _accept()==-2 ){ return -2; } }
        return is_blocked( c=SSL_write( obj->ssl, bf, sx ) ) ? -2 : c;
    }

    /*─······································································─*/

    bool _write_( char* bf, const ulong& sx, ulong& sy ) const noexcept {
        if( !obj->stt || obj->ssl == nullptr ){ return -1; } while( sy < sx ) {
            int c = __write( bf+sy, sx-sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ sy += c; continue; } return 1;
        }   return 0;
    }

    bool _read_( char* bf, const ulong& sx, ulong& sy ) const noexcept {
        if( !obj->stt || obj->ssl == nullptr ){ return -1; } while( sy < sx ) {
            int c = __read( bf+sy, sx-sy );
            if( c <= 0 && c != -2 ) /*----*/ { return 0; }
            if( c >  0 ){ sy += c; continue; } return 1;
        }   return 0;
    }
    
    /*─······································································─*/

    virtual void free() const noexcept {
        if( obj->ssl != nullptr && obj->stt ){
        if( obj->cnn == 1 )
          { SSL_shutdown( obj->ssl ); }   
            SSL_clear( obj->ssl ); SSL_free( obj->ssl ); 
        goto CLSE;
        } if ( obj->ctx != nullptr && obj->stt ){
            SSL_CTX_free( obj->ctx );
        }   CLSE:; obj->stt = false;
    }
    
} SSL_DEFAULT_CERT; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif
