#pragma once
#include "TrunkBehaviour.h"

#include "Gameplay/GameObject.h"

#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

// GetGameObject()->GetRotationEuler();

void TrunkBehaviour::Update(float deltaTime) {
	time += deltaTime;
	GetGameObject()->SetRotation(startRotation + glm::vec3(0, sin(time), 0));
}

void TrunkBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Wind Speed", &WindSpeed);
}

nlohmann::json TrunkBehaviour::ToJson() const {
	return {
		{ "speed", WindSpeed }
	};
}

TrunkBehaviour::Sptr TrunkBehaviour::FromJson(const nlohmann::json& data) {
	TrunkBehaviour::Sptr result = std::make_shared<TrunkBehaviour>();
	result->WindSpeed = JsonGet(data, "speed", result->WindSpeed);
	return result;
}