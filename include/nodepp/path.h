/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_PATH
#define NODEPP_PATH

/*────────────────────────────────────────────────────────────────────────────*/

#include "regex.h"
#include "map.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

struct path_t {
    string_t type;
    string_t path;
    string_t root; 
    string_t base;
    string_t name;
    string_t dir;
    string_t ext;
};

/*────────────────────────────────────────────────────────────────────────────*/

namespace _path_ { map_t<string_t,string_t> mimetype ({

    { "txt",  "text/plain" },
    { "text", "text/plain" },

    { "otf",  "font/otf" },
    { "ttf",  "font/ttf" },
    { "woff", "font/woff" },
    { "woff2","font/woff2" },

    { "oga",  "audio/ogg" },
    { "aac",  "audio/aac" },
    { "wav",  "audio/wav" },
    { "mp3",  "audio/mpeg" },
    { "opus", "audio/opus" },
    { "weba", "audio/webm" },

    { "ogv",  "video/ogg" },
    { "mp4",  "video/mp4" },
    { "ts",   "video/mp2t" },
    { "webm", "video/webm" },
    { "mpeg", "video/mpeg" },
    { "avi",  "video/x-msvideo" },

    { "c",    "text/X-C" },
    { "css",  "text/css" },
    { "csv",  "text/csv" },
    { "html", "text/html" },
    { "scss", "text/scss" },
    { "cpp",  "text/X-CPP" },
    { "ics",  "text/calendar" },
    { "js",   "text/javascript" },
    { "xml",  "application/xhtml+xml" },

    { "bmp",  "image/bmp" },
    { "gif",  "image/gif" },
    { "png",  "image/png" },
    { "jpg",  "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "webp", "image/webp" },
    { "svg",  "image/svg+xml" },
    { "ico",  "image/vnd.microsoft.icon" },

    { "url",  "application/x-www-form-urlencoded" },

    { "zip",  "application/zip" },
    { "gz",   "application/gzip" },
    { ".h",   "application/x-.h" },
    { "json", "application/json" },
    { "wasm", "application/wasm" },
    { "tar",  "application/x-tar" },
    { "rar",  "application/vnd.rar" },
    { "7z",   "application/x-7z-compressed" },
    { "m3u8", "application/vnd.apple.mpegurl" },
    { "exe",  "application/vnd.microsoft.portable-executable" },

    { "pdf",  "application/pdf" },
    { "doc",  "application/msword" },
    { "vsd",  "application/vnd.visio" },
    { "xls",  "application/vnd.ms-excel" },
    { "ppt",  "application/vnd.ms-powerpoint" },
    { "swf",  "application/x-shockwave-fla.h" },
    { "ods",  "application/vnd.oasis.opendocument.spreadsheet" },
    { "odp",  "application/vnd.oasis.opendocument.presentation" },
    { "odt",  "application/vnd.oasis.opendocument.presentation" },
    { "xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
    { "docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
    { "pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" }

}); }

/*────────────────────────────────────────────────────────────────────────────*/

namespace path {

namespace {
#if false // _KERNEL == NODEPP_KERNEL_WINDOWS
    string_t sep  = "\\\\";
    string_t root = "c:\\\\";
    string_t  one = "[^\\\\]+";
    string_t init = "\\w:\\\\";
    string_t _beg = "\\w:\\\\";
    string_t  beg = "^\\w:\\\\";
#else
    string_t init = "";
    string_t  sep = "/";
    string_t _beg = "/";
    string_t root = "./";
    string_t  beg = "^/";
    string_t  one = "[^/]+";
#endif
}
    
    /*─······································································─*/

    string_t normalize( string_t path ){ 
        auto sec = regex::split( path, "/+|\\\\+" );
        array_t<string_t> nsec; ulong y=0;

        for ( ulong x=0; x<sec.size(); x++ ){
         if ( sec[x] == ".." ){ 
              nsec.push( sec[x] );
              y++; continue; 
            } break;
        }

        for ( ulong x=y; x<sec.size(); x++ ){
         if ( sec[x] == ".." && !nsec.empty() ){
              nsec.pop(); continue; 
            } nsec.push( sec[x] );
        }

        return nsec.join( sep );
    }
    
    /*─······································································─*/

    bool is_absolute( const string_t& path ){
        regex_t reg("^"+beg); return reg.test( path );
    }
    
    /*─······································································─*/

    string_t extname( const string_t& path ){ string_t m;
        regex_t reg("\\.\\w+$"); if( !reg.test( path ) ) 
          { return m; } return reg.match( path ).slice(1);
    }
    
    /*─······································································─*/

    string_t mimetype( const string_t& path ){
        string_t ext = extname( path ); if( ext.empty() ) 
        { return ext; } for( auto x: _path_::mimetype.data() ){
            if( regex::test( x.first, ext ) ){ return x.second; }
        }   return string::format("aplication/%s",ext.c_str());
    }

    string_t mimetype( const path_t& path ){
         if( path.ext.empty() ) { return path.ext; } 
        for( auto x: _path_::mimetype.data() ){
            if( regex::test( x.first, path.ext ) ){ return x.second; }
        }   return string::format("aplication/%s",path.ext.c_str());
    }

    /*─······································································─*/

    string_t dirname( const string_t& path ){ 
        auto vec = regex::split( path, "/+|\\\\+" );
        vec.pop(); return vec.join( sep );
    }
    
    /*─······································································─*/

    string_t basename( const string_t& path ){ 
        auto vec = regex::match_all( path, one );
        if ( vec.empty() ){ return nullptr; }
        return vec[ vec.last() ];
    }
    
    /*─······································································─*/

    string_t basename( const string_t& path, const string_t& del ){ 
        auto vec = regex::match_all( path, one );
        if ( vec.empty() ){ return nullptr; }
        return regex::replace( vec[ vec.last() ], del, "" );
    }

    /*─······································································─*/

    string_t format( path_t& obj ) { string_t _path;

        if( !obj.path.empty() ){ return obj.path;   }
        
        if( !obj.root.empty() ){ _path += obj.root; }
        else                   { _path += root;     }

        if( !obj.dir .empty() ){ _path += obj.dir;  }
        if( !obj.base.empty() ){ _path += obj.base; }

        else {
            if( !obj.name.empty() ){ _path += obj.name + string::to_string("."); }
            if( !obj.ext .empty() ){ _path += obj.ext; }
        }
        
        return _path;
    }
    
    /*─······································································─*/

    path_t parse( const string_t& path ) { path_t result;

        if( regex::test( path, beg ) ) result.root = _beg;
        else                           result.root = root;

        result.path = path;
        result.ext  = extname( path ); 
        result.dir  = dirname( path );
        result.base = basename( path );
        result.type = mimetype( path );
        result.name = basename( path, "\\."+result.ext );

        return result;
    };

    /*─······································································─*/

    string_t relative( const string_t& path_a, const string_t& path_b ){

        auto secA = regex::split( path::normalize(path_a), "/+|\\\\+" );
        auto secB = regex::split( path::normalize(path_b), "/+|\\\\+" );
        auto sec  = array_t<string_t>(); ulong y=0;

        for ( ulong x=0; x<secA.size() && x<secB.size(); x++ ){
         if ( secA[x]==secB[x] ){ y++; continue; } break;
        }

        for ( ulong x=y; x<secA.size(); x++ ){ sec.push(    ".." ); }
        for ( ulong x=y; x<secB.size(); x++ ){ sec.push( secB[x] ); }

        return sec.join( sep );
    }
    
    /*─······································································─*/

    string_t push( const string_t& path, const string_t& dir ){
        auto sec = regex::split( path::normalize(path), "/+|\\\\+" );
             sec.push( dir ); return sec.join( sep );
    }

    string_t unshift( const string_t& path, const string_t& dir ){
        auto sec = regex::split( path::normalize(path), "/+|\\\\+" );
             sec.unshift( dir ); return sec.join( sep );
    }
    
    /*─······································································─*/

    string_t pop( const string_t& path ){
        auto sec = regex::split( path::normalize(path), "/+|\\\\+" );
             sec.pop(); return sec.join( sep );
    }

    string_t shift( const string_t& path ){
        auto sec = regex::split( path::normalize(path), "/+|\\\\+" );
             sec.shift(); return sec.join( sep );
    }
    
    /*─······································································─*/

    array_t<string_t> split( const string_t& path ){ 
      return regex::split( path::normalize(path), "/+|\\\\+" );
    }

    template< class T, class... V > 
    string_t join( const T& argc, const V&... args ){ 
      return normalize( string::join( sep, argc, args... ) ); 
    }
    
}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif
