// Global application state
let appState = {
    connected: false,
    picoIP: '192.168.1.100', // Default IP
    logLevel: 'info'
};

// Initialize application when DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
    initializeApplication();
});

function initializeApplication() {
    addLog('info', 'App', 'Initializing Multi-Channel Diagnostic Test Rig Interface');
    
    // Initialize WebSocket client
    picoClient = new PicoWebSocketClient();
    
    // Initialize diagnostic interface
    diagnosticInterface = new DiagnosticInterface();
    diagnosticInterface.initialize();
    
    // Load saved settings
    loadSettings();
    
    // Setup UI event handlers
    setupUIEventHandlers();
    
    addLog('info', 'App', 'Application initialized successfully');
}

function setupUIEventHandlers() {
    // Log level selector
    const logLevelSelect = document.getElementById('logLevel');
    if (logLevelSelect) {
        logLevelSelect.addEventListener('change', function() {
            appState.logLevel = this.value;
            saveSettings();
        });
    }
    
    // IP address input
    const picoIPInput = document.getElementById('picoIP');
    if (picoIPInput) {
        picoIPInput.addEventListener('change', function() {
            appState.picoIP = this.value;
            saveSettings();
        });
    }
}

// Connection Management
function connectToPico() {
    const ipInput = document.getElementById('picoIP');
    const ip = ipInput.value.trim();
    
    if (!ip) {
        addLog('error', 'Connection', 'Please enter a valid IP address');
        return;
    }
    
    if (!isValidIP(ip)) {
        addLog('error', 'Connection', 'Invalid IP address format');
        return;
    }
    
    appState.picoIP = ip;
    saveSettings();
    
    addLog('info', 'Connection', `Attempting to connect to ${ip}...`);
    picoClient.connect(ip);
}

function disconnectFromPico() {
    if (picoClient) {
        picoClient.disconnect();
        addLog('info', 'Connection', 'Disconnected from Pico W');
    }
}

function isValidIP(ip) {
    const regex = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
    return regex.test(ip);
}

// Channel Control Functions
function toggleChannel(channelNum) {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'Channel', 'Not connected to Pico W');
        return;
    }
    
    picoClient.toggleChannel(channelNum);
    addLog('info', 'Channel', `Toggled channel ${channelNum}`);
}

function enableAllChannels() {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'Channel', 'Not connected to Pico W');
        return;
    }
    
    picoClient.enableAllChannels();
    addLog('info', 'Channel', 'Enabled all channels');
}

function disableAllChannels() {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'Channel', 'Not connected to Pico W');
        return;
    }
    
    picoClient.disableAllChannels();
    addLog('info', 'Channel', 'Disabled all channels');
}

function toggleAllChannels() {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'Channel', 'Not connected to Pico W');
        return;
    }
    
    picoClient.sendCommand('TOGGLE_ALL_CHANNELS');
    addLog('info', 'Channel', 'Toggled all channels');
}

// System Control Functions
function runDiagnostics() {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'System', 'Not connected to Pico W');
        return;
    }
    
    picoClient.runDiagnostics();
    addLog('info', 'System', 'Running system diagnostics...');
}

function runHalDemo() {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'System', 'Not connected to Pico W');
        return;
    }
    
    picoClient.runHalDemo();
    addLog('info', 'System', 'Running HAL demonstration...');
}

function runHalTest() {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'System', 'Not connected to Pico W');
        return;
    }
    
    picoClient.runHalTest();
    addLog('info', 'System', 'Running HAL tests...');
}

function testSafety() {
    if (!picoClient || !picoClient.isConnected) {
        addLog('warn', 'System', 'Not connected to Pico W');
        return;
    }
    
    if (confirm('Run safety system test? This will test emergency shutdown procedures.')) {
        picoClient.sendCommand('TEST_SAFETY');
        addLog('warn', 'Safety', 'Running safety system test...');
    }
}

