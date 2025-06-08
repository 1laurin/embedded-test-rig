class PicoWebSocketClient {
    constructor() {
        this.ws = null;
        this.isConnected = false;
        this.picoIP = null;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectDelay = 1000; // Start with 1 second
        this.callbacks = {
            onConnect: [],
            onDisconnect: [],
            onMessage: [],
            onError: []
        };
    }

    connect(ip) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.disconnect();
        }

        this.picoIP = ip;
        const wsUrl = `ws://${ip}:8080/ws`;
        
        try {
            this.ws = new WebSocket(wsUrl);
            this.setupEventHandlers();
            addLog('info', 'WebSocket', `Connecting to ${wsUrl}...`);
        } catch (error) {
            addLog('error', 'WebSocket', `Connection failed: ${error.message}`);
            this.triggerCallback('onError', error);
        }
    }

    disconnect() {
        if (this.ws) {
            this.ws.close();
            this.ws = null;
        }
        this.isConnected = false;
        this.triggerCallback('onDisconnect');
    }

    setupEventHandlers() {
        this.ws.onopen = () => {
            this.isConnected = true;
            this.reconnectAttempts = 0;
            this.reconnectDelay = 1000;
            addLog('info', 'WebSocket', `Connected to Pico W at ${this.picoIP}`);
            this.triggerCallback('onConnect');
            
            // Request initial status
            this.sendCommand('GET_STATUS');
        };

        this.ws.onclose = (event) => {
            this.isConnected = false;
            addLog('warn', 'WebSocket', `Connection closed (code: ${event.code})`);
            this.triggerCallback('onDisconnect');
            
            // Auto-reconnect logic
            if (this.reconnectAttempts < this.maxReconnectAttempts) {
                setTimeout(() => {
                    this.reconnectAttempts++;
                    addLog('info', 'WebSocket', `Reconnection attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts}`);
                    this.connect(this.picoIP);
                }, this.reconnectDelay);
                
                this.reconnectDelay = Math.min(this.reconnectDelay * 2, 30000); // Max 30 seconds
            }
        };

        this.ws.onerror = (error) => {
            addLog('error', 'WebSocket', 'Connection error occurred');
            this.triggerCallback('onError', error);
        };

        this.ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                this.handleMessage(data);
            } catch (error) {
                // Handle plain text messages
                addLog('debug', 'RX', event.data);
                this.triggerCallback('onMessage', { type: 'text', data: event.data });
            }
        };
    }

    handleMessage(data) {
        addLog('debug', 'RX', JSON.stringify(data));
        this.triggerCallback('onMessage', data);
        
        // Handle specific message types
        switch (data.type) {
            case 'status':
                this.handleStatusUpdate(data);
                break;
            case 'channel_data':
                this.handleChannelData(data);
                break;
            case 'system_info':
                this.handleSystemInfo(data);
                break;
            case 'log':
                addLog(data.level || 'info', data.source || 'Pico', data.message);
                break;
            case 'error':
                addLog('error', 'Pico', data.message);
                break;
        }
    }

    handleStatusUpdate(data) {
        if (data.channels) {
            updateChannelStates(data.channels);
        }
        if (data.system) {
            updateSystemMetrics(data.system);
        }
    }

    handleChannelData(data) {
        updateChannelData(data.channel, data);
    }

    handleSystemInfo(data) {
        updateSystemInfo(data);
    }

    sendCommand(command, params = {}) {
        if (!this.isConnected || !this.ws) {
            addLog('warn', 'WebSocket', 'Not connected - command ignored');
            return false;
        }

        const message = {
            type: 'command',
            command: command,
            params: params,
            timestamp: Date.now()
        };

        try {
            this.ws.send(JSON.stringify(message));
            addLog('debug', 'TX', `${command} ${JSON.stringify(params)}`);
            return true;
        } catch (error) {
            addLog('error', 'WebSocket', `Send failed: ${error.message}`);
            return false;
        }
    }

    // Event system
    on(event, callback) {
        if (this.callbacks[event]) {
            this.callbacks[event].push(callback);
        }
    }

    off(event, callback) {
        if (this.callbacks[event]) {
            const index = this.callbacks[event].indexOf(callback);
            if (index > -1) {
                this.callbacks[event].splice(index, 1);
            }
        }
    }

    triggerCallback(event, data = null) {
        if (this.callbacks[event]) {
            this.callbacks[event].forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error(`Callback error for ${event}:`, error);
                }
            });
        }
    }

    // Convenience methods for common commands
    toggleChannel(channel) {
        return this.sendCommand('TOGGLE_CHANNEL', { channel: channel });
    }

    enableChannel(channel) {
        return this.sendCommand('ENABLE_CHANNEL', { channel: channel });
    }

    disableChannel(channel) {
        return this.sendCommand('DISABLE_CHANNEL', { channel: channel });
    }

    enableAllChannels() {
        return this.sendCommand('ENABLE_ALL_CHANNELS');
    }

    disableAllChannels() {
        return this.sendCommand('DISABLE_ALL_CHANNELS');
    }

    runDiagnostics() {
        return this.sendCommand('RUN_DIAGNOSTICS');
    }

    runHalDemo() {
        return this.sendCommand('RUN_HAL_DEMO');
    }

    runHalTest() {
        return this.sendCommand('RUN_HAL_TEST');
    }

    emergencyStop() {
        return this.sendCommand('EMERGENCY_STOP');
    }

    getStatus() {
        return this.sendCommand('GET_STATUS');
    }
}

// Global WebSocket client instance
let picoClient = null;
