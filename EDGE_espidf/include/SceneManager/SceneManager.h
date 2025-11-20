#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <cstdint>
#include "SceneManager/Scene.h"

#define MAX_SCENES 5

// Forward declarations
class InputManager;
class Renderer;

// Define the logger type directly to break circular dependency with EDGE.h
using EDGELogger = std::function<void(const char* message)>;

using SceneFactoryFunction = std::function<Scene*(void* configData)>;


class SceneManager {
public:
    SceneManager();
    ~SceneManager();
    void processSceneChanges();

    void setInputManager(InputManager* manager);
    void setLogger(EDGELogger logger);

    bool registerScene(const std::string& name, SceneFactoryFunction factory);

    bool setCurrentScene(const std::string& sceneName, void* configData = nullptr);
    bool pushScene(const std::string& sceneName, void* configData = nullptr);
    bool popScene();

    void requestSetCurrentScene(const std::string& sceneName, void* configData = nullptr);
    void requestPushScene(const std::string& sceneName, void* configData = nullptr);

    void update(unsigned long dt);
    void draw(Renderer& rendererRef);
    Scene* getCurrentScene() const;
    std::string getCurrentSceneName() const;
    std::string getPreviousSceneName() const;

    SceneFactoryFunction getFactoryByName(const std::string& name) const;
    std::vector<std::string> getRegisteredSceneNames() const;

    bool isSceneChangePending() const;
    std::string getPendingSceneName() const;
    void* getPendingConfigData() const;
    bool getPendingReplaceStack() const;
    void clearPendingSceneChange();

private:
    Scene* sceneStack[MAX_SCENES] = {nullptr};
    int sceneCount = 0;
    std::string _sceneNameStack[MAX_SCENES];
    std::string _previousSceneName = "";

    InputManager* inputManager = nullptr;
    EDGELogger _logger;

    std::map<std::string, SceneFactoryFunction> _sceneFactories;

    std::string _pendingNextSceneName = "";
    void* _pendingConfigData = nullptr;
    bool _pendingReplaceStack = true;
    bool _pendingSceneChange = false;

    Scene* createSceneByName(const std::string& sceneName, void* configData);
    void clearStack();
};
