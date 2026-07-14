#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#define DEBUG_SERIAL true

// Set true untuk skip jadwal harian dan langsung mix saat startup (mode testing)
// Set false untuk production — sistem menunggu jadwal jam DAILY_MIX_HOUR
#define SKIP_DAILY_SCHEDULE true

#endif