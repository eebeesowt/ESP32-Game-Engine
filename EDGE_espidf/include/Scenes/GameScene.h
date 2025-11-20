#pragma once
#include "SceneManager/Scene.h"
#include "SceneManager/SceneManager.h"
#include "InputManager/InputManager.h"
#include "Renderer/Renderer.h"

class GameScene : public Scene {
    SceneManager* sm;
    InputManager* im;
public:
    GameScene(SceneManager* manager, InputManager* input);
    void onEnter() override;
    void draw(Renderer& renderer) override;
};
