#include "oko-csp-webapp.h"
#include "global.h"
#include "ApiClient.h"
#include <crow.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>

int main()
{    
    // Set up web server
    crow::SimpleApp app;

	// Initialise API client
	ApiClient ApiClient;        

    // Serve static files
    CROW_ROUTE(app, "/")
        ([](const crow::request&) {
        std::ifstream file("public/index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        return crow::response(buffer.str());
            });

    // Create static file route for CSS and JS
    CROW_ROUTE(app, "/public/<path>")
        ([](const std::string& path) {
        std::ifstream file("public/" + path);
        if (!file) {
            return crow::response(404);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        crow::response res(buffer.str());

        if (path.ends_with(".css")) {
            res.set_header("Content-Type", "text/css");
        }
        else if (path.ends_with(".js")) {
            res.set_header("Content-Type", "application/javascript");
        }

        return res;
            });

    // API endpoint for login
    CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::POST)
        ([&ApiClient](const crow::request& req) {
        auto json = crow::json::load(req.body);

        if (!json) {
            return crow::response(400, R"({"status": "error", "message": "Invalid JSON"})");
        }

        std::string username = json["username"].s();
        std::string password = json["password"].s();        

        bool success = ApiClient.Authenticate(common::String(username.c_str()), common::String(password.c_str()));

        if (success) {
            return crow::response(200, R"({"status": "success", "username": ")" + username + R"("})");
        }
        else {
            return crow::response(401, R"({"status": "error", "message": "Authentication failed"})");
        }
            });

    // API endpoint for logout
    /*CROW_ROUTE(app, "/api/logout").methods(crow::HTTPMethod::POST)
        ([&ApiClient](const crow::request&) {
        apiClient.logout();
        return crow::response(200, R"({"status": "success"})");
            });*/

    // API endpoint for status
    CROW_ROUTE(app, "/api/status").methods(crow::HTTPMethod::GET)
        ([&ApiClient](const crow::request&) {
        crow::json::wvalue response;
        response["authenticated"] = ApiClient.IsAuthenticated();

        if (ApiClient.IsAuthenticated()) {
            response["username"] = ApiClient.GetUsername();
        }

        return crow::response(200, response);
            });

    // API endpoint for executing commands
    CROW_ROUTE(app, "/api/execute").methods(crow::HTTPMethod::POST)
        ([&apiClient](const crow::request& req) {
        if (!apiClient.isAuthenticated()) {
            return crow::response(401, R"({"status": "error", "message": "Not authenticated"})");
        }

        auto json = crow::json::load(req.body);

        if (!json) {
            return crow::response(400, R"({"status": "error", "message": "Invalid JSON"})");
        }

        std::string command = json["command"].s();
        std::string params = json["params"].s();

        std::string result = apiClient.executeCommand(command, params);

        return crow::response(200, result);
            });

    // API endpoint for getting logs
    CROW_ROUTE(app, "/api/logs").methods(crow::HTTPMethod::GET)
        ([&apiClient](const crow::request&) {
        if (!apiClient.isAuthenticated()) {
            return crow::response(401, R"({"status": "error", "message": "Not authenticated"})");
        }

        crow::json::wvalue response;
        std::vector<std::string> logs = apiClient.getLogs();

        crow::json::wvalue::list logList;
        for (const auto& log : logs) {
            logList.push_back(log);
        }

        response["logs"] = logList;

        return crow::response(200, response);
            });

    // API endpoint for getting available commands
    CROW_ROUTE(app, "/api/commands").methods(crow::HTTPMethod::GET)
        ([&apiClient](const crow::request&) {
        if (!apiClient.isAuthenticated()) {
            return crow::response(401, R"({"status": "error", "message": "Not authenticated"})");
        }

        // Return list of available commands and their descriptions
        return crow::response(200, R"({
            "commands": [
                {"name": "get_objects", "description": "Get list of all objects", "params": ""},
                {"name": "get_object_details", "description": "Get details for specific object", "params": "object_id"},
                {"name": "set_position", "description": "Set object position", "params": "object_id"},
                {"name": "set_rotation", "description": "Set object rotation", "params": "object_id"},
                {"name": "set_scale", "description": "Set object scale", "params": "object_id"},
                {"name": "create_object", "description": "Create new object", "params": "object_type"},
                {"name": "delete_object", "description": "Delete an object", "params": "object_id"}
            ]
        })");
            });

    // Create the static files if they don't exist
    std::ofstream indexFile("public/index.html");
    indexFile << R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>3D Environment API Client</title>
    <link rel="stylesheet" href="/public/styles.css">
</head>
<body>
    <div class="container">
        <h1>3D Environment API Client</h1>
        
        <div id="login-section" class="section">
            <h2>Login</h2>
            <div class="input-group">
                <label for="username">Username:</label>
                <input type="text" id="username" placeholder="Enter username">
            </div>
            <div class="input-group">
                <label for="password">Password:</label>
                <input type="password" id="password" placeholder="Enter password">
            </div>
            <button id="login-btn">Login</button>
            <button id="logout-btn" style="display: none;">Logout</button>
            <div id="login-status" class="status">Not logged in</div>
        </div>
        
        <div id="command-section" class="section" style="display: none;">
            <h2>Execute Command</h2>
            <div class="input-group">
                <label for="command-select">Command:</label>
                <select id="command-select">
                    <option value="">Select a command...</option>
                </select>
            </div>
            <div class="input-group" id="params-group" style="display: none;">
                <label for="command-params">Parameters:</label>
                <input type="text" id="command-params" placeholder="Enter parameters">
            </div>
            <button id="execute-btn">Execute</button>
        </div>
        
        <div id="log-section" class="section">
            <h2>Log Output</h2>
            <div id="log-output" class="log-output"></div>
        </div>
    </div>
    
    <script src="/public/app.js"></script>
</body>
</html>
)";
    indexFile.close();

    std::ofstream cssFile("public/styles.css");
    cssFile << R"(
* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
    line-height: 1.6;
    color: #333;
    background-color: #f5f5f5;
}

.container {
    max-width: 800px;
    margin: 0 auto;
    padding: 20px;
}

h1 {
    text-align: center;
    margin-bottom: 20px;
    color: #2c3e50;
}

h2 {
    margin-bottom: 15px;
    color: #3498db;
}

.section {
    background-color: #fff;
    border-radius: 5px;
    padding: 20px;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    margin-bottom: 20px;
}

.input-group {
    margin-bottom: 15px;
}

label {
    display: block;
    margin-bottom: 5px;
    font-weight: bold;
}

input, select {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-size: 16px;
}

button {
    background-color: #3498db;
    color: white;
    border: none;
    padding: 10px 15px;
    border-radius: 4px;
    cursor: pointer;
    font-size: 16px;
    margin-right: 10px;
}

button:hover {
    background-color: #2980b9;
}

.status {
    margin-top: 10px;
    padding: 10px;
    border-radius: 4px;
    background-color: #f8f9fa;
    border-left: 4px solid #6c757d;
}

.status.success {
    border-left-color: #28a745;
    background-color: #e8f5e9;
}

.status.error {
    border-left-color: #dc3545;
    background-color: #ffebee;
}

.log-output {
    height: 300px;
    overflow-y: auto;
    background-color: #f8f9fa;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-family: monospace;
    white-space: pre-wrap;
    word-break: break-all;
}

/* Make the application responsive */
@media (max-width: 600px) {
    .container {
        padding: 10px;
    }
    
    .section {
        padding: 15px;
    }
    
    button {
        width: 100%;
        margin-bottom: 10px;
        margin-right: 0;
    }
}
)";
    cssFile.close();

    std::ofstream jsFile("public/app.js");
    jsFile << R"(
document.addEventListener('DOMContentLoaded', function() {
    // Elements
    const loginSection = document.getElementById('login-section');
    const commandSection = document.getElementById('command-section');
    const logOutput = document.getElementById('log-output');
    const loginStatus = document.getElementById('login-status');
    const usernameInput = document.getElementById('username');
    const passwordInput = document.getElementById('password');
    const loginBtn = document.getElementById('login-btn');
    const logoutBtn = document.getElementById('logout-btn');
    const commandSelect = document.getElementById('command-select');
    const commandParams = document.getElementById('command-params');
    const paramsGroup = document.getElementById('params-group');
    const executeBtn = document.getElementById('execute-btn');
    
    // Check login status on page load
    checkLoginStatus();
    
    // Set up event listeners
    loginBtn.addEventListener('click', login);
    logoutBtn.addEventListener('click', logout);
    executeBtn.addEventListener('click', executeCommand);
    commandSelect.addEventListener('change', handleCommandChange);
    
    // Fetch logs periodically
    setInterval(fetchLogs, 2000);
    
    async function login() {
        const username = usernameInput.value.trim();
        const password = passwordInput.value.trim();
        
        if (!username || !password) {
            updateLoginStatus('Please enter both username and password', 'error');
            return;
        }
        
        try {
            const response = await fetch('/api/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ username, password })
            });
            
            const data = await response.json();
            
            if (response.ok) {
                updateLoginStatus(`Logged in as ${data.username}`, 'success');
                loginBtn.style.display = 'none';
                logoutBtn.style.display = 'inline-block';
                commandSection.style.display = 'block';
                passwordInput.value = '';
                fetchCommands();
                fetchLogs();
            } else {
                updateLoginStatus(data.message || 'Login failed', 'error');
            }
        } catch (error) {
            updateLoginStatus('Error connecting to server', 'error');
            console.error('Login error:', error);
        }
    }
    
    async function logout() {
        try {
            await fetch('/api/logout', {
                method: 'POST'
            });
            
            updateLoginStatus('Logged out', '');
            loginBtn.style.display = 'inline-block';
            logoutBtn.style.display = 'none';
            commandSection.style.display = 'none';
            
            // Clear command select
            while (commandSelect.options.length > 1) {
                commandSelect.remove(1);
            }
            
            // Clear log
            logOutput.innerHTML = '';
        } catch (error) {
            console.error('Logout error:', error);
        }
    }
    
    async function checkLoginStatus() {
        try {
            const response = await fetch('/api/status');
            const data = await response.json();
            
            if (data.authenticated) {
                updateLoginStatus(`Logged in as ${data.username}`, 'success');
                loginBtn.style.display = 'none';
                logoutBtn.style.display = 'inline-block';
                commandSection.style.display = 'block';
                usernameInput.value = data.username;
                fetchCommands();
                fetchLogs();
            } else {
                updateLoginStatus('Not logged in', '');
            }
        } catch (error) {
            updateLoginStatus('Error connecting to server', 'error');
            console.error('Status check error:', error);
        }
    }
    
    async function fetchCommands() {
        try {
            const response = await fetch('/api/commands');
            
            if (!response.ok) {
                return;
            }
            
            const data = await response.json();
            
            // Clear existing options except the first one
            while (commandSelect.options.length > 1) {
                commandSelect.remove(1);
            }
            
            // Add commands to dropdown
            data.commands.forEach(cmd => {
                const option = document.createElement('option');
                option.value = cmd.name;
                option.textContent = `${cmd.name} - ${cmd.description}`;
                option.dataset.params = cmd.params;
                commandSelect.appendChild(option);
            });
        } catch (error) {
            console.error('Fetch commands error:', error);
        }
    }
    
    function handleCommandChange() {
        const selected = commandSelect.options[commandSelect.selectedIndex];
        
        if (selected && selected.dataset.params) {
            paramsGroup.style.display = 'block';
            commandParams.placeholder = `Enter ${selected.dataset.params}`;
        } else {
            paramsGroup.style.display = 'none';
            commandParams.value = '';
        }
    }
    
    async function executeCommand() {
        const command = commandSelect.value;
        const params = commandParams.value;
        
        if (!command) {
            alert('Please select a command');
            return;
        }
        
        try {
            const response = await fetch('/api/execute', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ command, params })
            });
            
            if (!response.ok) {
                const data = await response.json();
                alert(data.message || 'Command execution failed');
                return;
            }
            
            // Command executed successfully, refresh logs
            fetchLogs();
        } catch (error) {
            console.error('Execute command error:', error);
            alert('Error executing command');
        }
    }
    
    async function fetchLogs() {
        try {
            const response = await fetch('/api/logs');
            
            if (!response.ok) {
                return;
            }
            
            const data = await response.json();
            
            // Update log output
            logOutput.innerHTML = data.logs.join('<br>');
            
            // Scroll to bottom
            logOutput.scrollTop = logOutput.scrollHeight;
        } catch (error) {
            console.error('Fetch logs error:', error);
        }
    }
    
    function updateLoginStatus(message, type) {
        loginStatus.textContent = message;
        loginStatus.className = 'status';
        
        if (type) {
            loginStatus.classList.add(type);
        }
    }
});
)";
    jsFile.close();

    // Create directory for static files
    system("mkdir -p public");

    // Start the server
    std::cout << "Starting web server on http://localhost:8080" << std::endl;
    app.port(8080).multithreaded().run();

    return 0;
}
