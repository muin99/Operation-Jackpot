# Testing Multiplayer - Guide

This guide explains how to test the multiplayer functionality with multiple players.

## Option 1: Testing on the Same PC (Quick Test)

You can run multiple instances of the game on the same PC to test locally:

### Method A: Using Command-Line Arguments (Easiest)

1. **Start Server Instance:**
   ```powershell
   python run.py --server
   ```
   Or directly:
   ```powershell
   bin\Debug\projectOj.exe --server
   ```
   - The server will auto-start and display its IP address
   - Note the server IP (usually `127.0.0.1:7777` for localhost)

2. **Start Client Instance(s):**
   - Open a **new terminal/command prompt** (keep server running)
   - Navigate to the project directory
   - Run:
   ```powershell
   python run.py --client 127.0.0.1
   ```
   Or directly:
   ```powershell
   bin\Debug\projectOj.exe --client 127.0.0.1
   ```
   - The client will automatically connect to the server
   - You should see "Connecting to server..." message

### Method B: Using Menu (Manual)

1. **Start Server Instance:**
   - Run the game (first instance)
   - Press `S` to start server mode
   - Note the server IP displayed (e.g., `192.168.0.100:7777` or `127.0.0.1:7777`)
   - The server is now waiting for clients

2. **Start Client Instance(s):**
   - Run the game again (second instance - open another terminal/command prompt)
   - Navigate to the project directory
   - Run: `bin\Debug\projectOj.exe` (or use `python run.py` again)
   - Press `L` to connect as client
   - When prompted for IP, the default is `127.0.0.1` (localhost)
   - You should see "Connecting to server..." message

3. **Create and Join Rooms:**
   - **Server**: Press `C` to create a room
   - **Client**: Press `J` to view available rooms
   - **Client**: Press `1-9` to join a room (or press number corresponding to room)
   - **Server**: Press `S` to start the match when ready

4. **Play:**
   - Use `WASD` to move
   - Mouse to aim
   - Click to shoot
   - You should see other players moving in real-time

## Option 2: Testing Across Multiple PCs on Local Network

For a more realistic test with multiple physical PCs:

### Prerequisites:

1. **All PCs must be on the same local network (same router)**
2. **Windows Firewall**: Allow port 7777 UDP on server PC
   - Open Windows Defender Firewall → Advanced Settings
   - Inbound Rules → New Rule → Port → UDP → 7777
   - Allow connection → Apply to all profiles

### Steps:

1. **On Server PC:**
   - Run the game with:
   ```powershell
   python run.py --server
   ```
   Or press `S` in the menu
   - **Note the IP address displayed** (e.g., `192.168.0.100:7777`)
   - Share this IP with other players

2. **On Client PC(s):**
   - Run the game with the server's IP:
   ```powershell
   python run.py --client 192.168.0.100
   ```
   Replace `192.168.0.100` with the actual server IP address
   - The client will automatically connect to the server
   - You should see "Connecting to server..." message

3. **Alternative: Quick IP Change Method**
   - You can temporarily hardcode the server IP in `Game.cpp` for testing
   - Or use command-line arguments (if implemented)

4. **Create Rooms and Play:**
   - Same as Option 1, but now across different PCs

## Command-Line Arguments

The game supports command-line arguments for easy testing:

### Available Options:

- `--server` or `-s`: Auto-start as server
- `--client [IP]` or `-c [IP]`: Auto-connect as client to specified IP (default: 127.0.0.1)
- `--help` or `-h`: Show help message

### Examples:

```powershell
# Start server
python run.py --server
bin\Debug\projectOj.exe --server

# Connect as client to localhost
python run.py --client
python run.py --client 127.0.0.1

# Connect as client to network server
python run.py --client 192.168.0.100

# Show help
python run.py --help
```

## Troubleshooting

### "Failed to connect to server"
- **Check firewall**: Ensure port 7777 UDP is allowed on server PC
- **Check IP address**: Verify you're using the correct server IP
- **Check network**: Ensure both PCs are on the same network
- **Check server**: Make sure server is running and shows "Server started"

### "Server IP: 127.0.0.1" when it should show local network IP
- This is normal if the PC doesn't have a network adapter with a local IP
- Try connecting from clients using `127.0.0.1` if testing on same PC
- For network testing, ensure the server PC has a valid local IP (192.168.x.x or 10.x.x.x)

### Players not seeing each other
- Ensure all players are in the same room
- Check that match has started (server pressed `S`)
- Verify network connection status in console output

### Room list not showing
- Press `R` to refresh the room list
- Ensure server has created at least one room
- Check that rooms are in `WAITING` status (not `IN_MATCH`)

## Testing Checklist

- [ ] Server starts successfully
- [ ] Client connects to server
- [ ] Room list displays correctly
- [ ] Client can join room
- [ ] Multiple clients can join same room
- [ ] Match starts when server presses `S`
- [ ] Players see each other in game
- [ ] Player movement syncs across clients
- [ ] Bullets spawn and sync correctly
- [ ] Disconnection handled gracefully

## Quick Test Scripts

For rapid testing on same PC, you can create batch files:

**test_server.bat:**
```batch
@echo off
cd /d "%~dp0"
start "Server" bin\Debug\projectOj.exe --server
```

**test_client.bat:**
```batch
@echo off
cd /d "%~dp0"
start "Client" bin\Debug\projectOj.exe --client 127.0.0.1
```

**test_client_network.bat** (for network testing):
```batch
@echo off
cd /d "%~dp0"
set /p SERVER_IP="Enter server IP: "
start "Client" bin\Debug\projectOj.exe --client %SERVER_IP%
```

Run both batch files to start server and client instances quickly.

