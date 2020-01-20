/******************************************************************************
 *  Filename:       rom.h
 *  Revised:        $Date: 2012-10-03 22:23:04 +0200 (on, 03 okt 2012) $
 *  Revision:       $Revision: 8460 $
 *
 *  Description:    Prototypes for the ROM utility functions.
 *
 *  Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef __emote3_rom_h
#define __emote3_rom_h

__BEGIN_SYS;

class eMote3_ROM
{
    //
    // Start address of the ROM hard API access table (located after the ROM FW rev field)
    //
    const static unsigned int ROM_API_TABLE_ADDR = 0x00000048;

    //
    // ROM utility function interface types
    //
    typedef unsigned int (* volatile FPTR_CRC32_T)     (unsigned char* pData, unsigned int byteCount);
    typedef unsigned int (* volatile FPTR_GETFLSIZE_T) (void);
    typedef unsigned int (* volatile FPTR_GETCHIPID_T) (void);
    typedef int          (* volatile FPTR_PAGEERASE_T) (unsigned int FlashAddr, unsigned int Size);
    typedef int          (* volatile FPTR_PROGFLASH_T) (const unsigned int* pRamData, unsigned int FlashAdr, unsigned int ByteCount);
    typedef void         (* volatile FPTR_RESETDEV_T)  (void);
    typedef void*        (* volatile FPTR_MEMSET_T)    (void* s, int c, unsigned int n);
    typedef void*        (* volatile FPTR_MEMCPY_T)    (void* s1, const void* s2, unsigned int n);
    typedef int          (* volatile FPTR_MEMCMP_T)    (const void* s1, const void* s2, unsigned int n);
    typedef void*        (* volatile FPTR_MEMMOVE_T)   (void* s1, const void* s2, unsigned int n);

    //
    // ROM Hard-API access table type
    //
    typedef struct
    {
        FPTR_CRC32_T     Crc32;
        FPTR_GETFLSIZE_T GetFlashSize;
        FPTR_GETCHIPID_T GetChipId;
        FPTR_PAGEERASE_T PageErase;
        FPTR_PROGFLASH_T ProgramFlash;
        FPTR_RESETDEV_T  ResetDevice;
        FPTR_MEMSET_T    memset;
        FPTR_MEMCPY_T    memcpy;
        FPTR_MEMCMP_T    memcmp;
        FPTR_MEMMOVE_T   memmove;
    } ROM_API_table;

public:
    // Hooks to functions implemented in ROM. Documentation is available in the ROM User Guide (swru333a.pdf)

#define ROM_API ((ROM_API_table*)ROM_API_TABLE_ADDR)
    static unsigned int crc32(unsigned char * data, unsigned int size) { return ROM_API->Crc32(data, size); }

    static unsigned int flash_size() { return ROM_API->GetFlashSize(); }

    static unsigned int chip_id() { return ROM_API->GetChipId(); }

    static int page_erase(unsigned int flash_address, unsigned int size) { return ROM_API->PageErase(flash_address, size); }

    static int program_flash(const unsigned int* ram_data, unsigned int flash_addr, unsigned int size) { return ROM_API->ProgramFlash(ram_data,flash_addr,size); }

    static void reboot(void) { ROM_API->ResetDevice(); }

    static void* memset(void* s, int c, unsigned int n) { return ROM_API->memset(s,c,n); }

    static void* memcpy(void* s1, const void* s2, unsigned int n) { return ROM_API->memcpy(s1,s2,n); }

    static int memcmp(const void* s1, const void* s2, unsigned int n) { return ROM_API->memcmp(s1,s2,n); }

    static void* memmove(void* s1, const void* s2, unsigned int n) { return ROM_API->memmove(s1,s2,n); }
#undef ROM_API
};

__END_SYS;

#endif // __ROM_H__
