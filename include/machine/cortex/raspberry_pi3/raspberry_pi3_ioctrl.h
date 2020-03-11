// EPOS Cortex-A I/O Control Module Mediator Declarations

#ifndef __raspberry_pi3_ioctrl_h
#define __raspberry_pi3_ioctrl_h

#include <architecture/cpu.h>
#include <system/memory_map.h>

__BEGIN_SYS

class IOCtrl
{
    // This is a hardware object.
    // Use with something like "new (Memory_Map::MBOX_BASE) IOCtrl".

private:
    typedef CPU::Reg32 Reg32;

    struct msg_tag {
        volatile Reg32 tag_id;              // the message id
        volatile Reg32 buffer_size;         // size of the buffer (which in this case is always 8 bytes)
        volatile Reg32 respreq;             // the ID of the clock/voltage to get or set
        volatile Reg32 val;                 // the value (e.g. rate (in Hz)) to set
        volatile Reg32 val2;                    // the value (e.g. rate (in Hz)) to set
    };

    struct set_msg_tag {
        volatile Reg32 tag_id;              // the message id
        volatile Reg32 buffer_size;         // size of the buffer (which in this case is always 8 bytes)
        volatile Reg32 data_size;             // the ID of the clock/voltage to get or set
        volatile Reg32 val;                 // the value (e.g. rate (in Hz)) to set
        volatile Reg32 val2;                    // the value (e.g. rate (in Hz)) to set
        volatile Reg32 val3;                    // the value (e.g. rate (in Hz)) to set
    };

    struct mailbox_msg {
        volatile Reg32 msg_size;                // simply, sizeof(struct vc_msg)
        volatile Reg32 request_code;            // holds various information like the success and number of bytes returned (refer to mailboxes wiki)
        struct msg_tag tag;             // the tag structure above to make
        volatile Reg32 end_tag;             // an end identifier, should be set to NULL
    } __attribute__((aligned(16)));

    struct set_mailbox_msg {
        volatile Reg32 msg_size;                // simply, sizeof(struct vc_msg)
        volatile Reg32 request_code;            // holds various information like the success and number of bytes returned (refer to mailboxes wiki)
        struct set_msg_tag tag;             // the tag structure above to make
        volatile Reg32 end_tag;             // an end identifier, should be set to NULL
    } __attribute__((aligned(16)));

    // The struct bellow was replaced by enum of MBOX0 offsets
    /*typedef struct {
        volatile Reg32 read;
        volatile Reg32 rsvd[3];
        volatile Reg32 peek;
        volatile Reg32 sender;
        volatile Reg32 status;
        volatile Reg32 config;
        volatile Reg32 write;
    } Mailbox;
    */

public:

    // usefull ARM MBOX REG STATUS
    enum {
        MBOX_FULL               = 0x80000000,
        MBOX_EMPTY              = 0x40000000
    };

    // usefull offsets
    enum {
        MBOX_COM_CPU_OFFSET         = 0x00040000, // memory offset per cpu
        MBOX0_OFFSET                = 0x80, //Mailbox 0, read by ARM
        MBOX1_OFFSET                = 0xA0  //Mailbox 0, read by GPU
    };

    // MBOX0 offsets
    enum {
        MBOX0_READ                        = 0x80,
        MBOX0_RSVD0                       = 0x84,
        MBOX0_RSVD1                       = 0x88,
        MBOX0_RSVD2                       = 0x8c,
        MBOX0_PEEK                        = 0x90,
        MBOX0_SENDER                      = 0x94,
        MBOX0_STATUS                      = 0x98,
        MBOX0_CONFIG                      = 0x9c,
        MBOX0_WRITE                       = 0xA0,
    };

    // usefull ARM MBOX TAGS
    enum {
        TEMPERATURE_TAG         = 0x00030006,
        CLOCK_TAG               = 0x00030002,
        CLOCK_MAX_TAG           = 0x00030004,
        CLOCK_MIN_TAG           = 0x00030007,
        DVFS_TAG                = 0x00038002,
        VOLTAGE_TAG             = 0x00030003
    };

    // usefull ARM MBOX MESSAGE ID
    enum {
        ARM_CLOCK_ID            = 0x3,
        TEMPERATURE_ID          = 0x0,
        VOLTAGE_ID              = 0x1,
    };

public:
    static void wait_mailbox_write() {
        while (ioc(MBOX0_STATUS) & MBOX_FULL)
            ASM ("nop");
    }

    static void wait_mailbox_read() {
        while (ioc(MBOX0_STATUS) & MBOX_EMPTY)
            ASM ("nop");
    }

    static void writeMailbox0(Reg32 data, Reg32 channel) {
        wait_mailbox_write();
        ioc(MBOX0_WRITE) = (data &~0xf) | (Reg32) (channel & 0xf);
    }