function emergencyStop() {
    if (confirm('EMERGENCY STOP: Are you sure? This will immediately disable all channels and halt operations.')) {
        if (picoClient && picoClient.isConnected) {
            picoClient.emergencyStop();
        }
        
        // Also update UI immediately
        if (diagnosticInterface) {
            diagnosticInterface.updateChannelStates([false, false, false, false]);
        }
        
        addLog('error', 'Emergency', 'EMERGENCY STOP ACTIVATED');
    }
}

// Logging Functions
function addLog(level, source, message) {
    // Check log level filtering
    const levels = ['error', 'warn', 'info', 'debug'];
    const currentLevelIndex = levels.indexOf(appState.logLevel);
    const messageLevelIndex = levels.indexOf(level);
    
    if (messageLevelIndex > currentLevelIndex) {
        return; // Skip this message due to log level filtering
    }
    
    const container = document.getElementById('logContainer');
    if (!container) return;
    
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    
    const timestamp = new Date().toLocaleTimeString();
    
    entry.innerHTML = `
        <span class="log-timestamp">[${timestamp}]</span>
        <span class="log-level ${level}">[${level.toUpperCase()}]</span>
        <span class="log-message">${source}: ${message}</span>
    `;
    
    container.appendChild(entry);
    container.scrollTop = container.scrollHeight;
    
    // Keep only last 200 log entries for performance
    while (container.children.length > 200) {
        container.removeChild(container.firstChild);
    }
}

function clearLogs() {
    const container = document.getElementById('logContainer');
    if (container) {
        container.innerHTML = '';
        addLog('info', 'Log', 'Log cleared');
    }
}

// Helper functions for diagnostic interface
function updateChannelStates(states) {
    if (diagnosticInterface) {
        diagnosticInterface.updateChannelStates(states);
    }
}

function updateChannelData(channel, data) {
    if (diagnosticInterface) {
        diagnosticInterface.updateChannelData(channel, data);
    }
}

function updateSystemMetrics(metrics) {
    if (diagnosticInterface) {
        diagnosticInterface.updateSystemMetrics(metrics);
    }
}

function updateSystemInfo(info) {
    if (diagnosticInterface) {
        diagnosticInterface.updateSystemInfo(info);
    }
}

// Settings Management
function saveSettings() {
    const settings = {
        picoIP: appState.picoIP,
        logLevel: appState.logLevel
    };
    
    try {
        localStorage.setItem('diagnosticRigSettings', JSON.stringify(settings));
    } catch (error) {
        // localStorage might not be available, ignore silently
    }
}

function loadSettings() {
    try {
        const saved = localStorage.getItem('diagnosticRigSettings');
        if (saved) {
            const settings = JSON.parse(saved);
            appState.picoIP = settings.picoIP || appState.picoIP;
            appState.logLevel = settings.logLevel || appState.logLevel;
            
            // Update UI elements
            const picoIPInput = document.getElementById('picoIP');
            if (picoIPInput) {
                picoIPInput.value = appState.picoIP;
            }
            
            const logLevelSelect = document.getElementById('logLevel');
            if (logLevelSelect) {
                logLevelSelect.value = appState.logLevel;
            }
        }
    } catch (error) {
        // localStorage might not be available, use defaults
        addLog('debug', 'Settings', 'Using default settings');
    }
}

// Expose functions globally for onclick handlers
window.connectToPico = connectToPico;
window.disconnectFromPico = disconnectFromPico;
window.toggleChannel = toggleChannel;
window.enableAllChannels = enableAllChannels;
window.disableAllChannels = disableAllChannels;
window.toggleAllChannels = toggleAllChannels;
window.runDiagnostics = runDiagnostics;
window.runHalDemo = runHalDemo;
window.runHalTest = runHalTest;
window.testSafety = testSafety;
window.emergencyStop = emergencyStop;
window.clearLogs = clearLogs;
window.addLog = addLog;
