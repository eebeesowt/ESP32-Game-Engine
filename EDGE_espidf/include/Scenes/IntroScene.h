#pragma once
#include "SceneManager/Scene.h"
#include "SceneManager/SceneManager.h"
#include "InputManager/InputManager.h"
#include "Renderer/Renderer.h"

class IntroScene : public Scene {
    SceneManager* sm;
    InputManager* im;
public:
    IntroScene(SceneManager* manager, InputManager* input);
    void onEnter() override;
    void draw(Renderer& renderer) override;
};
