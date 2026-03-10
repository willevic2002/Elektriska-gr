#ifndef LAB_CAN_H
#define LAB_CAN_H

#include <stdint.h>
#include <stdbool.h>

// Ensure LAB_NODE_ID is unique on the network.
// Each connected node should have its own ID (set via build configuration )


#ifndef LAB_NODE_ID
#error "LAB_NODE_ID must be defined in the build configuration(e.g.-DLAB_NODE_ID =1)."
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// Optional: build -time hint (shows active node during build output)
#pragma message ("Building firmware for LAB_NODE_ID = " STR(LAB_NODE_ID))

// Periods (ms)
#ifndef LAB_PERIOD_STATUS_MS
#define LAB_PERIOD_STATUS_MS 100
#endif

#ifndef LAB_PERIOD_SENSOR_MS
#define LAB_PERIOD_SENSOR_MS 200
#endif

// ID plan (broadcast)
#define LAB_ID_STATUS_BASE 0x100u // + node_id
#define LAB_ID_SENSOR_BASE 0x200u // + node_id
#define LAB_ID_EVENT_BASE 0x300u // + node_id

// Arbitration demo IDs (lower wins)
#define LAB_ID_ARB_LOW 0x080u
#define LAB_ID_ARB_HIGH 0x100u


void LAB_CAN_PrintBanner (void);
bool LAB_CAN_InitAndStart (void);
void LAB_CAN_Periodic (void);

// HAL callbacks forward here
void LAB_CAN_OnRxFifo0Pending (void);
void LAB_CAN_OnError (void);

// Optional: used for arbitration demo (called from button handling)
void LAB_CAN_SendArbLow (void);
void LAB_CAN_SendArbHigh (void);

#endif
