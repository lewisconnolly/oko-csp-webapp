#include "SpaceManager.h"

SpaceManager::SpaceManager()
{
}

void SpaceManager::GetSpaceByName(promise<const systems::Space*> Promise,  common::String Name)
{
	systems::SystemsManager& SystemManager = systems::SystemsManager::Get();
	systems::SpaceSystem* SpaceSystem = SystemManager.GetSpaceSystem();
    
    // Create a shared_ptr to manage the lifetime of the promise
    // (Necessary because Promise will be removed from memory after this function returns but before the callback has been executed)
    auto PromisePtr = make_shared<promise<const systems::Space*>>(move(Promise));
    auto NamePtr = make_shared<common::String>(move(Name)); // Do same for Name param

    auto OnQuery = [PromisePtr, NamePtr](const systems::SpacesResult& Result)
        {
            systems::EResultCode ResCode = Result.GetResultCode();
            const systems::Space* Space;
            
            // Respond to a successful query
            if (ResCode == systems::EResultCode::Success)
            {
                cout << "Request succeeded" << endl;

                // Search for space by name
                for (int i = 0; i < Result.GetSpaces().Size(); i++)
                {
                    if (Result.GetSpaces()[i].Name == *NamePtr)
                    {
                        cout << "Space found: " << Result.GetSpaces()[i].Name << " (ID: " << Result.GetSpaces()[i].Id << ")" << endl;
                        Space = &(Result.GetSpaces()[i]);
                    }
                }

                PromisePtr->set_value(Space);
                
                return true;
            }
            else if (ResCode == csp::systems::EResultCode::InProgress)
            {
                // Report progress
                cout << "Progress: " << Result.GetRequestProgress() << endl;
                return true;
            }
            else
            {
                // Check failure reasons
                if (ResCode == systems::EResultCode::Failed) {
                    
                    auto reason = static_cast<systems::ERequestFailureReason>(Result.GetFailureReason());

                    switch (reason) {
                    case systems::ERequestFailureReason::Unknown:
                        cout << "Request failed with HTTP response code " << Result.GetHttpResultCode() << endl;
                        break;
                    default:
                        cout << "Request failed" << endl;
                    }                    
                }

                PromisePtr->set_value(Space);

                return false;
            }
        };

    // Get spaces from logged in account
    SpaceSystem->GetSpaces(OnQuery);
}

multiplayer::ComponentBase* SpaceManager::GetComponentByName(common::String Name, common::String EntityName)
{
    // Get space entity system
    systems::SystemsManager& SystemManager = systems::SystemsManager::Get();
    multiplayer::SpaceEntitySystem* SpaceEntitySystem = SystemManager.GetSpaceEntitySystem();
       
    multiplayer::ComponentBase* Component = nullptr;
   
    multiplayer::SpaceEntity* SpaceEntity = SpaceEntitySystem->FindSpaceEntity(EntityName);   
    const common::Map<unsigned short, multiplayer::ComponentBase*>* ComponentMap = SpaceEntity->GetComponents();    
    const common::Array<multiplayer::ComponentBase*>* Components = ComponentMap->Values();
    
    if (ComponentMap->Size() == 0)
    {
        cout << "No component found" << endl;
        return Component;
    }

    for (multiplayer::ComponentBase* const* it = Components->begin(); it != Components->end(); ++it)
    {
        common::String ComponentName = (*it)->GetComponentName();        
        if (ComponentName == Name)
        {
            cout << "\"" << Name << "\" component found" << endl;
            Component = *it;
        }
    }

    return Component;
}

void SpaceManager::EnterSpace(promise<common::String> Promise, common::String SpaceId)
{
    systems::SystemsManager& SystemManager = systems::SystemsManager::Get();
    systems::SpaceSystem* SpaceSystem = SystemManager.GetSpaceSystem();

    // Create a shared_ptr to manage the lifetime of the promise
    // (Necessary because Promise will be removed from memory after this function returns but before the callback has been executed)
    auto PromisePtr = make_shared<promise<common::String>>(move(Promise));
    auto IdPtr = make_shared<common::String>(move(SpaceId)); // Do same for Name param

    auto OnEnter = [PromisePtr, IdPtr](const systems::NullResult& Result)
        {
            systems::EResultCode ResCode = Result.GetResultCode();
           			
            // Respond to a successful entry
            if (ResCode == systems::EResultCode::Success)
            {
                cout << "Request succeeded" << endl;
				cout << "Entered space with ID: " << *IdPtr << endl;

                PromisePtr->set_value("Success");

                return true;
            }
            else if (ResCode == csp::systems::EResultCode::InProgress)
            {
                // Report progress
                cout << "Progress: " << Result.GetRequestProgress() << endl;
                return true;
            }
            else
            {
                cout << "Request failed" << endl;
                
                // Check failure reasons
                if (ResCode == systems::EResultCode::Failed) {

                    auto reason = static_cast<systems::ERequestFailureReason>(Result.GetFailureReason());

                    switch (reason) {
                    case systems::ERequestFailureReason::Unknown:
                        cout << "Request failed with HTTP response code " << Result.GetHttpResultCode() << endl;
                        break;
                    default:
                        cout << "Request failed" << endl;
                    }
                }

                cout << "Failed to enter space with ID: " << *IdPtr << endl;

                PromisePtr->set_value("Failed");

                return false;
            }
        };

    // Get spaces from logged in account
    SpaceSystem->EnterSpace(*IdPtr, OnEnter);
}

void SpaceManager::WaitForEntityRetrieval(promise<bool> Promise)
{
    systems::SystemsManager& SystemManager = systems::SystemsManager::Get();
    multiplayer::SpaceEntitySystem* SpaceEntitySystem = SystemManager.GetSpaceEntitySystem();

    // Create a shared_ptr to manage the lifetime of the promise
    // (Necessary because Promise will be removed from memory after this function returns but before the callback has been executed)
    auto PromisePtr = make_shared<promise<bool>>(move(Promise));

    SpaceEntitySystem->SetInitialEntitiesRetrievedCallback([PromisePtr](bool Success)
        {
            if (Success)
            {
				cout << "Space entities retrieved" << endl;
            }
            else
            {
                cout << "Failed to retrieve space entities" << endl;
            }

            PromisePtr->set_value(Success);
        });    
}
