// Diagnostic Interface Controller
class DiagnosticInterface {
    constructor() {
        this.channelStates = [false, false, false, false];
        this.channelData = [
            { voltage: 0, current: 0, status: 'OFF' },
            { voltage: 0, current: 0, status: 'OFF' },
            { voltage: 0, current: 0, status: 'OFF' },
            { voltage: 0, current: 0, status: 'OFF' }
        ];
        this.systemMetrics = {
            temperature: 25.0,
            loopCount: 0,
            freeMemory: 64,
            uptime: 0
        };
        this.isConnected = false;
    }

    initialize() {
        this.setupEventHandlers();
        this.startPeriodicUpdates();
        addLog('info', 'Interface', 'Diagnostic interface initialized');
    }

    setupEventHandlers() {
        // Connection status updates
        if (picoClient) {
            picoClient.on('onConnect', () => {
                this.isConnected = true;
                this.updateConnectionStatus();
                addLog('info', 'Interface', 'Connected to Pico W');
            });

            picoClient.on('onDisconnect', () => {
                this.isConnected = false;
                this.updateConnectionStatus();
                addLog('warn', 'Interface', 'Disconnected from Pico W');
            });
        }
    }

    updateConnectionStatus() {
        const indicator = document.getElementById('connectionIndicator');
        const status = document.getElementById('connectionStatus');
        const systemStatus = document.getElementById('systemStatus');
        const systemStatusText = document.getElementById('systemStatusText');
        
        if (this.isConnected) {
            indicator.classList.add('connected');
            status.textContent = 'Connected';
            systemStatus.classList.add('online');
            systemStatusText.textContent = 'System Online';
        } else {
            indicator.classList.remove('connected');
            status.textContent = 'Disconnected';
            systemStatus.classList.remove('online');
            systemStatusText.textContent = 'System Offline';
        }
    }

    updateChannelStates(states) {
        if (Array.isArray(states)) {
            this.channelStates = states;
            for (let i = 0; i < 4; i++) {
                this.updateChannelDisplay(i + 1);
            }
        }
    }

    updateChannelData(channel, data) {
        const index = channel - 1;
        if (index >= 0 && index < 4) {
            this.channelData[index] = { ...this.channelData[index], ...data };
            this.updateChannelMetrics(channel);
        }
    }

    updateChannelDisplay(channelNum) {
        const index = channelNum - 1;
        const channel = document.getElementById(`channel${channelNum}`);
        const toggle = channel.querySelector('.channel-toggle');
        const status = document.getElementById(`ch${channelNum}-status`);
        
        if (this.channelStates[index]) {
            channel.classList.add('active');
            toggle.classList.add('active');
            status.textContent = 'ON';
            status.classList.add('on');
            status.classList.remove('off');
        } else {
            channel.classList.remove('active');
            toggle.classList.remove('active');
            status.textContent = 'OFF';
            status.classList.add('off');
            status.classList.remove('on');
        }
    }

    updateChannelMetrics(channelNum) {
        const index = channelNum - 1;
        const data = this.channelData[index];
        
        document.getElementById(`ch${channelNum}-voltage`).textContent = `${data.voltage.toFixed(2)}V`;
        document.getElementById(`ch${channelNum}-current`).textContent = `${data.current.toFixed(3)}A`;
        
        // Update channel warning/error states based on values
        const channel = document.getElementById(`channel${channelNum}`);
        channel.classList.remove('warning', 'error');
        
        if (data.voltage > 14.5) {
            channel.classList.add('warning');
        }
        if (data.voltage > 15.5) {
            channel.classList.add('error');
        }
        if (data.current > 2.5) {
            channel.classList.add('warning');
        }
        if (data.current > 3.0) {
            channel.classList.add('error');
        }
    }

    updateSystemMetrics(metrics) {
        this.systemMetrics = { ...this.systemMetrics, ...metrics };
        
        if (metrics.temperature !== undefined) {
            document.getElementById('systemTemp').textContent = `${metrics.temperature.toFixed(1)}°C`;
        }
        if (metrics.loopCount !== undefined) {
            document.getElementById('loopCount').textContent = metrics.loopCount.toLocaleString();
        }
        if (metrics.freeMemory !== undefined) {
            document.getElementById('freeMemory').textContent = `${metrics.freeMemory}KB`;
        }
        if (metrics.uptime !== undefined) {
            const hours = Math.floor(metrics.uptime / 3600);
            const minutes = Math.floor((metrics.uptime % 3600) / 60);
            const seconds = metrics.uptime % 60;
            document.getElementById('deviceUptime').textContent = 
                `Uptime: ${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
        }
    }

    updateSystemInfo(info) {
        if (info.ip) {
            document.getElementById('deviceIP').textContent = `IP: ${info.ip}`;
        }
        if (info.buildDate) {
            document.getElementById('buildDate').textContent = info.buildDate;
        }
    }

    startPeriodicUpdates() {
        // Request status updates every 2 seconds if connected
        setInterval(() => {
            if (this.isConnected && picoClient) {
                picoClient.getStatus();
            }
        }, 2000);
    }
}

// Global diagnostic interface instance
let diagnosticInterface = null;
