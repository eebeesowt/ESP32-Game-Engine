#include "Scenes/GameScene.h"
#include "esp_log.h"

GameScene::GameScene(SceneManager* manager, InputManager* input) : sm(manager), im(input) {}

void GameScene::onEnter() {
    ESP_LOGI("GameScene", "Entered Game Scene");
    // Register button listener to go back to menu (Long Press)
    im->registerButtonListener(EDGE_Button::OK, EDGE_Event::LONG_PRESS, this, [this](){
            ESP_LOGI("GameScene", "Long Press! Returning to Intro...");
            sm->requestSetCurrentScene("IntroScene");
    });
}

void GameScene::draw(Renderer& renderer) {
    renderer.beginFrame();
    
    // Green background
    renderer.setColor(0, 100, 0); // Dark Green
    renderer.drawFilledRectangle(0, 0, 320, 170);
    
    // Game Text
    renderer.setColor(255, 255, 255);
    renderer.drawTextSafe(10, 10, "Game Running...");
    renderer.drawTextSafe(10, 30, "Long Press OK to Exit");
    
    // Draw a player placeholder
    renderer.setColor(255, 0, 0);
    renderer.drawFilledCircle(160, 85, 10);
    
    renderer.endFrame();
}
