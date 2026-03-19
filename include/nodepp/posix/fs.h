/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_FS
#define NODEPP_POSIX_FS

/*────────────────────────────────────────────────────────────────────────────*/

#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace fs {

    inline file_t readable( const string_t& path, const ulong& _size=CHUNK_SIZE ){ return file_t( path, "r", _size ); }
    inline file_t writable( const string_t& path, const ulong& _size=CHUNK_SIZE ){ return file_t( path, "w", _size ); }

    /*─······································································─*/

    inline file_t std_output( const ulong& _size=CHUNK_SIZE ){ return file_t( STDOUT_FILENO, _size ); }
    inline file_t std_input ( const ulong& _size=CHUNK_SIZE ){ return file_t( STDIN_FILENO , _size ); }
    inline file_t std_error ( const ulong& _size=CHUNK_SIZE ){ return file_t( STDERR_FILENO, _size ); }

    /*─······································································─*/

    inline bool exists_folder( const string_t& path ){
        if( path.empty() ){ return 0; }
        DIR* dir = opendir( path.c_str() );
        if( dir==nullptr ){ return 0; }
        return closedir(dir)==0 ? 1 : 0;
    }

    inline bool exists_file( const string_t& path ){
        struct stat /*-------------*/ fileStat; 
        if( path.empty() ) /*---*/ { return 0; }
        if( exists_folder( path ) ){ return 0; }
        return stat( path.data(), &fileStat )==-1 ? 0 : 1;
    }

    /*─······································································─*/

    inline expected_t<time_t,except_t> 
    file_modification_time( const string_t& path ){
    struct stat fileStat; if( stat( path.data(), &fileStat ) < 0 ) {
         return except_t("Failed to get file last modification time properties");
    }    return fileStat.st_mtime; }

    inline expected_t<time_t,except_t> 
    file_access_time( const string_t& path ){
    struct stat fileStat; if( stat( path.data(), &fileStat ) < 0 ) {
         return except_t("Failed to get file last access time properties");
    }    return fileStat.st_atime; }

    inline expected_t<time_t,except_t> 
    file_creation_time( const string_t& path ){
    struct stat fileStat; if( stat( path.data(), &fileStat ) < 0 ) {
         return except_t("Failed to get file creation time properties");
    }    return fileStat.st_ctime; }

    /*─······································································─*/

    inline promise_t<string_t,except_t> read_file( const string_t& path ){
    return promise_t<string_t,except_t> ([=]( 
        res_t<string_t> res,  rej_t<except_t> rej
    ){

        if( !exists_file( path ) ){ rej( "file not found" ); return; }

        auto rd1 = type::bind( generator::file::read() );
        auto fl1 = type::bind( file_t( path, "r" ) );
        auto bff = ptr_t<string_t>( 0UL );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( fl1->is_available() ){
                
                coWait( (*rd1)( &fl1 ) == 1 );
                if( rd1->state==0 ){ break; }

               *bff += rd1->data;

            coNext; } res( *bff );

        coFinish
        }));

    }); }

    /*─······································································─*/

    inline promise_t<ulong,except_t> write_file( const string_t& path, const string_t& message ){
    return promise_t<ulong,except_t> ([=]( 
        res_t<ulong> res, rej_t<except_t> rej
    ){
        
        if( !exists_file( path ) ){ rej( "file not found" ); return; }

        auto rd1 = type::bind( generator::file::write() );
        auto fl1 = type::bind( file_t( path, "w" ) );
        auto bff = ptr_t<ulong>( 0UL, 0UL );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( fl1->is_available() ){
                
                coWait( (*rd1)( &fl1, message ) == 1 );
                if( rd1->state==0 ){ break; }

               *bff += rd1->state;

            coNext; } res( *bff );

        coFinish
        }));

    }); }

    /*─······································································─*/

    inline promise_t<ulong,except_t> append_file( const string_t& path, const string_t& message ){
    return promise_t<ulong,except_t> ([=]( 
        res_t<ulong> res,  rej_t<except_t> rej
    ){
        
        if( !exists_file( path ) ){ rej( "file not found" ); return; }

        auto rd1 = type::bind( generator::file::write() );
        auto fl1 = type::bind( file_t( path, "a+" ) );
        auto bff = ptr_t<ulong>( 0UL, 0UL );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( fl1->is_available() ){
                
                coWait( (*rd1)( &fl1, message ) == 1 );
                if( rd1->state==0 ){ break; }

               *bff += rd1->state;

            coNext; } res( *bff );

        coFinish
        }));

    }); }

    /*─······································································─*/

    inline int copy_file( const string_t& src, const string_t& des ){
        if( !exists_file( src ) ){ return -1; } 
        stream::pipe( file_t( src, "r" ), file_t( des, "w" ) ); 
    return 1; }

    /*─······································································─*/

    inline int rename_file( const string_t& oname, const string_t& nname ) {
        if( oname.empty() || nname.empty() ){ return -1; }
        return rename( oname.c_str(), nname.c_str() );
    }

    /*─······································································─*/

    inline int move_file( const string_t& oname, const string_t& nname ) {
        if( oname.empty() || nname.empty() ){ return -1; }
        return rename_file( oname, nname );
    }

    /*─······································································─*/

    inline int remove_file( const string_t& path ){
        if( path.empty() ){ return -1; }
        return remove( path.c_str() );
    }

    /*─······································································─*/

    inline int create_file( const string_t& path ){
        if  ( exists_folder( path ) ){ return -1; }
        if  ( path.empty() )/*-----*/{ return -1; }
        file_t( path, "w+" ); /*----*/ return  1;
    }

    /*─······································································─*/

    inline ulong file_size( const string_t& path ){
        if( exists_file( path ) ){
            return file_t( path, "r" ).size();
        }   return 0;
    }

    /*─······································································─*/

    inline int rename_folder( const string_t& oname, const string_t& nname ) {
        return rename_file( oname, nname );
    }

    /*─······································································─*/

    inline int move_folder( const string_t& oname, const string_t& nname ){
        return rename_file( oname, nname );
    }

    /*─······································································─*/

    inline int create_folder( const string_t& path, uint permission=0777 ){
        if( path.empty() ){ return -1; }
        return mkdir( path.c_str(), permission );
    }

    /*─······································································─*/

    inline int remove_folder( const string_t& path ){
        if( path.empty() ){ return -1; }
        return rmdir( path.c_str() );
    }

    /*─······································································─*/

    inline int read_folder_iterator( const string_t& path, function_t<void,string_t> cb ){
        if( path.empty() ){ return -1; } 
        DIR* dir=opendir( path.c_str() );
        if ( dir == nullptr ){ return -1; }

        process::add( coroutine::add( COROUTINE(){
            struct dirent* entry;
        coBegin

            while( (entry=readdir(dir)) != NULL ){ do {
    		if( string_t(entry->d_name) == ".." ){ break; }
    		if( string_t(entry->d_name) == "."  ){ break; }
                cb( entry->d_name );
            } while(0); coNext; } 
            
            closedir( dir );

        coFinish
        }));

    return 1; }

    /*─······································································─*/

    inline promise_t<ptr_t<string_t>,except_t> read_folder( const string_t& path ){
    return promise_t<ptr_t<string_t>,except_t> ([=](
        res_t<ptr_t<string_t>> res, rej_t<except_t> rej
    ){  process::add([=](){

        if( path.empty() ){ rej( except_t( "invalid path" ) ); return -1; }
        DIR* dir = opendir( path.c_str() );
        if( dir==nullptr ){ rej( except_t( "invalid dir"  ) ); return -1; }
            
        struct dirent* entry; queue_t<string_t> list;

        while( (entry=readdir(dir))  != NULL ){
        if( string_t ( entry->d_name )==".." ){ continue; }
        if( string_t ( entry->d_name )=="."  ){ continue; }
            list.push( entry->d_name );
        }
        
        closedir( dir ); res( list.data() );

    return -1; }); });}

    /*─······································································─*/

    inline bool is_folder( const string_t& path ){ return exists_folder(path); }
    inline bool   is_file( const string_t& path ){ return exists_file  (path); }

    /*─······································································─*/

    inline int copy_folder( const string_t& opath, const string_t& npath ){
        auto cmd = string::format( "cp -R %s %s", (char*)opath, (char*)npath );
        return ::system( cmd.c_str() );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/