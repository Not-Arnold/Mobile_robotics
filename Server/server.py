from fastapi import FastAPI, WebSocket, WebSocketDisconnect, Request, Header, HTTPException
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles # Add this import at the top
import json
from typing import List

app = FastAPI()

# Add this line right after "app = FastAPI()"
app.mount("/assets", StaticFiles(directory="assets"), name="assets")

# --- CONNECTION MANAGER ---
class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []
    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)
    def disconnect(self, websocket: WebSocket):
        if websocket in self.active_connections:
            self.active_connections.remove(websocket)
    async def broadcast(self, message: dict):
        for connection in self.active_connections:
            try: await connection.send_json(message)
            except: pass

manager = ConnectionManager()
robot_obstacles = {} 

# --- DASHBOARD HTML/JS ---
INDEX_HTML = r"""
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/><title>Delivery Tracker</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: white; margin: 20px; }
    .main-layout { display: flex; gap: 20px; justify-content: center; align-items: flex-start; }
    .card { background: #2d2d2d; padding: 20px; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); position: relative; }
    #arena-container { position: relative; width: 600px; height: 450px; border-radius: 10px; overflow: hidden; border: 2px solid #444;}
    #map-canvas { position: absolute; top: 0; left: 0; z-index: 1; background: #222; }
    #arena { width: 600px; height: 450px; position: relative; z-index: 2; background: transparent; }
    .node { width: 10px; height: 10px; background: #00d2ff; border-radius: 50%; position: absolute; transform: translate(-50%, -50%); z-index: 3; }
    .node-label { font-size: 11px; color: #888; position: absolute; transform: translate(-50%, 12px); z-index: 3; }
    #car { position: absolute; transition: all 0.4s ease; z-index: 10; transform: translate(-50%, -50%); width: 50px; height: 50px; background-image: url('/assets/car.png'); background-size: contain; background-repeat: no-repeat; left: -50px; top: -50px;}
    
    .stats-sidebar { width: 280px; display: flex; flex-direction: column; gap: 10px; }
    .stat-box { background: #3d3d3d; padding: 15px; border-radius: 10px; text-align: center; border-left: 4px solid #00d2ff; }
    .val { font-size: 28px; font-weight: bold; color: #00d2ff; display: block; }
    .route-display { font-family: monospace; background: #111; padding: 12px; border-radius: 8px; color: #2ecc71; border: 1px solid #333; font-size: 13px; min-height: 20px;}
  </style>
</head>
<body>
  <h2 style="text-align:center; color: #00d2ff; margin-bottom: 30px;">ASUN2881 DELIVERY TRACKER</h2>
  
  <div class="main-layout">
    <div class="card">
      <div id="arena-container">
        <canvas id="map-canvas" width="600" height="450"></canvas>
        <div id="arena"><div id="car"></div></div>
      </div>
    </div>

    <div class="stats-sidebar">
      <div class="stat-box"><small>LEFT PWM</small><span id="pl" class="val">0</span></div>
      <div class="stat-box" style="border-left-color: #2ed573;"><small>RIGHT PWM</small><span id="pr" class="val">0</span></div>
      <div class="stat-box" style="border-left-color: #ff4757;"><small>STATUS</small><span id="status" class="val" style="font-size: 18px;">WAITING...</span></div>
      
      <div class="card" style="padding: 10px;">
        <small style="color:#aaa; display:block; margin-bottom:5px;">ACTIVE ROUTE</small>
        <div id="route" class="route-display">---</div>
      </div>
    </div>
  </div>

<script>
    const canvas = document.getElementById('map-canvas');
    const ctx = canvas.getContext('2d');
    
    // 1. PRELOAD YOUR IMAGES
    const carImg = new Image();
    carImg.src = '/assets/car.png'; // REPLACE WITH YOUR CAR URL OR FILENAME

    const obsImg = new Image();
    obsImg.src = '/assets/obstacle.png'; // REPLACE WITH YOUR OBSTACLE URL OR FILENAME

    // --- ELONGATED MAP COORDINATES ---
    const nodeCoords = {
        5: {x: -24, y: 0}, 
        7: {x: -14, y: 0}, 
        1: {x: 4, y: 0},   
        6: {x: 22, y: 0},  

        3: {x: -12, y: 8}, 
        4: {x: -12, y: -8},
        2: {x: 18, y: 8},  
        0: {x: 18, y: -8}  
    };

    function toPxX(x) { return 300 + (x * 11); } 
    function toPxY(y) { return 225 - (y * 15); }

    let activeObstacles = []; 

    function drawMap() {
        // Use the global ctx we defined at the top
        if (!ctx) return;
        
        ctx.clearRect(0, 0, 600, 450);
        ctx.strokeStyle = '#555'; 
        ctx.lineWidth = 4; 
        ctx.lineCap = 'round';
        
        const drawCurve = (sIdx, eIdx, cpX, cpY) => {
            const s = nodeCoords[sIdx], e = nodeCoords[eIdx];
            ctx.beginPath(); 
            ctx.moveTo(toPxX(s.x), toPxY(s.y));
            ctx.quadraticCurveTo(toPxX(cpX), toPxY(cpY), toPxX(e.x), toPxY(e.y)); 
            ctx.stroke();
        };

        const drawStraight = (sIdx, eIdx) => {
            const s = nodeCoords[sIdx], e = nodeCoords[eIdx];
            ctx.beginPath(); 
            ctx.moveTo(toPxX(s.x), toPxY(s.y));
            ctx.lineTo(toPxX(e.x), toPxY(e.y)); 
            ctx.stroke();
        };

        // --- DRAW TRACK ---
        drawStraight(5, 7); 
        drawStraight(7, 1); 
        drawStraight(1, 6); 
        drawCurve(6, 2, 25, 6);    
        drawCurve(6, 0, 25, -6);   
        drawCurve(7, 3, -15, 6);    
        drawCurve(7, 4, -15, -6);   
        drawCurve(3, 2, 3, 12);    
        drawCurve(0, 4, 3, -12);   

        // --- DRAW OBSTACLES ---
        activeObstacles.forEach(obs => {
            const n1 = nodeCoords[obs.u], n2 = nodeCoords[obs.v];
            if(!n1 || !n2) return;

            let midX, midY;
            const curves = {
                '6-2': {cpX: 25, cpY: 6},  '2-6': {cpX: 25, cpY: 6},
                '6-0': {cpX: 25, cpY: -6}, '0-6': {cpX: 25, cpY: -6},
                '7-3': {cpX: -15, cpY: 6}, '3-7': {cpX: -15, cpY: 6},
                '7-4': {cpX: -15, cpY: -6},'4-7': {cpX: -15, cpY: -6},
                '3-2': {cpX: 3, cpY: 12},  '2-3': {cpX: 3, cpY: 12},
                '0-4': {cpX: 3, cpY: -12}, '4-0': {cpX: 3, cpY: -12}
            };

            const edgeKey = `${obs.u}-${obs.v}`;
            if (curves[edgeKey]) {
                const cp = curves[edgeKey];
                midX = 0.25 * n1.x + 0.5 * cp.cpX + 0.25 * n2.x;
                midY = 0.25 * n1.y + 0.5 * cp.cpY + 0.25 * n2.y;
            } else {
                midX = (n1.x + n2.x) / 2;
                midY = (n1.y + n2.y) / 2;
            }

            if (obsImg.complete) {
                ctx.drawImage(obsImg, toPxX(midX) - 20, toPxY(midY) - 20, 40, 40);
            }
        });
    }

    function initNodes() {
        const arena = document.getElementById('arena');
        Object.keys(nodeCoords).forEach(id => {
            const n = nodeCoords[id];
            const dot = document.createElement('div'); dot.className = 'node';
            dot.style.left = toPxX(n.x)+'px'; dot.style.top = toPxY(n.y)+'px';
            arena.appendChild(dot);
            const lbl = document.createElement('div'); lbl.className = 'node-label';
            lbl.style.left = toPxX(n.x)+'px'; lbl.style.top = toPxY(n.y)+'px';
            lbl.innerText = id; arena.appendChild(lbl);
        });
    }

    function connect() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const ws = new WebSocket(`${protocol}//${window.location.host}/ws`);
        ws.onmessage = (ev) => {
            const data = JSON.parse(ev.data);
            if(data.curr !== undefined && nodeCoords[data.curr]) {
                const car = document.getElementById('car');
                car.style.left = toPxX(nodeCoords[data.curr].x) + 'px';
                car.style.top = toPxY(nodeCoords[data.curr].y) + 'px';
            }
            if(data.obstacles) { activeObstacles = data.obstacles; drawMap(); }
            if(data.pwmL !== undefined) document.getElementById('pl').innerText = data.pwmL;
            if(data.pwmR !== undefined) document.getElementById('pr').innerText = data.pwmR;
            if(data.state) document.getElementById('status').innerText = data.state;
            if(data.route) document.getElementById('route').innerText = data.route;
        };
        ws.onclose = () => setTimeout(connect, 2000);
    }

    window.onload = () => { initNodes(); drawMap(); connect(); };
</script>
</body>
</html>
"""

