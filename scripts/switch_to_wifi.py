import serial
import time

PORT     = "/dev/ttyUSB0"
BAUDRATE = 115200

print(f"Connecting to ESP32 on {PORT}...")
s = serial.Serial(PORT, BAUDRATE, timeout=3)
time.sleep(0.5)
s.write(b"ZTB|seq=1|cmd=TRANSPORT_SWITCH|mode=WIFI\r\n")
print("Sent switch to WIFI — waiting for IP...")
deadline = time.time() + 30
while time.time() < deadline:
    line = s.readline().decode(errors="ignore").strip()
    if line:
        print(line)
    if "ZTB|status=READY" in line or "port=5000" in line:
        break
s.close()
