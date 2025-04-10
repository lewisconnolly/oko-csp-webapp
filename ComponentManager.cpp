#include "ComponentManager.h"

ComponentManager::ComponentManager()
{
}

void ComponentManager::SetText(promise<bool> Promise, multiplayer::TextSpaceComponent* TextComponent, common::String Text)
{
    systems::SystemsManager& SystemManager = systems::SystemsManager::Get();
    multiplayer::SpaceEntitySystem* SpaceEntitySystem = SystemManager.GetSpaceEntitySystem();

    // Create a shared_ptr to manage the lifetime of the promise
    // (Necessary because Promise will be removed from memory after this function returns but before the callback has been executed)
    auto PromisePtr = make_shared<promise<bool>>(move(Promise));
    
    cout << "Setting text on \"" << TextComponent->GetComponentName() << "\" component..." << endl;

    TextComponent->SetText(Text);
    
    cout << "Sending update patch..." << endl;    

    TextComponent->GetParent()->SetPatchSentCallback([PromisePtr](bool Success)
        {
            if (Success)
            {
                cout << "Update patch sent" << endl;
            }
            else
            {
                cout << "Update patch failed to send" << endl;
            }

            PromisePtr->set_value(Success);
        });

    TextComponent->GetParent()->QueueUpdate();    
    SpaceEntitySystem->ProcessPendingEntityOperations();
}