#include "Core/ActivityMonitor.h"
#include "Core/EDGETime.h"

static unsigned long s_lastActivity = 0;

// Update the timestamp of last user activity (called by InputManager)
void updateLastActivityTime() {
    s_lastActivity = edge_millis();
}

// Optional getter for debugging
unsigned long getLastActivityTime() {
    return s_lastActivity;
}
