/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_FS
#define NODEPP_WINDOWS_FS

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace fs {

    inline file_t std_input ( const ulong& _size=NODEPP_CHUNK_SIZE ){ return file_t( GetStdHandle( STD_INPUT_HANDLE ), _size ); }
    inline file_t std_output( const ulong& _size=NODEPP_CHUNK_SIZE ){ return file_t( GetStdHandle( STD_OUTPUT_HANDLE), _size ); }
    inline file_t std_error ( const ulong& _size=NODEPP_CHUNK_SIZE ){ return file_t( GetStdHandle( STD_ERROR_HANDLE ), _size ); }

    /*─······································································─*/

    inline file_t readable( const string_t& path, const ulong& _size=NODEPP_CHUNK_SIZE ){ return file_t( path, "r", _size ); }
    inline file_t writable( const string_t& path, const ulong& _size=NODEPP_CHUNK_SIZE ){ return file_t( path, "w", _size ); }

    /*─······································································─*/

    inline bool exists_folder( const string_t& path ){
        if( path.empty() ){ return 0; } DWORD attributes = GetFileAttributesA( path.c_str() );
        return ( attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) );
    }

    inline bool exists_file( const string_t& path ){
        if( path.empty() ) /*----------------*/ { return 0; }
        if( exists_folder( path ) ) /*-------*/ { return 0; }
        return !GetFileAttributesA( path.c_str() ) ? 0 : 1;
    }

    /*─······································································─*/

    inline expected_t<time_t,except_t> 
    file_modification_time( const string_t& path ){
        WIN32_FILE_ATTRIBUTE_DATA fileData; ULARGE_INTEGER uli;

        if (!GetFileAttributesExA(path.c_str(),GetFileExInfoStandard,&fileData) )
           { return except_t("Failed to get file last modification time properties"); }

        FILETIME ftModified = fileData.ftLastWriteTime;
        uli.LowPart         = ftModified.dwLowDateTime;
        uli.HighPart        = ftModified.dwHighDateTime;

        return uli.QuadPart / 10000000ULL - 11644473600ULL;
    }
    
    inline expected_t<time_t,except_t> 
    file_access_time( const string_t& path ){
        WIN32_FILE_ATTRIBUTE_DATA fileData; ULARGE_INTEGER uli;

        if (!GetFileAttributesExA(path.c_str(),GetFileExInfoStandard,&fileData) )
           { return except_t("Failed to get file last access time properties"); }

        FILETIME ftAccess = fileData.ftLastAccessTime;
        uli.HighPart      = ftAccess.dwHighDateTime;
        uli.LowPart       = ftAccess.dwLowDateTime;

        return uli.QuadPart / 10000000ULL - 11644473600ULL;
    }
    
    inline expected_t<time_t,except_t> 
    file_creation_time( const string_t& path ){
        WIN32_FILE_ATTRIBUTE_DATA fileData; ULARGE_INTEGER uli;

        if (!GetFileAttributesExA(path.c_str(),GetFileExInfoStandard,&fileData) )
           { return except_t("Failed to get file creation time properties"); }

        FILETIME ftCreated = fileData.ftCreationTime;
        uli.LowPart        = ftCreated.dwLowDateTime;
        uli.HighPart       = ftCreated.dwHighDateTime;

        return uli.QuadPart / 10000000ULL - 11644473600ULL;
    }

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
                if( rd1->state<=0 ){ break; }

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
        
        auto rd1 = type::bind( generator::file::write() );
        auto fl1 = type::bind( file_t( path, "w" ) );
        auto bff = ptr_t<ulong>( 0UL, 0UL );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( fl1->is_available() ){
                
                coWait( (*rd1)( &fl1, message ) == 1 );
                if( rd1->state<=0 ){ break; }

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
                if( rd1->state<=0 ){ break; }

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
        return DeleteFileA( path.c_str() );
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

    inline int create_folder( const string_t& path, uint /*unused*/ ){
        if( path.empty() ){ return -1; }
        return CreateDirectoryA( path.c_str(), NULL )!=0 ? -1 : 0;
    }

    /*─······································································─*/

    inline int remove_folder( const string_t& path ){
        if( path.empty() ){ return -1; }
        return RemoveDirectoryA( path.c_str() )!=0 ? -1 : 0;
    }

    /*─······································································─*/

    inline int read_folder_iterator( const string_t& path, function_t<void,string_t> cb ){
        if( path.empty() ){ return -1; } 

        auto findData = type::bind( new WIN32_FIND_DATAA() );
        memset( &findData, 0, sizeof(WIN32_FIND_DATAA) );

        string_t npath= path::push( path, "*" );
        HANDLE hFind  = FindFirstFileA( npath.c_str(), &findData );

        if( hFind == INVALID_HANDLE_VALUE ){ return -1; }

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( FindNextFileA(hFind,&findData) != 0 ){ do {
        	if( string_t(findData->cFileName)==".." ){ break; }
        	if( string_t(findData->cFileName)=="."  ){ break; }
                cb( findData->cFileName );
            } while(0); coNext; }

            FindClose( hFind );

        coFinish
        }));
        
    return 1; }

    /*─······································································─*/

    inline promise_t<ptr_t<string_t>,except_t> read_folder( const string_t& path ){
    return promise_t<ptr_t<string_t>,except_t> ([=](
        res_t<ptr_t<string_t>> res, rej_t<except_t> rej
    ){  process::add([=](){

        if( path.empty() )
          { rej( except_t( "invalid path" ) ); return -1; }
        
        string_t npath = path::push( path, "*" );

        WIN32_FIND_DATAA findData; queue_t<string_t> list;
        HANDLE hFind = FindFirstFileA( npath.c_str(), &findData );

        if( hFind==INVALID_HANDLE_VALUE )
          { rej( except_t( "invalid dir" ) ); return -1; }

        while( FindNextFileA(hFind,&findData) != 0 ){
            string_t fileName = findData.cFileName;
            if( fileName != "." && fileName != ".." )
              { list.push( fileName ); }
        }

        FindClose( hFind ); res( list.data() );
    
    return -1; }); });}

    /*─······································································─*/

    inline bool is_folder( const string_t& path ){ return exists_folder(path); }
    inline bool   is_file( const string_t& path ){ return exists_file  (path); }

    /*─······································································─*/

    inline int copy_folder( const string_t& opath, const string_t&  npath ){
        return CopyFileA( opath.c_str(), npath.c_str(), 0 )!=0 ? -1 : 0;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/