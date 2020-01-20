#ifndef __emote3_flash_h
#define __emote3_flash_h

#include <utility/math.h>
#include <flash.h>
#include "emote3_rom.h"

__BEGIN_SYS;

class eMote3_Flash : public Flash_Common
{
public:
    static const unsigned int size() { return eMote3_ROM::flash_size(); }
    static const unsigned int base_address() { return FLASH_BASE; }

    static void write(
            unsigned int address_offset,
            const unsigned int *ram_data,
            unsigned int n_bytes
    )
    {

        if((address_offset + n_bytes) > size())
        {
            db<eMote3_Flash>(ERR) << "eMote3_Flash::write(" << hex << address_offset << "," << ram_data << "," << n_bytes << ") ERROR: write would pass the end of flash, aborting!" << endl;
            db<eMote3_Flash>(ERR) << size() << endl;
            return;
        }

        unsigned int flash_address = FLASH_BASE + address_offset;
        unsigned int end_address = flash_address + n_bytes;
        unsigned int first_page = address_to_page(flash_address);
        unsigned int last_page = address_to_page(end_address - 1);

        unsigned int write_begin = flash_address;
        const unsigned int * read_begin = ram_data;

        for (unsigned int page = first_page; page <= last_page; ++page) {
            bool erase_needed = false;
            unsigned int next_page_boundary = page_to_address(page+1);
            unsigned int write_end = min(end_address, next_page_boundary);
            unsigned int delta = (write_end - write_begin)/4;

            if (data.write_count[page] >= 8) {
                erase_needed = true;
            } else {
                for (auto i = 0u; i < delta; ++i) {
                    auto flash_ptr = reinterpret_cast<unsigned int*>(write_begin);
                    if ((flash_ptr[i] & read_begin[i]) != read_begin[i]) {
                        erase_needed = true;
                        break;
                    }
                }
            }

            if (erase_needed)
            {
                unsigned int page_begin = page_to_address(page);
                unsigned int page_end = next_page_boundary;

                if ((write_begin != page_begin) || (write_end != page_end)) {
                    save(page);
                }

                erase(page);

                unsigned int write_delta = (write_begin - page_begin)/4;
                for (auto i = 0u; i < delta; ++i) {
                    data.page_backup[write_delta + i] = read_begin[i];
                }

                unsigned int i = 0;
                do
                {
                    // The ROM function simply won't write everything. We have to call it multiple times.
                    eMote3_ROM::program_flash(&(data.page_backup[i]), page_begin + (4*i), SIZE_OF_PAGE - (4*i));
                    for(; (i < SIZE_OF_PAGE/4) && (reinterpret_cast<volatile unsigned int *>(page_begin)[i] == data.page_backup[i]); ++i);
                } while (i < SIZE_OF_PAGE/4);

            } else {
                unsigned int i = 0;
                do
                {
                    // The ROM function simply won't write everything. We have to call it multiple times.
                    eMote3_ROM::program_flash(&(read_begin[i]), write_begin + (4*i), (delta-i)*4);
                    for(; (i < delta) && (reinterpret_cast<volatile unsigned int *>(write_begin)[i] == read_begin[i]); ++i);
                } while (i < delta);
            }

            ++data.write_count[page];
            write_begin += delta*4;
            read_begin += delta;
        }
    }

    static void read(
            unsigned int address_offset,
            unsigned int *ram_data,
            unsigned int n_bytes
    )
    {        
        for(unsigned int i = 0; i < n_bytes; i+=4) {
            ram_data[i/4] = read(address_offset + i);
        }
    }

    static unsigned int read(unsigned int address_offset) {
        return *reinterpret_cast<unsigned int*>(FLASH_BASE + address_offset);
    }

private:
    enum {
        FLASH_BASE = 0x200000
    };

    static void save(unsigned int page)
    {
        auto address = reinterpret_cast<unsigned int *>(page_to_address(page));
        for (auto i = 0u; i < WORDS_IN_PAGE; ++i) {
            data.page_backup[i] = address[i];
        }
    }

    static int erase(unsigned int page) {
        data.write_count[page] = 0;
        return eMote3_ROM::page_erase(page_to_address(page), 1);
    }

    static unsigned int address_to_page(unsigned int address)
    {
        return (address - FLASH_BASE)/SIZE_OF_PAGE;
    }

    static unsigned int page_to_address(unsigned int page)
    {
        return (page * SIZE_OF_PAGE) + FLASH_BASE;
    }

    static const unsigned int N_PAGES = 256;
    static const unsigned int SIZE_OF_PAGE = 2048;
    static const unsigned int WORDS_IN_PAGE = SIZE_OF_PAGE/sizeof(unsigned int);

    struct Data {
        Data()
        {
            for (auto i = 0u; i < WORDS_IN_PAGE; ++i) {
                page_backup[i] = 0;
            }

            for (auto i = 0u; i < N_PAGES; ++i) {
                write_count[i] = 8;
            }
        }

        unsigned int page_backup[WORDS_IN_PAGE];
        unsigned char write_count[N_PAGES];
    };

    static Data data;
};

__END_SYS;

#endif
