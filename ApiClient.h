#pragma once

#include "global.h"
#include "Init.h"
#include "SpaceManager.h"
#include "ComponentManager.h"

class ApiClient
{
public:

	ApiClient();

	// Make an async call to any function, with any return type 
	template <typename ReturnType, typename Func, typename Instance, typename... Args>
	ReturnType CallAsyncFunction(Func MemberFunc, Instance* ObjInstance, Args&&... args);

	bool Authenticate(common::String Email, common::String Password);
	bool IsAuthenticated();
	string GetUsername();

private:

	Init* Initialise;
	SpaceManager* SpaceMgr;
	ComponentManager* ComponentMgr;
	
	bool IsAuthenticated;
	string Username;
};
