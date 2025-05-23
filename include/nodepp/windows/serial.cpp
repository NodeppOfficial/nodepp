/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once
#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class serial_t : public file_t { 
protected:

	int GETBAUD( uint baud ) const noexcept {
		switch( baud ){
			case 110:     return CBR_110;    break;
			case 300:     return CBR_300;    break;
			case 600:     return CBR_600;    break;
			case 1200:    return CBR_1200;   break;
			case 2400:    return CBR_2400;   break;
			case 4800:    return CBR_4800;   break;
			case 9600:    return CBR_9600;   break;
			case 19200:   return CBR_19200;  break;
			case 38400:   return CBR_38400;  break;
			case 57600:   return CBR_57600;  break;
			case 115200:  return CBR_115200; break;
			default:      return CBR_9600;   break;
		}
	}

	void set_baud_rate( const HANDLE& fd, uint baud ) const noexcept {
		DCB dcbSerialParams; memset( &dcbSerialParams, 0, sizeof(DCB) );

		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		dcbSerialParams.BaudRate  = GETBAUD(baud);
		dcbSerialParams.ByteSize  = 8;
		dcbSerialParams.Parity    = NOPARITY;
		dcbSerialParams.StopBits  = ONESTOPBIT;

		SetCommState( fd, &dcbSerialParams );
	}

public: serial_t() noexcept : file_t() {}

    event_t<serial_t> onConnect;

	serial_t( const string_t& path, uint baud=0, const ulong& _size=CHUNK_SIZE ): file( path, nullptr )
			{ set_baud_rate( this->get_fd(), baud ); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace serial {

    template <class... T> serial_t connect( const T&... args ){ 

		serial_t client( args... ); 

		client.onConnect.once([=]( serial_t cli ){
			ptr_t<_file_::read> _read = new _file_::read;
			cli.onDrain.once([=](){ cli.free(); });

			process::task::add([=](){
				if(!cli.is_available() )    { cli.close(); return -1; }
				if((*_read)(&cli)==1 )      { return 1; }
				if(  _read->state<=0 )      { return 1; }
				cli.onData.emit(_read->data); return 1;
			});

		}); 

        process::task::add([=](){
            client.onConnect.emit( client ); return -1;
        });
		
		return client; 
	}

	array_t<string_t> get_devices(){
		array_t<string_t> result; for( auto x=0; x<=255; x++ ){
			if( fs::exists_file(string::format("COM%d",x)) )
			  { result.push( string::format("COM%d",x) ); }
		}	return result;
	}

}}

/*────────────────────────────────────────────────────────────────────────────*/
