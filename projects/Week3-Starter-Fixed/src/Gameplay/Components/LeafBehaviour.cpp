#pragma once
#include "LeafBehaviour.h"

#include "Gameplay/GameObject.h"

#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

// GetGameObject()->GetRotationEuler();

void LeafBehaviour::Update(float deltaTime) {
	time += deltaTime;
	GetGameObject()->SetRotation(startRotation + glm::vec3(cos(time)*WindSpeed, 0, sin(time)*WindSpeed));
	GetGameObject()->SetPostion(startPosition + glm::vec3(sin(time)/32, 0, (sin((time*2)+1)-1)/256));
}

void LeafBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Wind Speed", &WindSpeed);
}

nlohmann::json LeafBehaviour::ToJson() const {
	return {
		{ "speed", WindSpeed }
	};
}

LeafBehaviour::Sptr LeafBehaviour::FromJson(const nlohmann::json& data) {
	LeafBehaviour::Sptr result = std::make_shared<LeafBehaviour>();
	result->WindSpeed = JsonGet(data, "speed", result->WindSpeed);
	return result;
}