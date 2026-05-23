#ifndef INTERFACE_H
#define INTERFACE_H

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>CSIR-CMERI Robot Control Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; user-select: none; -webkit-user-select: none; }
    .btn { width: 60px; height: 60px; font-size: 2em; margin: 10px; border-radius: 10px; border: none; background: #2196F3; color: white; }
    .btn:active { background: #0b7dda; }
    #f, #b { background: #4CAF50; }
    #f:active, #b:active { background: #45a049; }
    #s { background: #f44336; }
    #s:active { background: #da190b; }
    #fs { background: #1a3a70; width: 80px; }
    #fs:active { background: #0f2550; }
    #bs { background: #000000; width: 80px; }
    #bs:active { background: #333333; }
    #move { background: #4CAF50; width: 150px; height: 80px; font-size: 1.2em; }
    #move:active { background: #45a049; }
    #start_calibrate { background: #808080; width: 120px; height: 80px; font-size: 1em; }
    #start_calibrate:active { background: #606060; }
    #calibrate { background: #FFD700; width: 120px; height: 80px; font-size: 1em; color: black; }
    #calibrate:active { background: #FFC700; }
    .btn-row { display: flex; justify-content: center; align-items: center; gap: 10px; }
    .calibration-row { display: flex; justify-content: center; align-items: center; gap: 10px; }
    .mode-row { display: flex; justify-content: center; align-items: center; gap: 20px; margin: 20px 0; }
    .toggle-btn { width: 140px; height: 60px; font-size: 1em; border: 2px solid #333; background: #e0e0e0; color: black; border-radius: 8px; cursor: pointer; transition: all 0.3s; }
    .toggle-btn.active { background: #4CAF50; color: white; border-color: #45a049; }
    #mapContainer { margin: 20px auto; border: 2px solid #333; border-radius: 8px; background: #f0f0f0; display: inline-block; }
    #mapCanvas { display: block; border: 1px solid #999; background: white; cursor: crosshair; width: 300px; height: 300px; }
    #debug { width: 90vw; height: 120px; border: 1px solid #ccc; margin-top: 20px; overflow-y: auto; background: #f9f9f9; padding: 10px; }
  </style>
</head>
<body>
  <div style="text-align:center;">
    <h1><b>CSIR-CMERI</b></h1>
    <h2>Robot Control Dashboard</h2>
    
    <div class="mode-row">
      <button class="toggle-btn" id="lineFollowBtn" onclick="toggleLineFollowing()">Line Following: OFF</button>
      <button class="toggle-btn" id="obstacleAvoidBtn" onclick="toggleObstacleAvoid()">Obstacle Avoid: OFF</button>
    </div>

    <hr>
    
    <h3>Environment Map (ToF Sensor)</h3>
    <div id="mapContainer">
      <canvas id="mapCanvas" width="300" height="300"></canvas>
    </div>
    <p style="font-size: 0.8em; color: #666;">Green: Robot | Blue: Free Space | Red: Obstacles</p>

    <hr>
    
    <button class="btn" id="f" onmousedown="sendCmd('move','f')" onmouseup="sendCmd('move','s')" ontouchstart="sendCmd('move','f')" ontouchend="sendCmd('move','s')">F</button><br>
    <button class="btn" id="l" onmousedown="sendCmd('move','l')" onmouseup="sendCmd('move','s')" ontouchstart="sendCmd('move','l')" ontouchend="sendCmd('move','s')">L</button>
    <button class="btn" id="s" onmousedown="sendCmd('move','s')" onmouseup="sendCmd('move','s')" ontouchstart="sendCmd('move','s')" ontouchend="sendCmd('move','s')">S</button>
    <button class="btn" id="r" onmousedown="sendCmd('move','r')" onmouseup="sendCmd('move','s')" ontouchstart="sendCmd('move','r')" ontouchend="sendCmd('move','s')">R</button><br>
    <button class="btn" id="b" onmousedown="sendCmd('move','b')" onmouseup="sendCmd('move','s')" ontouchstart="sendCmd('move','b')" ontouchend="sendCmd('move','s')">B</button>

    <hr>
    <input type="number" id="time" placeholder="time" value="500"><br>
    <div class="btn-row">
      <button class="btn" id="fs" onclick="sendCmd('fs',document.getElementById('time').value)">FS</button>
      <button class="btn" id="bs" onclick="sendCmd('bs',document.getElementById('time').value)">BS</button>
    </div>

    <hr>
    <input type="number" id="angle" placeholder="time" value="90"><br>
    <button class="btn" id="turn" onclick="sendCmd('turn',document.getElementById('angle').value)">turn </button><br>

    <hr>
    <input type="number" id="cal_distance" placeholder="Cal_Distance"><br>
    <div class="calibration-row">
      <button class="btn" id="start_calibrate" onclick="sendCmd('start_calibration','1')">Start Cal</button>
      <button class="btn" id="calibrate" onclick="sendCmd('calibrate',document.getElementById('cal_distance').value)">Calibrate</button>
    </div>

    <hr>
    <input type="number" id="move_distance" placeholder="Distance"><br>
    <button class="btn" id="move" onclick="sendCmd('moveED',document.getElementById('move_distance').value)">Move Exact Distance</button><br>

    <div id="debug"></div>
  </div>
  <script>
    var socket;
    var lineFollowingActive = false;
    var obstacleAvoidingActive = false;
    var mapCanvas, mapCtx;
    var mapData = null;
    
    window.onload = function() {
      // Initialize canvas
      mapCanvas = document.getElementById('mapCanvas');
      mapCtx = mapCanvas.getContext('2d');
      
      socket = new WebSocket('ws://' + window.location.hostname + ":81");
      socket.onopen = function() { console.log('WebSocket Connected'); };
      socket.onmessage = function(event) {
        try {
          var data = JSON.parse(event.data);
          
          // Handle debug messages
          if (data.key === 'debug') {
            document.getElementById('debug').innerHTML += data.msg + '<br>';
            document.getElementById('debug').scrollTop = document.getElementById('debug').scrollHeight;
          } 
          // Handle map data (new format with obstacles, free, robot, etc.)
          else if (data.obstacles !== undefined || data.robot !== undefined) {
            mapData = data;
            console.log('Map data received:', {
              obstacles: (data.obstacles || []).length,
              free: (data.free || []).length,
              robot: data.robot
            });
            renderMap();
          } 
          // Handle map wrapper format
          else if (data.type === 'map' && data.data) {
            mapData = data.data;
            renderMap();
          }
        } catch (e) { 
          console.log('JSON parse error:', e.message, 'Data:', event.data.substring(0, 100)); 
        }
      };
      socket.onclose = function() { console.log('WebSocket Closed'); };
      
      // Initial map render
      renderMap();
    };
    
    function renderMap() {
      if (!mapCanvas || !mapCtx) return;
      
      const w = mapCanvas.width;
      const h = mapCanvas.height;
      
      // Clear canvas
      mapCtx.fillStyle = '#ffffff';
      mapCtx.fillRect(0, 0, w, h);
      
      // Draw grid
      mapCtx.strokeStyle = '#eeeeee';
      mapCtx.lineWidth = 0.5;
      for (let i = 0; i <= 10; i++) {
        const x = (w / 10) * i;
        const y = (h / 10) * i;
        mapCtx.beginPath();
        mapCtx.moveTo(x, 0);
        mapCtx.lineTo(x, h);
        mapCtx.stroke();
        mapCtx.beginPath();
        mapCtx.moveTo(0, y);
        mapCtx.lineTo(w, y);
        mapCtx.stroke();
      }
      
      // Draw corner index labels for reference
      mapCtx.fillStyle = '#cccccc';
      mapCtx.font = '10px Arial';
      mapCtx.textAlign = 'left';
      mapCtx.fillText('0,0', 2, 12);
      mapCtx.textAlign = 'right';
      mapCtx.fillText('100,0', w-2, 12);
      mapCtx.textAlign = 'left';
      mapCtx.fillText('0,100', 2, h-2);
      mapCtx.textAlign = 'right';
      mapCtx.fillText('100,100', w-2, h-2);
      
      if (!mapData || (!mapData.obstacles && !mapData.robot)) {
        // Draw "No data" message
        mapCtx.fillStyle = '#999999';
        mapCtx.font = '16px Arial';
        mapCtx.textAlign = 'center';
        mapCtx.fillText('Waiting for sensor data...', w/2, h/2);
        return;
      }
      
      const mapSize = mapData.size || 100;
      const cellWidth = w / mapSize;
      const cellHeight = h / mapSize;
      
      // Draw free space cells first (so obstacles overlay them)
      if (mapData.free && mapData.free.length > 0) {
        mapCtx.fillStyle = '#aaccff';
        mapData.free.forEach(cell => {
          if (cell.x >= 0 && cell.x < mapSize && cell.y >= 0 && cell.y < mapSize) {
            mapCtx.fillRect(cell.x * cellWidth, cell.y * cellHeight, cellWidth, cellHeight);
          }
        });
      }
      
      // Draw obstacles (red)
      if (mapData.obstacles && mapData.obstacles.length > 0) {
        mapCtx.fillStyle = '#ff4444';
        mapData.obstacles.forEach(cell => {
          if (cell.x >= 0 && cell.x < mapSize && cell.y >= 0 && cell.y < mapSize) {
            mapCtx.fillRect(cell.x * cellWidth, cell.y * cellHeight, cellWidth, cellHeight);
          }
        });
      }
      
      // Draw robot position (green circle)
      if (mapData.robot) {
        const robotX = mapData.robot.x * cellWidth;
        const robotY = mapData.robot.y * cellHeight;
        const robotRadius = Math.max(cellWidth, cellHeight) * 1.5;
        
        // Robot circle
        mapCtx.fillStyle = '#00cc00';
        mapCtx.beginPath();
        mapCtx.arc(robotX, robotY, robotRadius, 0, 2 * Math.PI);
        mapCtx.fill();
        
        // Robot border
        mapCtx.strokeStyle = '#008800';
        mapCtx.lineWidth = 2;
        mapCtx.stroke();
        
        // Direction indicator (arrow)
        const angle = (mapData.robot.angle || 0) * Math.PI / 180;
        const dirLength = robotRadius * 2;
        mapCtx.strokeStyle = '#00aa00';
        mapCtx.lineWidth = 3;
        mapCtx.beginPath();
        mapCtx.moveTo(robotX, robotY);
        mapCtx.lineTo(robotX + dirLength * Math.cos(angle), robotY + dirLength * Math.sin(angle));
        mapCtx.stroke();
        
        // Draw direction text
        mapCtx.fillStyle = '#000000';
        mapCtx.font = 'bold 12px Arial';
        mapCtx.textAlign = 'center';
        mapCtx.fillText(Math.round(mapData.robot.angle) + '°', robotX, robotY - robotRadius - 10);
      }
      
      // Draw stats
      mapCtx.fillStyle = '#000000';
      mapCtx.font = '11px Arial';
      mapCtx.textAlign = 'left';
      let yOffset = 15;
      if (mapData.obstacles_count !== undefined) {
        mapCtx.fillText('Obstacles: ' + mapData.obstacles_count, 5, yOffset);
        yOffset += 16;
      }
      if (mapData.free_count !== undefined) {
        mapCtx.fillText('Free cells: ' + mapData.free_count, 5, yOffset);
        yOffset += 16;
      }
      mapCtx.fillText('Legend: 🟢 Robot | 🔵 Free | 🔴 Obstacle', 5, h - 5);
    }
    
    function sendCmd(key, value) {
      // Block manual commands if any autonomous mode is active
      if ((lineFollowingActive || obstacleAvoidingActive) && (key === 'move' || key === 'turn')) {
        console.log('Manual control disabled while autonomous mode is active');
        return;
      }
      if (socket && socket.readyState === 1) {
        socket.send(JSON.stringify({ key: key, value: value }));
      }
    }
    
    function toggleLineFollowing() {
      // Disable obstacle avoidance when enabling line following
      if (!lineFollowingActive && obstacleAvoidingActive) {
        obstacleAvoidingActive = false;
        document.getElementById('obstacleAvoidBtn').textContent = 'Obstacle Avoid: OFF';
        document.getElementById('obstacleAvoidBtn').classList.remove('active');
        sendCmd('obstacleAvoid', 'off');
      }
      
      lineFollowingActive = !lineFollowingActive;
      document.getElementById('lineFollowBtn').textContent = lineFollowingActive ? 'Line Following: ON' : 'Line Following: OFF';
      document.getElementById('lineFollowBtn').classList.toggle('active');
      sendCmd('lineFollow', lineFollowingActive ? 'on' : 'off');
    }
    
    function toggleObstacleAvoid() {
      // Disable line following when enabling obstacle avoidance
      if (!obstacleAvoidingActive && lineFollowingActive) {
        lineFollowingActive = false;
        document.getElementById('lineFollowBtn').textContent = 'Line Following: OFF';
        document.getElementById('lineFollowBtn').classList.remove('active');
        sendCmd('lineFollow', 'off');
      }
      
      obstacleAvoidingActive = !obstacleAvoidingActive;
      document.getElementById('obstacleAvoidBtn').textContent = obstacleAvoidingActive ? 'Obstacle Avoid: ON' : 'Obstacle Avoid: OFF';
      document.getElementById('obstacleAvoidBtn').classList.toggle('active');
      sendCmd('obstacleAvoid', obstacleAvoidingActive ? 'on' : 'off');
    }
  </script>
</body>
</html>
)rawliteral";

#endif // INTERFACE_H