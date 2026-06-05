from ultralytics import YOLO
import cv2
import requests
import os
import time
import threading
from flask import Flask, render_template_string, Response

# ========= CONFIG =========
CAR_IP = "10.221.172.15"
ESP_CAM1 = "192.168.137.8:81"
ESP_CAM2 = "192.168.137.8:81"
MODEL_PATH = "yolov8n.pt"
# ==========================

app = Flask(__name__)
model = YOLO(MODEL_PATH)

if not os.path.exists("enemy"):
    os.makedirs("enemy")

camera = cv2.VideoCapture(0)
camera.set(cv2.CAP_PROP_FRAME_WIDTH,640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT,480)

last_capture = 0
last_alert_time = 0
person_count = 0

# ================= HTML =================
HTML = f"""
<!DOCTYPE html>
<html>
<head>
<title>AI Defense CMS</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<style>

body {{
margin:0;
background:#0e0e0e;
font-family:Arial;
color:white;
text-align:center;
}}

h1 {{
padding:15px;
color:#00ffcc;
}}

.cms-container {{
display:flex;
justify-content:center;
gap:8px;
padding:10px;
}}

.cms-container img {{
width:32%;
border:2px solid #00ffcc;
border-radius:8px;
}}

.section {{
margin-top:25px;
}}

.dpad {{
display:grid;
grid-template-columns:90px 90px 90px;
grid-template-rows:80px 80px 80px;
gap:10px;
justify-content:center;
}}

button {{
padding:12px;
border:none;
border-radius:8px;
background:#00ffcc;
font-size:16px;
cursor:pointer;
}}

button:hover {{
background:#00cc99;
}}

.stop-btn {{
background:#ff4444;
color:white;
}}

.emergency {{
background:red;
color:white;
margin-top:10px;
width:220px;
}}

.gun {{
background:#ffaa00;
color:black;
margin-top:10px;
width:220px;
}}

.counter {{
color:#ff4444;
margin-top:5px;
font-size:18px;
}}

#metalAlert {{
margin-top:10px;
font-size:20px;
font-weight:bold;
color:lime;
}}

</style>
</head>

<body>

<h1>🎥 AI Defense CMS Dashboard</h1>

<div class="counter">
Persons Detected: <span id="count">0</span>
</div>

<div id="metalAlert">
Metal Sensor: SAFE
</div>

<div class="cms-container">
<img src="/video">
<img src="http://{ESP_CAM1}/stream">
<img src="http://{ESP_CAM2}/stream">
</div>

<div class="section">

<h2>Head Control</h2>

<button onclick="send('head_forward')">Forward</button>
<button onclick="send('head_left')">Left</button>
<button onclick="send('head_stop')">Stop</button>

</div>

<div class="section">

<h2>Car Control</h2>

<div class="dpad">

<div></div>
<button onclick="send('forward')">↑</button>
<div></div>

<button onclick="send('left')">←</button>
<button class="stop-btn" onclick="send('stop')">STOP</button>
<button onclick="send('right')">→</button>

<div></div>
<button onclick="send('back')">↓</button>
<div></div>

</div>

<button class="emergency"
onmousedown="send('buzzer_on')"
onmouseup="send('buzzer_off')">
⚠ SELF DESTRUCTION
</button>

<button class="gun"
onmousedown="send('gun_on')"
onmouseup="send('gun_off')">
🔫 GUN FIRE
</button>

</div>

<script>

function send(cmd){{
fetch("http://{CAR_IP}/"+cmd);
}}

setInterval(function(){{
fetch('/count')
.then(response => response.text())
.then(data => {{
document.getElementById("count").innerText = data;
}});
}},1000);

setInterval(function(){{
fetch('/metal')
.then(response => response.text())
.then(data => {{

if(data == "1"){{
document.getElementById("metalAlert").innerHTML = "⚠ METAL DETECTED - POSSIBLE BOMB!";
document.getElementById("metalAlert").style.color = "red";
}}
else{{
document.getElementById("metalAlert").innerHTML = "Metal Sensor: SAFE";
document.getElementById("metalAlert").style.color = "lime";
}}

}});
}},1000);

</script>

</body>
</html>
"""

# ================= YOLO STREAM =================
def send_person_alert():
    try:
        requests.get(f"http://{CAR_IP}/person_detected",timeout=0.3)
    except:
        pass


def generate():

    global last_capture,last_alert_time,person_count

    while True:

        success,frame=camera.read()

        if not success:
            continue

        results=model(frame,conf=0.6)

        detected_person=False

        for r in results:

            for box in r.boxes:

                if model.names[int(box.cls[0])]=="person":

                    detected_person=True

                    x1,y1,x2,y2=map(int,box.xyxy[0])

                    cv2.rectangle(frame,(x1,y1),(x2,y2),(0,0,255),3)

                    cv2.putText(frame,"PERSON",(x1,y1-10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.9,(0,0,255),2)

        if detected_person:

            if time.time()-last_alert_time>2:

                threading.Thread(target=send_person_alert).start()

                last_alert_time=time.time()

                person_count+=1

            if time.time()-last_capture>3:

                cv2.imwrite(f"enemy/person_{int(time.time())}.jpg",frame)

                last_capture=time.time()

        _,buffer=cv2.imencode('.jpg',frame)

        frame=buffer.tobytes()

        yield(b'--frame\r\n'
        b'Content-Type: image/jpeg\r\n\r\n'+frame+b'\r\n')


# ================= ROUTES =================

@app.route('/')
def index():
    return render_template_string(HTML)

@app.route('/video')
def video():
    return Response(generate(),
    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/count')
def count():
    return str(person_count)

@app.route('/metal')
def metal():

    try:
        r=requests.get(f"http://{CAR_IP}/metal_status",timeout=0.5)
        return r.text
    except:
        return "0"


# ================= RUN =================

if __name__=="__main__":
    app.run(host='0.0.0.0',port=5000,threaded=True)