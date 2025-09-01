#ifndef PIT_H
#define PIT_H

#define PIT_NANOSECONDS_PER_CYCLE 838 // or pulse each 1/11931812 

enum Pit_Port
{
	PIT_CHANNEL_0_PORT = 0x40,
	PIT_CHANNEL_1_PORT = 0x41,
	PIT_CHANNEL_2_PORT = 0x42,

	PIT_MODE_COMMAND_REG_PORT = 0x43,
};

enum Pit_Operating_Mode
{
	PIT_OPERATING_INTERRUPT_ON_TERMINAL_COUNT = 0,
	PIT_OPERATING_HARDWARE_RETRIGGERABLE_ONESHOT = 1,
	PIT_OPERATING_RATE_GENERATOR = 2,
	PIT_OPERATING_SQUARE_WAVE_GENERATOR = 3,
	PIT_OPERATING_SOFTWARE_TRIGGERED_STROBE = 4,
	PIT_OPERATING_HARDWARE_TRIGGERED_STROBE = 5,
};

enum Pit_Access_Mode
{
	PIT_ACCESS_LATCH_COUNT = 0,
	PIT_ACCESS_LOBYTE_ONLY = 1,
	PIT_ACCESS_HIBYTE_ONLY = 2,
	PIT_ACCESS_LOBYTE_HIBYTE = 3,
};

typedef struct Pit_timer
{
	_Bool received_reload_low;
	_Bool sent_current_low;

	uint16_t current_count;
	uint16_t reload_value;

	enum Pit_Operating_Mode operating_mode;
	enum Pit_Access_Mode access_mode;
} Pit_timer;

typedef struct Pit
{
	uint64_t last_tick;
	Pit_timer timers[3];
} Pit;

#endif
