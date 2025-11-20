#include "SceneManager/Scene.h"
#include <cstdarg>

// Define the static logger member
EDGELogger Scene::_masterLogger = nullptr;

Scene::~Scene() {
}

void Scene::init() {
    managesOwnDrawing = false;
}

void Scene::update(unsigned long /*deltaTime*/) {
    // Default: nothing
}

void Scene::draw(Renderer& /*renderer*/) {
    // Default: nothing
}
