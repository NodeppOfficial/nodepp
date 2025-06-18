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
#include <stdio.h>

static volatile long PipeSerialNumber;

BOOL APIENTRY CreatePipe(
    OUT LPHANDLE lpReadPipe ,
    OUT LPHANDLE lpWritePipe,
    IN  LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN  DWORD  nSize,
    DWORD dwReadMode,
    DWORD dwWriteMode
){  CHAR  PipeNameBuffer[ MAX_PATH ];

    if ( nSize == 0 ) { nSize = CHUNK_SIZE; }

    sprintf( PipeNameBuffer, "\\\\.\\Pipe\\RemoteNodepp.%08x.%08x",
        GetCurrentProcessId(), InterlockedIncrement(&PipeSerialNumber)
    );

    *lpReadPipe = CreateNamedPipeA(
        PipeNameBuffer, PIPE_ACCESS_DUPLEX | dwReadMode,
        PIPE_TYPE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
        nSize, nSize, 0, lpPipeAttributes
    );

    if( lpReadPipe==INVALID_HANDLE_VALUE) { throw except_t("Invalid Read Pipe"); }

    *lpWritePipe = CreateFileA(
        PipeNameBuffer, GENERIC_WRITE, FILE_SHARE_WRITE,
        lpPipeAttributes, OPEN_EXISTING, dwWriteMode,
        NULL
    );

    if( lpWritePipe==INVALID_HANDLE_VALUE) { throw except_t("Invalid Write Pipe"); }

    return( TRUE );
}
