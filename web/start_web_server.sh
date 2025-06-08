#!/bin/bash

# Web Server Startup Script for Diagnostic Test Rig

set -e

WEB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$WEB_DIR/.." && pwd)"

echo "🌐 Starting Diagnostic Test Rig Web Server"
echo "=========================================="
echo "Web Directory: $WEB_DIR"
echo "Project Root: $PROJECT_ROOT"
echo ""

# Check if Python is available
if command -v python3 >/dev/null 2>&1; then
    PYTHON_CMD="python3"
elif command -v python >/dev/null 2>&1; then
    PYTHON_CMD="python"
else
    echo "❌ Error: Python not found"
    echo "Please install Python 3 to run the web server"
    exit 1
fi

echo "🐍 Using Python: $PYTHON_CMD"

# Change to web directory
cd "$WEB_DIR"

# Check for port availability
PORT=8000
if command -v lsof >/dev/null 2>&1; then
    if lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; then
        echo "⚠️  Port $PORT is in use, trying port $((PORT + 1))"
        PORT=$((PORT + 1))
    fi
fi

echo "🚀 Starting web server on port $PORT..."
echo ""
echo "📱 Access the diagnostic interface at:"
echo "   http://localhost:$PORT/static/"
echo ""
echo "🔧 To connect to your Pico W:"
echo "   1. Make sure your Pico W is connected to the same network"
echo "   2. Find your Pico W's IP address"
echo "   3. Enter the IP in the connection field in the web interface"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

# Start the Python HTTP server
$PYTHON_CMD -m http.server $PORT

echo ""
echo "Web server stopped."
