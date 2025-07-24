/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once
#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace fs {

    file_t readable( const string_t& path, const ulong& _size=CHUNK_SIZE ){ return file_t( path, "r", _size ); }
    file_t writable( const string_t& path, const ulong& _size=CHUNK_SIZE ){ return file_t( path, "w", _size ); }

    /*─······································································─*/

    file_t std_input ( const ulong& _size=CHUNK_SIZE ){ return file_t( GetStdHandle(STD_INPUT_HANDLE) , _size ); }
    file_t std_output( const ulong& _size=CHUNK_SIZE ){ return file_t( GetStdHandle(STD_OUTPUT_HANDLE), _size ); }
    file_t std_error ( const ulong& _size=CHUNK_SIZE ){ return file_t( GetStdHandle(STD_ERROR_HANDLE) , _size ); }

    /*─······································································─*/

    time_t file_modification_time( const string_t& path ){
        WIN32_FILE_ATTRIBUTE_DATA fileData; ULARGE_INTEGER uli;

        if (!GetFileAttributesExA(path.c_str(),GetFileExInfoStandard,&fileData) )
           { throw except_t("Failed to get file last modification time properties"); }

        FILETIME ftModified = fileData.ftLastWriteTime;
        uli.LowPart         = ftModified.dwLowDateTime;
        uli.HighPart        = ftModified.dwHighDateTime;

        return uli.QuadPart / 10000000ULL - 11644473600ULL;
    }

    time_t file_access_time( const string_t& path ){
        WIN32_FILE_ATTRIBUTE_DATA fileData; ULARGE_INTEGER uli;

        if (!GetFileAttributesExA(path.c_str(),GetFileExInfoStandard,&fileData) )
           { throw except_t("Failed to get file last access time properties"); }

        FILETIME ftAccess = fileData.ftLastAccessTime;
        uli.HighPart      = ftAccess.dwHighDateTime;
        uli.LowPart       = ftAccess.dwLowDateTime;

        return uli.QuadPart / 10000000ULL - 11644473600ULL;
    }

    time_t file_creation_time( const string_t& path ){
        WIN32_FILE_ATTRIBUTE_DATA fileData; ULARGE_INTEGER uli;

        if (!GetFileAttributesExA(path.c_str(),GetFileExInfoStandard,&fileData) )
           { throw except_t("Failed to get file creation time properties"); }

        FILETIME ftCreated = fileData.ftCreationTime;
        uli.LowPart        = ftCreated.dwLowDateTime;
        uli.HighPart       = ftCreated.dwHighDateTime;

        return uli.QuadPart / 10000000ULL - 11644473600ULL;
    }

    /*─······································································─*/

    void read_file( const string_t& path, function_t<void,string_t> cb ){
        if( path.empty() ){ return; } file_t _file( path, "r" );
        _file.onData( cb ); stream::pipe(_file);
    }

    string_t read_file( const string_t& path ){ string_t s;
        if( path.empty() ){ return s; }
        file_t _file( path, "r" );
        return stream::await(_file);
    }

    /*─······································································─*/

    int copy_file( const string_t& src, const string_t& des ){
        return CopyFileA( src.c_str(), des.c_str(), 0 )!=0 ? -1 : 0;
    }

    /*─······································································─*/

    int rename_file( const string_t& oname, const string_t& nname ) {
        if( oname.empty() || nname.empty() ){ return -1; }
        return rename( oname.c_str(), nname.c_str() );
    }

    /*─······································································─*/

    int move_file( const string_t& oname, const string_t& nname ) {
        return rename_file( oname, nname );
    }

    /*─······································································─*/

    int remove_file( const string_t& path ){
        if( path.empty() ){ return -1; }
        return remove( path.c_str() );
    }

    /*─······································································─*/

    bool exists_file( const string_t& path ){
         if ( path.empty() )     { return 0; }
        try { file_t( path, "r" ); return 1;
            } catch(...){} return 0;
    }

    /*─······································································─*/

    int create_file( const string_t& path ){
        if ( path.empty() )      { return -1; }
        try{ file_t( path, "w+" ); return  1;
        } catch(...){} return 0;
    }

    /*─······································································─*/

    ulong file_size( const string_t& path ){
        try { file_t file( path, "r" );
              return file.size();
        } catch(...){} return 0;
    }

    /*─······································································─*/

    void write_file( const string_t& path, const string_t& data ){
        file_t file( path, "w" ); file.write( data );
    }

    /*─······································································─*/

    void append_file( const string_t& path, const string_t& data ){
        file_t file( path, "a" ); file.write( data );
    }

    /*─······································································─*/

    int rename_folder( const string_t& oname, const string_t& nname ) {
        return rename_file( oname, nname );
    }

    /*─······································································─*/

    int move_folder( const string_t& oname, const string_t& nname ){
        return rename_file( oname, nname );
    }

    /*─······································································─*/

    int create_folder( const string_t& path, uint /*unused*/ ){
        if( path.empty() ){ return -1; }
        return CreateDirectoryA( path.c_str(), NULL )!=0 ? -1 : 0;
    }

    /*─······································································─*/

    int remove_folder( const string_t& path ){
        if( path.empty() ){ return -1; }
        return RemoveDirectoryA( path.c_str() )!=0 ? -1 : 0;
    }

    /*─······································································─*/

    bool exists_folder( const string_t& path ){
        if( path.empty() ){ return -1; } DWORD attributes = GetFileAttributesA( path.c_str() );
        return ( attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) );
    }

    /*─······································································─*/

    void read_folder( const string_t& path, function_t<void,string_t> cb ){
        if( path.empty() ){ return; } 

        auto findData = type::bind( new WIN32_FIND_DATAA() );
        memset( &findData, 0, sizeof(WIN32_FIND_DATAA) );

        string_t npath= path::push( path, "*" );
        HANDLE hFind  = FindFirstFileA( npath.c_str(), &findData );

        if( hFind == INVALID_HANDLE_VALUE ){ return; }

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
        
    }

    /*─······································································─*/

    ptr_t<string_t> read_folder( const string_t& path ){
        if( path.empty() ){ return nullptr; }
        string_t npath = path::push( path, "*" );

        WIN32_FIND_DATAA findData; queue_t<string_t> list;
        HANDLE hFind = FindFirstFileA( npath.c_str(), &findData );

        if( hFind == INVALID_HANDLE_VALUE ){ return nullptr; }

        while( FindNextFileA(hFind,&findData) != 0 ){
            string_t fileName = findData.cFileName;
            if( fileName != "." && fileName != ".." )
              { list.push( fileName ); }
        }

        FindClose( hFind ); return list.data();
    }

    /*─······································································─*/

    long folder_size( const string_t& path ){
          auto list = read_folder( path );
        return list.size();
    }

    /*─······································································─*/

    bool is_folder( const string_t& path ){ return exists_folder(path); }
    bool   is_file( const string_t& path ){ return exists_file(path); }

    /*─······································································─*/

    int copy_folder( const string_t& opath, const string_t&  npath ){
        return CopyFileA( opath.c_str(), npath.c_str(), 0 )!=0 ? -1 : 0;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/
