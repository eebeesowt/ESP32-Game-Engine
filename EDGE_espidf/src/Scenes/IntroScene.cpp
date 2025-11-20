#include "Scenes/IntroScene.h"
#include "esp_log.h"

IntroScene::IntroScene(SceneManager* manager, InputManager* input) : sm(manager), im(input) {}

void IntroScene::onEnter() {
    ESP_LOGI("IntroScene", "Entered Intro Scene");
    // Register button listener to start game
    im->registerButtonListener(EDGE_Button::OK, EDGE_Event::PRESS, this, [this](){
        ESP_LOGI("IntroScene", "Button Pressed! Switching to GameScene...");
        sm->requestSetCurrentScene("GameScene");
    });
}

void IntroScene::draw(Renderer& renderer) {
    renderer.beginFrame();
    renderer.setContrast(255);
    
    // Clear screen (black)
    renderer.setColor(0, 0, 0);
    renderer.drawFilledRectangle(0, 0, 320, 170);
    
    // Draw Title
    renderer.setColor(255, 255, 0); // Yellow
    renderer.drawTextSafe(120, 60, "RPG-game");
    
    // Draw Instruction
    renderer.setColor(255, 255, 255); // White
    renderer.drawTextSafe(100, 100, "Press OK to Start");
    
    renderer.endFrame();
}
