#!/bin/bash
# Test results color output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}FastestWebsiteEver Server Test${NC}"

# Function to cleanup processes and ports
cleanup() {
    echo "Cleaning up processes on port 80..."
    lsof -ti :80 | xargs -r kill -9 2>/dev/null
    sleep 1
    
    if lsof -i :80 >/dev/null 2>&1; then
        echo "Warning: Port 80 is still in use"
        lsof -i :80
        echo "Please manually kill the process and try again"
        exit 1
    fi
}

trap cleanup EXIT

cleanup



# Check if server is already running
if pgrep cpkthttp > /dev/null; then
  echo -e "${RED}Warning: cpkthttp server is already running.${NC}"
  echo "Please terminate the running server and try again."
  exit 1
fi

# Start the server in the background
echo -e "${YELLOW}1. Starting server...${NC}"
cd server
./cpkthttp 80 > server_log.txt 2>&1 &
SERVER_PID=$!

# Wait a bit for the server to start
sleep 2

# Check if the server started successfully
if ! ps -p $SERVER_PID > /dev/null; then
  echo -e "${RED}Server startup failed${NC}"
  cat server_log.txt
  exit 1
else
  echo -e "${GREEN}Server started successfully (PID: $SERVER_PID)${NC}"
  echo "server: waiting for connections on port 80..."
fi

# Send request from client
echo -e "${YELLOW}2. Sending request from client...${NC}"
OUTPUT_FILE="test_output.bin"
curl_output=$(curl -s -w "%{http_code}" http://localhost:80 -o $OUTPUT_FILE)
HTTP_CODE=${curl_output: -3}

# Check HTTP response code
if [ "$HTTP_CODE" != "200" ]; then
  echo -e "${RED}Request failed: HTTP code $HTTP_CODE${NC}"
  kill $SERVER_PID
  exit 1
fi

# Check response size
RESPONSE_SIZE=$(stat -c%s "$OUTPUT_FILE")
if [ $RESPONSE_SIZE -lt 100 ]; then
  echo -e "${RED}Abnormal response size: $RESPONSE_SIZE bytes${NC}"
  kill $SERVER_PID
  exit 1
else
  echo -e "${GREEN}Request successful: received $RESPONSE_SIZE bytes${NC}"
fi

# Check file type
FILE_TYPE=$(file -b $OUTPUT_FILE)
echo "Received file type: $FILE_TYPE"

# Test multiple requests
echo -e "${YELLOW}3. Testing multiple requests...${NC}"
for i in {1..5}; do
  code=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:80)
  if [ "$code" != "200" ]; then
    echo -e "${RED}Request $i failed: HTTP $code${NC}"
    kill $SERVER_PID 2>/dev/null
    exit 1
  fi
  echo -e "Request $i: ${GREEN}$code${NC}"
done

# Terminate the server
echo -e "${YELLOW}4. Terminating server...${NC}"
kill $SERVER_PID
sleep 2  

# Check if the server terminated
if ps -p $SERVER_PID > /dev/null; then
  echo -e "${RED}Server termination failed${NC}"
  kill -9 $SERVER_PID
  echo "Force terminated"
else
  echo -e "${GREEN}Server terminated normally${NC}"
fi

# Display test results summary
echo ""
echo -e "${GREEN}Test completed: FastestWebsiteEver is functioning properly${NC}"
echo "- Server startup: Success"
echo "- Client request: Success ($RESPONSE_SIZE bytes)"
echo "- Multiple requests: Success"
echo "- Server termination: Success"

# Clean up temporary files
rm -f $OUTPUT_FILE server_log.txt
