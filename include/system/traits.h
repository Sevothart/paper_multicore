#ifndef __traits_types_h
#define __traits_types_h

__BEGIN_UTIL

// Utilities
template<unsigned int KEY_SIZE> class SWAES;
class Bitmaps;
class Ciphers;
class CRC;
class Debug;
class ELF;
class Handler;
class Hashes;
class Heaps;
class Lists;
class Observeds;
class Observers;
class OStream;
class Predictors;
class Queues;
class Random;
class Spin;
class SREC;
class Vectors;
template<typename> class Scheduler;
namespace Scheduling_Criteria
{
    class Priority;
    class FCFS;
    class RR;
    class RM;
    class DM;
    class EDF;
    class GRR;
    class CPU_Affinity;
    class GEDF;
    class PEDF;
    class CEDF;
    class PRM;
};

__END_UTIL

__BEGIN_SYS

// System parts
class Build;
class Boot;
class Setup;
class Init;
class Utility;

// Architecture Hardware Mediators
class CPU;
class TSC;
class MMU;
class FPU;
class PMU;

// Machine Hardware Mediators
class Machine;
class PCI;
class IC;
class Timer;
class RTC;
class UART;
class SPI;
class RS485;
class USB;
class EEPROM;
class Display;
class Serial_Display;
class Keyboard;
class Serial_Keyboard;
class Scratchpad;
class Watchdog;
class GPIO;
class I2C;
class ADC;
class FPGA;
template<unsigned int KEY_SIZE> class HWAES;
class Ethernet;
class IEEE802_15_4;
class Modem;
template<typename Family> class NIC;
class PCNet32;
class RTL8139;
class C905;
class E100;
class CC2538;
class M95;
class AT86RF;
class Ethernet_NIC;
class CC1101;

// Transducer Mediators (i.e. sensors and actuators)
class Transducers;
class Dummy_Transducer;
class Accelerometer;
class Gyroscope;
class Thermometer;
class Alternate_Thermometer;
class Hygrometer;
class Alternate_Hygrometer;
class CO2_Sensor;
class Pluviometer;
class Pressure_Sensor;
class Keypad;

// API Components
class System;
class Application;

class Thread;
class Active;
class Periodic_Thread;
class RT_Thread;
class Task;

class Tracer;

class Address_Space;
class Segment;

class Synchronizer;
class Mutex;
class Semaphore;
class Condition;

class Time;
class Clock;
class Chronometer;
class Alarm;
class Delay;

template<typename T> class Clerk;
class Monitor;

class Network;
class ELP;
class TSTPOE;
class TSTP;
template<typename NIC, typename Network, unsigned int HTYPE> class ARP;
class IP;
class ICMP;
class UDP;
class TCP;
class DHCP;
class HTTP;
class IPC;
template<typename Channel, bool connectionless = Channel::connectionless> class Link;
template<typename Channel, bool connectionless = Channel::connectionless> class Port;

class SmartData;
template<typename Transducer, typename Network = TSTP> class Responsive_SmartData;
template<typename Transducer, typename Network = TSTP> class Interested_SmartData;

// Framework
class Framework;
template<typename Component> class Handle;
template<typename Component, bool remote> class Stub;
template<typename Component> class Proxy;
template<typename Component> class Adapter;
template<typename Component> class Scenario;
class Agent;

// Aspects
class Aspect;
template<typename Component> class Authenticated;
template<typename Component> class Shared;
template<typename Component> class Remote;

// Configuration Tokens
template<typename T>
struct Traits
{
    // EPOS software architecture (aka mode)
    enum {LIBRARY, BUILTIN, KERNEL};

    // CPU hardware architectures
    enum {AVR8, H8, ARMv4, ARMv7, ARMv8, IA32, X86_64, SPARCv8, PPC32};

    // Machines
    enum {eMote1, eMote2, STK500, RCX, Cortex, PC, Leon, Virtex};

    // Machine models
    enum {Unique, Legacy_PC, eMote3, LM3S811, Zynq, Realview_PBX, Raspberry_Pi3};

    // Serial display engines
    enum {UART, USB};

    // Life span multipliers
    enum {FOREVER = 0, SECOND = 1, MINUTE = 60, HOUR = 3600, DAY = 86400, WEEK = 604800, MONTH = 2592000, YEAR = 31536000};

    // IP configuration strategies
    enum {STATIC, MAC, INFO, RARP, DHCP};

    // SmartData predictors
    enum :unsigned char {NONE, LVP, DBP};

    // Default traits
    static const bool enabled = true;
    static const bool debugged = true;
    static const bool monitored = false;
    static const bool hysterically_debugged = false;

    typedef LIST<> DEVICES;
    typedef TLIST<> ASPECTS;
};

__END_SYS

#endif
