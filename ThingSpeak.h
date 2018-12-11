#include "mbed.h"

int wifi_init();
void ChannelUpdate(float field1, float field2);

void NotifyDietAchieved(float weight, float target);
void NotifyDietNotAchieved(float weight, float target, float diff);
void NotifyUnlockFailedInvalidPass(void);
void NotifyUnlockFailedDoDiet(void);
void NotifyChangeTarget(int old_target, int new_target);
void NotifyLocked(void);
void NotifyUnlocked(void);
