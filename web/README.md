# Multi-Channel Diagnostic Test Rig - Web Interface

This directory contains the complete web-based user interface for controlling and monitoring the Pico W diagnostic test rig.

## Quick Start

1. **Start the web server:**
   ```bash
   ./start_web_server.sh
   ```

2. **Open your browser to:**
   ```
   http://localhost:8000/static/
   ```

3. **Connect to your Pico W:**
   - Enter your Pico W's IP address in the connection field
   - Click "Connect"
   - The interface will show real-time data from your hardware

## Directory Structure

```
web/
├── static/
│   └── index.html          # Main diagnostic interface
├── assets/
│   ├── css/
│   │   ├── main.css        # Main styles
│   │   └── diagnostic.css  # Diagnostic-specific styles
│   └── js/
│       ├── websocket-client.js      # WebSocket communication
│       ├── diagnostic-interface.js  # UI controller
│       └── main.js                  # Application logic
├── api/
│   └── test.json           # Test API endpoint
└── start_web_server.sh     # Server startup script
```

## Features

### Real-time Monitoring
- 4-channel voltage/current display
- System temperature and memory usage
- Connection status indicators
- Live logging with multiple levels

### Interactive Controls
- Individual channel enable/disable
- Bulk channel operations
- System diagnostics
- HAL demonstrations
- Emergency stop functionality

### Communication
- WebSocket connection to Pico W
- JSON message protocol
- Automatic reconnection
- Command acknowledgment

## Integration with Pico W

The web interface communicates with your Pico W via WebSocket on port 8080. The expected message format:

### Commands (Web → Pico)
```json
{
    "type": "command",
    "command": "TOGGLE_CHANNEL",
    "params": {"channel": 1},
    "timestamp": 1234567890
}
```

### Status Updates (Pico → Web)
```json
{
    "type": "status",
    "channels": [true, false, true, false],
    "system": {
        "temperature": 25.5,
        "loopCount": 150000,
        "freeMemory": 62,
        "uptime": 3600
    }
}
```

### Data Updates (Pico → Web)
```json
{
    "type": "channel_data",
    "channel": 1,
    "voltage": 12.34,
    "current": 0.567
}
```

## Customization

### Adding New Commands
1. Add the command handler in `websocket-client.js`
2. Add UI elements in `index.html`
3. Wire up event handlers in `main.js`

### Styling Changes
- Edit `assets/css/main.css` for layout changes
- Edit `assets/css/diagnostic.css` for diagnostic-specific styling

### Adding New Metrics
1. Update the system metrics in `diagnostic-interface.js`
2. Add display elements in `index.html`
3. Handle incoming data in the WebSocket client

## Troubleshooting

### Connection Issues
- Verify Pico W is on the same network
- Check firewall settings
- Ensure WebSocket server is running on Pico W port 8080

### UI Not Loading
- Check browser console for JavaScript errors
- Verify all CSS/JS files are loading correctly
- Try refreshing or clearing browser cache

### Data Not Updating
- Check WebSocket connection status
- Verify message format from Pico W
- Check browser developer tools Network tab

## Development

For development, you can test the interface without a Pico W by enabling simulation mode in the browser console:

```javascript
// Enable simulation mode
appState.simulationMode = true;
```

This will generate fake data for testing the UI components.
