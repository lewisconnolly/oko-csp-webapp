#include "ApiClient.h"

ApiClient::ApiClient()
{
    cout << "Initialising connection to tenant..." << endl;
    // Initialise connection to OKO tenant
    Initialise = new Init("https://ogs.magnopus.cloud/", "OKO");

    // Initialise custom CPS managers
    SpaceMgr = new SpaceManager();
    ComponentMgr = new ComponentManager();
}

template <typename ReturnType, typename Func, typename Instance, typename... Args>
ReturnType ApiClient::CallAsyncFunction(Func MemberFunc, Instance* ObjInstance, Args&&... args)
{
    // Create promise and future
    promise<ReturnType> Promise;
    future<ReturnType> Future = Promise.get_future();

    // Start thread - directly call the function with the promise and args
    thread Thread(
        MemberFunc,
        ObjInstance,
        move(Promise),
        forward<Args>(args)...
    );

    ReturnType Result = ReturnType(); // Set to default for type

    // Wait for callback with timeout
    future_status Status = Future.wait_for(chrono::seconds(60));
    if (Status == future_status::timeout)
    {
        cout << "Timeout waiting for callback!" << endl;
    }
    else
    {
        Result = Future.get(); // Get the result        
    }

    // Clean up
    Thread.join();

    return Result;
}

bool ApiClient::Authenticate(common::String Email, common::String Password)
{
    cout << "Logging in to tenant..." << endl;
    // Log in to tenant with command line email and password arguments
    common::String LoginResult = CallAsyncFunction<common::String>(&Init::LoginToTenant, Initialise, Email, Password);        

    cout << "Login result: " << LoginResult << endl;

    if (LoginResult != "Success")
    {    
        IsAuthenticated = true;
		Username = Email;

        return true;
    }

    IsAuthenticated = false;

    return false;
}

bool ApiClient::IsAuthenticated()
{
	return IsAuthenticated;
}   

string ApiClient::GetUsername()
{
	return Username;
}