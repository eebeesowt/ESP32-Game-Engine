#pragma once
#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <deque>
#include <cstdint>

// FreeRTOS Queue (used as type only)
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "SceneManager/Scene.h"

// Define the logger type directly to break circular dependency with EDGE.h
using EDGELogger = std::function<void(const char* message)>;

// --- Abstract Input Definitions ---
enum class EDGE_Button {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    OK,
    CANCEL
};

enum class EDGE_Event {
    PRESS,
    RELEASE,
    CLICK,
    LONG_PRESS
};

// --- Abstract Encoder Definitions ---
// Keep encoder API minimal: rotation direction only.
// Button events are already represented by EDGE_Event and should be
// delivered via the regular button API. This enum represents only
// rotation actions coming from a rotary encoder.
enum class EDGE_EncoderRotate {
    ROTATE_CW,
    ROTATE_CCW
};
// --- End Encoder Definitions ---

// Forward declarations
class SceneManager;

class InputManager {
public:
    using DeferredAction = std::function<void()>;

    struct DeferredActionEntry {
        Scene* ownerScene;
        DeferredAction action;
    };

    struct ListenerInfo {
        EDGE_Button button;
        EDGE_Event eventType;
        Scene* scene;
        DeferredAction callback;

        bool operator==(const ListenerInfo& other) const {
            return button == other.button && eventType == other.eventType && scene == other.scene;
        }
    };

    void init();
    void update(unsigned long dt);
    void setLogger(EDGELogger logger) { _logger = logger; }

    void setSceneManager(SceneManager* sm);

    bool registerButtonListener(EDGE_Button button, EDGE_Event eventType, Scene* scene, DeferredAction callback);
    void unregisterButtonListener(EDGE_Button button, EDGE_Event eventType, Scene* scene);
    void unregisterAllListenersForScene(Scene* scene);
    void processButtonEvent(EDGE_Button button, EDGE_Event eventType);

    // Encoder API (rotation only)
    bool registerEncoderListener(int encoderId, EDGE_EncoderRotate eventType, Scene* scene, DeferredAction callback);
    void unregisterEncoderListener(int encoderId, EDGE_EncoderRotate eventType, Scene* scene);
    void processEncoderEvent(int encoderId, EDGE_EncoderRotate eventType);

    void clearDeferredActionsForScene(Scene* scene);

private:
    std::vector<ListenerInfo> listeners;
    // Encoder listeners
    struct EncoderListenerInfo {
        int encoderId;
        EDGE_EncoderRotate eventType;
        Scene* scene;
        DeferredAction callback;

        bool operator==(const EncoderListenerInfo& other) const {
            return encoderId == other.encoderId && eventType == other.eventType && scene == other.scene;
        }
    };
    std::vector<EncoderListenerInfo> encoderListeners;
    SceneManager* sceneManager = nullptr;
    EDGELogger _logger;

    std::deque<DeferredActionEntry> _deferredActionsQueue;
    void deferAction(Scene* ownerScene, DeferredAction action);
};
