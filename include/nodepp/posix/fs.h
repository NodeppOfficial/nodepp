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

    inline void read_file( const string_t& path, function_t<void,string_t> cb ){
        if( path.empty() ){ return; } try {
            file_t _file( path, "r" );
            _file.onData( cb ); stream::pipe(_file);
        } catch( except_t ){}
    }

    inline string_t read_file( const string_t& path ){ string_t s;
        if( path.empty() ){ return s; } try {
            file_t _file( path, "r" );
            return stream::await(_file);
        } catch( except_t ){} 
    return nullptr; }

    /*─······································································─*/

    inline int copy_file( const string_t& src, const string_t& des ){
        if( src.empty() || des.empty() ){ return -1; } try {
            file_t _file_a ( src, "r" );
            file_t _file_b ( des, "w" );
            stream::pipe( _file_a, _file_b ); 
        return 0; } catch( except_t ) {} return -1;
    }

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

    inline bool exists_file( const string_t& path ){
        if  ( path.empty() )/*-*/{ return 0; }
        try { file_t( path, "r" ); return 1;
            } catch( except_t ){ } return 0;
    }

    /*─······································································─*/

    inline int create_file( const string_t& path ){
        if  ( path.empty() )/*--*/{ return -1; }
        try { file_t( path, "w+" ); return  1;
            } catch( except_t ){  } return  0;
    }

    /*─······································································─*/

    inline ulong file_size( const string_t& path ){
        try { file_t file( path, "r" );
              return file.size();
        } catch( except_t ){} return 0;
    }

    /*─······································································─*/

    inline void write_file( const string_t& path, const string_t& data ){
        file_t file( path, "w" ); file.write( data );
    }

    /*─······································································─*/

    inline void append_file( const string_t& path, const string_t& data ){
        file_t file( path, "a" ); file.write( data );
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

    inline bool exists_folder( const string_t& path ){
        if( path.empty() ){ return 0; }
        DIR* dir = opendir( path.c_str() );
        if( dir==nullptr ){ return 0; }
        return closedir(dir)==0 ? 1 : 0;
    }

    /*─······································································─*/

    inline void read_folder( const string_t& path, function_t<void,string_t> cb ){
        if( path.empty() ){ return; } 
        DIR* dir=opendir( path.c_str() );
        if ( dir == nullptr ){ return; }

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

    }

    inline ptr_t<string_t> read_folder( const string_t& path ){
        if( path.empty() ){ return nullptr; }
        DIR* dir = opendir( path.c_str() );

        if( dir == nullptr ){ return nullptr; }
        struct dirent* entry; queue_t<string_t> list;

        while ((entry = readdir(dir)) != NULL) {
		if( string_t(entry->d_name) == ".." ) continue;
		if( string_t(entry->d_name) == "."  ) continue;
            list.push( entry->d_name );
        }   
        
        closedir( dir ); return list.data();
    }

    /*─······································································─*/

    inline long folder_size( const string_t& path ){
          auto list = read_folder( path );
        return list.size();
    }

    /*─······································································─*/

    inline bool is_folder( const string_t& path ){ return exists_folder(path); }
    inline bool   is_file( const string_t& path ){ return exists_file(path); }

    /*─······································································─*/

    inline int copy_folder( const string_t& opath, const string_t& npath ){
        auto cmd = string::format( "cp -R %s %s", (char*)opath, (char*)npath );
        return ::system( cmd.c_str() );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/