@app.get("/", response_class=HTMLResponse)
async def get_dashboard():
    return INDEX_HTML

@app.post("/telemetry/{robot_id}")
async def post_telemetry(robot_id: str, req: Request, x_api_token: str = Header(None)):
    if x_api_token != "twiningESP": raise HTTPException(status_code=403)
    data = await req.json()
    
    if robot_id not in robot_obstacles: robot_obstacles[robot_id] = []
    
    u, v = data.get("obs_u"), data.get("obs_v")
    if u is not None and v is not None:
        new_obs = {"u": u, "v": v}
        if new_obs not in robot_obstacles[robot_id]:
            robot_obstacles[robot_id].append(new_obs)
            if len(robot_obstacles[robot_id]) > 3: 
                robot_obstacles[robot_id].pop(0)

    data["obstacles"] = robot_obstacles[robot_id]
    await manager.broadcast(data)
    return {"status": "ok"}

@app.post("/reset/{robot_id}")
async def reset_robot(robot_id: str):
    robot_obstacles[robot_id] = []
    await manager.broadcast({
        "curr": 4, 
        "obstacles": [],
        "state": "RECONNECTED",
        "route": "---"
    })
    return {"status": "cleared"}

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        while True: await websocket.receive_text()
    except WebSocketDisconnect: manager.disconnect(websocket)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8001)