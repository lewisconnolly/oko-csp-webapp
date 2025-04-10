#include "Init.h"

Init::Init(common::String EnvUrl, common::String Tenant)
{    
    // Set endpoints for various services required for CSPFoundation, pass over client header information,
    // initialise systems required for CPSFoundation    
    bool InitResult = CSPFoundation::Initialise(EnvUrl, Tenant);
    
    cout << "Initialisation result: " << (InitResult ? "Success" : "Failure") << endl;

    // Set user agent information used for all requests made by CSP
    UserAgent.CSPVersion = CSPFoundation::GetVersion();
    UserAgent.ClientOS = "Win11";
    UserAgent.ClientSKU = "oko-csp-app";
    UserAgent.ClientVersion = "1";
    UserAgent.ClientEnvironment = "C++";

    CSPFoundation::SetClientUserAgentInfo(UserAgent);
}

void Init::LoginToTenant(promise<common::String> OnLoginPromise, common::String Email, common::String Password)
{
    systems::SystemsManager& SystemManager = systems::SystemsManager::Get();
    systems::UserSystem* UserSystem = SystemManager.GetUserSystem();

    // Create a shared_ptr to manage the lifetime of the promise
    // (Necessary because OnLoginPromise will be removed from memory after this function returns but before the callback has been exectuted)
    auto PromisePtr = make_shared<promise<common::String>>(move(OnLoginPromise));

    // Define login callback 
    auto OnLogin = [PromisePtr](const systems::LoginStateResult& Result) mutable
        {
            systems::EResultCode ResCode = Result.GetResultCode();

            // Some endpoints (like AssetSystem::UploadAssetData()) report progress status.
            // This endpoint does not, so it can be ignored.
            if (ResCode == systems::EResultCode::InProgress)
            {
                cout << "Waiting for authentication response..." << endl;
                return;
            }

            if (ResCode == systems::EResultCode::Success)
            {
                cout << "Authentication succeeded" << endl;
                PromisePtr->set_value("Success");
                return;
            }

            // If the request fails, ResultCode will be set to EResultCode::Failed
            if (ResCode == systems::EResultCode::Failed) {
                
                cout << "Authentication failed" << endl;

                // ResultBase::FailureReason will be set if the request failed and should
                // be used to determine the cause of the failure.
                auto reason = static_cast<systems::ERequestFailureReason>(Result.GetFailureReason());

                switch (reason) {
                case systems::ERequestFailureReason::UserUnverifiedEmail:
                    cout << "User email not verified! Please follow the link in the verification email sent to you after registering an account." << endl;
                    PromisePtr->set_value("UserUnverifiedEmail");
                    break;
                case systems::ERequestFailureReason::UserAgeNotVerified:
                    cout << "User has not confirmed they are over 18! Either you are too young or should re-enter your DoB on the sign in page." << endl;
                    PromisePtr->set_value("UserAgeNotVerified");
                    break;
                case systems::ERequestFailureReason::Unknown:
                    // If FailureReason is unknown, check the HTTP response code.
                    // If it is 403, the provided account credentials were incorrect.
                    cout << "Request failed with HTTP response code " << Result.GetHttpResultCode() << endl;
                    PromisePtr->set_value("Unknown");
                    break;
                default:
                    PromisePtr->set_value("Failed");
                }

                return;
            }
        };

    // Log in using a registered account
    // The 4th parameter is a boolean indicating whether the user has verified their age or not,
    // and can be null to use the value supplied during a previous call to Login() or CreateUser()
    UserSystem->Login("", Email, Password, true, OnLogin);
}
