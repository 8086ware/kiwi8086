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

typedef struct Pit_timer
{
	_Bool received_reload_low;
	_Bool sent_current_low;

	uint16_t current_count;
	uint16_t reload_value;
} Pit_timer;

typedef struct Pit
{
	uint64_t last_tick;
	Pit_timer timers[3];
} Pit;

#endif
