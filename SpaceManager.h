#pragma once
#include "global.h"
#include "CSP/Systems/SystemsManager.h"
#include "CSP/Systems/Spaces/SpaceSystem.h"
#include "CSP/Systems/Spaces/Space.h"
#include "CSP/Multiplayer/SpaceEntitySystem.h"
#include "CSP/Multiplayer/SpaceEntity.h"
#include "CSP/Multiplayer/ComponentBase.h"

using namespace csp;

class SpaceManager
{
public:
	
	SpaceManager();
	void EnterSpace(promise<common::String> Promise, common::String SpaceId);
	void GetSpaceByName(promise<const systems::Space*> Promise, common::String Name);
	multiplayer::ComponentBase* GetComponentByName(common::String Name, common::String EntityName);
	void WaitForEntityRetrieval(promise<bool> Promise);

private:

};

