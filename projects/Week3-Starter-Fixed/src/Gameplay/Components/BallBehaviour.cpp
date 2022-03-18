#pragma once
#include "BallBehaviour.h"

#include "Gameplay/GameObject.h"

#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

// GetGameObject()->GetRotationEuler();

void BallBehaviour::Update(float deltaTime) {
	time += deltaTime;
	GetGameObject()->SetRotation(startRotation + glm::vec3(cos(time) * BallSpeed, 0, sin(time) * BallSpeed));
	GetGameObject()->SetPostion(startPosition + glm::vec3(0, 0, sin(time)/16));
}

void BallBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Wind Speed", &BallSpeed);
}

nlohmann::json BallBehaviour::ToJson() const {
	return {
		{ "speed", BallSpeed }
	};
}

BallBehaviour::Sptr BallBehaviour::FromJson(const nlohmann::json& data) {
	BallBehaviour::Sptr result = std::make_shared<BallBehaviour>();
	result->BallSpeed = JsonGet(data, "speed", result->BallSpeed);
	return result;
}