#pragma once

#include "global.h"
#include "CSP/CSPFoundation.h"
#include "CSP/Systems//Users//UserSystem.h"
#include "CSP/Systems//SystemsManager.h"

using namespace csp;

class Init
{
public:
	
	Init(common::String EnvUrl, common::String Tenant);
	void LoginToTenant(promise<common::String> OnLoginPromise, common::String Email, common::String Password);	

private:
	
	ClientUserAgent UserAgent;
};

