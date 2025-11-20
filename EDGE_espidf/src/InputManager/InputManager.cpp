#include "InputManager/InputManager.h"
#include "SceneManager/SceneManager.h"
#include "Core/ActivityMonitor.h"
#include <algorithm>
#include <cstdio>

void InputManager::init() {
    listeners.clear();
    _deferredActionsQueue.clear();
}

void InputManager::update(unsigned long dt) {
    int actionsToProcessThisFrame = (int)_deferredActionsQueue.size();
    actionsToProcessThisFrame = std::min(actionsToProcessThisFrame, 3);

    for (int i = 0; i < actionsToProcessThisFrame && !_deferredActionsQueue.empty(); ++i) {
        DeferredActionEntry entry = _deferredActionsQueue.front();
        _deferredActionsQueue.pop_front();

        if (entry.action) {
            if (_logger) { char buf[128]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] Executing deferred action for scene %p.", (void*)entry.ownerScene); _logger(buf); }
            entry.action();
        }
    }
}

void InputManager::setSceneManager(SceneManager* sm) {
    sceneManager = sm;
}

bool InputManager::registerButtonListener(EDGE_Button button, EDGE_Event eventType, Scene* scene, DeferredAction callback) {
    if (!scene || !callback) {
        if (_logger) _logger("[HARDWARE_INPUT] Error: Invalid scene or callback provided for listener registration.");
        return false;
    }
    listeners.push_back({button, eventType, scene, callback});
    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Registered listener for button %d, event %d, scene %p", (int)button, (int)eventType, (void*)scene); _logger(buf); }
    return true;
}

void InputManager::unregisterButtonListener(EDGE_Button button, EDGE_Event eventType, Scene* scene) {
    listeners.erase(
        std::remove_if(listeners.begin(), listeners.end(),
                       [button, eventType, scene](const ListenerInfo& listener) {
                           return listener.button == button && listener.eventType == eventType && listener.scene == scene;
                       }),
        listeners.end());
    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Unregistered listener for button %d, event %d, scene %p", (int)button, (int)eventType, (void*)scene); _logger(buf); }
}

void InputManager::unregisterAllListenersForScene(Scene* scene) {
    if (!scene) return;
    listeners.erase(
        std::remove_if(listeners.begin(), listeners.end(),
                       [scene](const ListenerInfo& listener) {
                           return listener.scene == scene;
                       }),
        listeners.end());
    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Unregistered all listeners for scene %p", (void*)scene); _logger(buf); }
}

bool InputManager::registerEncoderListener(int encoderId, EDGE_EncoderRotate eventType, Scene* scene, DeferredAction callback) {
    if (!scene || !callback) {
        if (_logger) _logger("[HARDWARE_INPUT] Error: Invalid scene or callback provided for encoder listener registration.");
        return false;
    }
    encoderListeners.push_back({encoderId, eventType, scene, callback});
    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Registered encoder listener for encoder %d, event %d, scene %p", encoderId, (int)eventType, (void*)scene); _logger(buf); }
    return true;
}

void InputManager::unregisterEncoderListener(int encoderId, EDGE_EncoderRotate eventType, Scene* scene) {
    encoderListeners.erase(
        std::remove_if(encoderListeners.begin(), encoderListeners.end(),
                       [encoderId, eventType, scene](const EncoderListenerInfo& listener) {
                           return listener.encoderId == encoderId && listener.eventType == eventType && listener.scene == scene;
                       }),
        encoderListeners.end());
    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Unregistered encoder listener for encoder %d, event %d, scene %p", encoderId, (int)eventType, (void*)scene); _logger(buf); }
}

void InputManager::processEncoderEvent(int encoderId, EDGE_EncoderRotate eventType) {
    updateLastActivityTime();

    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager::processEncoderEvent: Encoder %d event %d", encoderId, (int)eventType); _logger(buf); }

    if (!sceneManager) {
        if (_logger) _logger("[HARDWARE_INPUT] InputManager Warning: SceneManager not set during encoder event processing!");
        return;
    }

    Scene* currentScene = sceneManager->getCurrentScene();
    std::string currentSceneName = sceneManager->getCurrentSceneName();
    if (!currentScene) {
        if (_logger) _logger("[HARDWARE_INPUT] InputManager Warning: No active scene to process encoder event!");
        return;
    }

    // Notify matching encoder listeners (deferred execution)
    bool eventDeferred = false;
    for (const auto& listener : encoderListeners) {
        if (listener.encoderId == encoderId && listener.eventType == eventType && listener.scene == currentScene) {
            if (listener.callback) {
                deferAction(currentScene, listener.callback);
                eventDeferred = true;
                if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Deferred encoder callback for encoder %d, event %d on scene %p.", encoderId, (int)eventType, (void*)currentScene); _logger(buf); }
            }
        }
    }

    if (!eventDeferred) {
        if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: No encoder callback found or deferred for encoder %d, event %d on scene %p.", encoderId, (int)eventType, (void*)currentScene); _logger(buf); }
    }
}

void InputManager::processButtonEvent(EDGE_Button button, EDGE_Event eventType) {
    updateLastActivityTime();

    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager::processButtonEvent: Received button %d, event %d", (int)button, (int)eventType); _logger(buf); }

    if (!sceneManager) {
        if (_logger) _logger("[HARDWARE_INPUT] InputManager Warning: SceneManager not set during event processing!");
        return;
    }

    Scene* currentScene = sceneManager->getCurrentScene();
    std::string currentSceneName = sceneManager->getCurrentSceneName();
    if (!currentScene) {
        if (_logger) _logger("[HARDWARE_INPUT] InputManager Warning: No active scene to process event!");
        return;
    }
    if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Current scene name: %s", currentSceneName.c_str()); _logger(buf); }

    bool eventDeferred = false;
    for (const auto& listener : listeners) {
        if (listener.button == button && listener.eventType == eventType && listener.scene == currentScene) {
            if (listener.callback) {
                deferAction(currentScene, listener.callback);
                eventDeferred = true;
                if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Deferred direct callback for button %d, event %d on scene %p.", (int)button, (int)eventType, (void*)currentScene); _logger(buf); }
            }
        }
    }

    if (!eventDeferred) {
        if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: No direct callback found or deferred for button %d, event %d on scene %p.", (int)button, (int)eventType, (void*)currentScene); _logger(buf); }
    }
}

void InputManager::deferAction(Scene* ownerScene, DeferredAction action) {
    if (!ownerScene || !action) return;
    _deferredActionsQueue.push_back({ownerScene, action});
}

void InputManager::clearDeferredActionsForScene(Scene* scene) {
    if (!scene) return;
    size_t initialSize = _deferredActionsQueue.size();
    _deferredActionsQueue.erase(
        std::remove_if(_deferredActionsQueue.begin(), _deferredActionsQueue.end(),
                       [scene](const DeferredActionEntry& entry) {
                           return entry.ownerScene == scene;
                       }),
        _deferredActionsQueue.end());
    size_t removedCount = initialSize - _deferredActionsQueue.size();
    if (removedCount > 0) {
        if (_logger) { char buf[256]; snprintf(buf, sizeof(buf), "[HARDWARE_INPUT] InputManager: Cleared %u deferred actions for scene %p", (unsigned int)removedCount, (void*)scene); _logger(buf); }
    }
}
