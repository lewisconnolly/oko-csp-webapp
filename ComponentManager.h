#pragma once
#include "global.h"
#include "CSP/Systems/SystemsManager.h"
#include "CSP/Multiplayer/SpaceEntity.h"
#include "CSP/Multiplayer/SpaceEntitySystem.h"
#include "CSP/Multiplayer/ComponentBase.h"
#include "CSP/Multiplayer/Components/TextSpaceComponent.h"

using namespace csp;

class ComponentManager
{
public:
	ComponentManager();
	void SetText(promise<bool> Promise, multiplayer::TextSpaceComponent* TextComponent, common::String Text);
};

