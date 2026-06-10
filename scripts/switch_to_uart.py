import socket
import time

HOST = "192.168.0.85"
PORT = 5000

print(f"Connecting to ESP32 at {HOST}:{PORT}...")
s = socket.create_connection((HOST, PORT), timeout=5)
s.sendall(b"ZTB|seq=1|cmd=TRANSPORT_SWITCH|mode=UART\r\n")
time.sleep(1)
response = s.recv(1024).decode(errors="ignore").strip()
print(f"Response: {response}")
s.close()
print("Done — ESP32 rebooting into UART mode.")
print("Connect picocom on /dev/ttyUSB1 to confirm.")