    static Reg32 readMailbox0(Reg32 channel) {
        Reg32 res;
        Reg32 read_channel;
        wait_mailbox_read();
        do {
            res = ioc(MBOX0_READ);
            read_channel = res & 0xf;
        } while (read_channel != channel);
        return (res>>4);
    }

    static void prepare_call(volatile struct mailbox_msg* msg, Reg32 id, Reg32 tag) {
        msg->msg_size = sizeof(struct mailbox_msg);
        msg->request_code = 0x0;
        msg->tag.tag_id = tag;
        msg->tag.buffer_size = 0x8;
        msg->tag.respreq = 0x8;
        msg->tag.val = id;
        msg->tag.val2 = 0x0;
        msg->end_tag = 0x0;
    }

    static void prepare_req(volatile struct set_mailbox_msg* msg, Reg32 id, Reg32 tag, unsigned int value) {
        msg->msg_size = sizeof(struct set_mailbox_msg);
        msg->request_code = 0x0;
        msg->tag.tag_id = tag;
        msg->tag.buffer_size = 0xc;
        msg->tag.data_size = 0xc;
        msg->tag.val = id;
        msg->tag.val2 = value;
        msg->tag.val3 = 0x1;
        msg->end_tag = 0x0;
    }

    static void clean_msg(volatile struct mailbox_msg* msg) {
        msg->msg_size = 0x0;
        msg->request_code = 0x0;
        msg->tag.tag_id = 0x0;
        msg->tag.buffer_size = 0x0;
        msg->tag.respreq = 0x0;
        msg->tag.val = 0x0;
        msg->tag.val2 = 0x0;
        msg->end_tag = 0x0;
    }

    static Reg32 temperature() {
        volatile struct mailbox_msg * local= (volatile struct mailbox_msg *)(Memory_Map::MBOX_COM_BASE+MBOX_COM_CPU_OFFSET*Machine::cpu_id());
        prepare_call(local, (Reg32)TEMPERATURE_ID, (Reg32)TEMPERATURE_TAG);
        writeMailbox0((unsigned long)local, 8);
        readMailbox0(8);
        return local->tag.val2;
    }

    static Reg32 arm_clock() {
        volatile struct mailbox_msg * local= (volatile struct mailbox_msg *)(Memory_Map::MBOX_COM_BASE+MBOX_COM_CPU_OFFSET*Machine::cpu_id());
        prepare_call(local, (Reg32)ARM_CLOCK_ID, (Reg32)CLOCK_TAG);
        writeMailbox0((unsigned long)local, 8);
        readMailbox0(8);
        return local->tag.val2;
    }

    static Reg32 arm_voltage() {
        volatile struct mailbox_msg * local= (volatile struct mailbox_msg *)(Memory_Map::MBOX_COM_BASE+MBOX_COM_CPU_OFFSET*Machine::cpu_id());
        prepare_call(local, (Reg32)VOLTAGE_ID, (Reg32)VOLTAGE_TAG);
        writeMailbox0((unsigned long)local, 8);
        readMailbox0(8);
        return local->tag.val2;
    }

    static Reg32 arm_min_clock() {
        volatile struct mailbox_msg * local= (volatile struct mailbox_msg *)(Memory_Map::MBOX_COM_BASE+MBOX_COM_CPU_OFFSET*Machine::cpu_id());
        prepare_call(local, (Reg32)ARM_CLOCK_ID, (Reg32)CLOCK_MIN_TAG);
        writeMailbox0((unsigned long)local, 8);
        readMailbox0(8);
        return local->tag.val2;
    }

    static Reg32 arm_max_clock() {
        volatile struct mailbox_msg * local= (volatile struct mailbox_msg *)(Memory_Map::MBOX_COM_BASE+MBOX_COM_CPU_OFFSET*Machine::cpu_id());
        prepare_call(local, (Reg32)ARM_CLOCK_ID, (Reg32)CLOCK_MAX_TAG);
        writeMailbox0((unsigned long)local, 8);
        readMailbox0(8);
        return local->tag.val2;
    }

    static Reg32 arm_clock(unsigned int hertz) {
        volatile struct set_mailbox_msg * local= (volatile struct set_mailbox_msg *)(Memory_Map::MBOX_COM_BASE+MBOX_COM_CPU_OFFSET*Machine::cpu_id());
        prepare_req(local, (Reg32)ARM_CLOCK_ID, (Reg32)DVFS_TAG, hertz);
        writeMailbox0((unsigned long)local, 8);
        readMailbox0(8);
        return local->tag.val2;
    }
private:
    volatile Reg32 & ioc(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(this)[o / sizeof(Reg32)]; }
};

__END_SYS

#endif
