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
#include <termios.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class serial_t : public file_t { 
protected:

	int GETBAUD( uint baud ) const noexcept {
		switch( baud ){
			case 110:     return B110;    break;
			case 300:     return B300;    break;
			case 600:     return B600;    break;
			case 1200:    return B1200;   break;
			case 2400:    return B2400;   break;
			case 4800:    return B4800;   break;
			case 9600:    return B9600;   break;
			case 19200:   return B19200;  break;
			case 38400:   return B38400;  break;
			case 57600:   return B57600;  break;
			case 115200:  return B115200; break;
			default:      return B9600;   break;
		}
	}

	void set_baud_rate( const int& fd, uint baud ) const noexcept {
        struct termios options; tcgetattr( fd, &options );

        options.c_cflag = GETBAUD(baud) | CS8 | CLOCAL | CREAD;
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;

        tcflush( fd, TCIFLUSH ); tcsetattr( fd, TCSANOW, &options );
	}

public: serial_t() noexcept : file_t() {}

    event_t<serial_t> onConnect;

	serial_t( const string_t& path, uint baud=0, const ulong& _size=CHUNK_SIZE ): file_t( path, nullptr )
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
		array_t<string_t> result; for( auto x : fs::read_folder("/dev") ){
			if( strncmp(x.data(),"ttyUSB",6)==0 || 
				strncmp(x.data(),"ttyACM",6)==0 ||
				strncmp(x.data(),"ttyS"  ,4)==0 
			){  result.push( "/dev/" + x );  }
		}	return result;
	}

}}

/*────────────────────────────────────────────────────────────────────────────*/