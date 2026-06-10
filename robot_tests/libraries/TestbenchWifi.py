import socket
import time


class TestbenchWifi:
    """
    Robot Framework library for ZTB testbench over WiFi TCP.
    Drop-in replacement for TestbenchSerial — same keywords, same API.
    """
    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self):
        self.sock   = None
        self.buffer = b""

    def open_testbench(self, host="192.168.1.100", port=5000, timeout=10.0):
        """Connect to ESP32 TCP server. Call in Suite Setup."""
        self.sock = socket.create_connection((host, int(port)), timeout=float(timeout))
        self.sock.settimeout(0.1)
        self.buffer = b""
        time.sleep(0.3)
        # drain any boot messages
        try:
            while True:
                self.buffer += self.sock.recv(1024)
        except socket.timeout:
            pass
        self.buffer = b""

    def close_testbench(self):
        """Disconnect. Call in Suite Teardown."""
        if self.sock:
            self.sock.close()
            self.sock = None

    def send_ztb_command(self, frame, seq, timeout=5.0):
        if self.sock is None:
            raise AssertionError("Not connected")

        expected_prefix = f"ZTB|seq={seq}|"
        deadline = time.time() + float(timeout)
        seen_lines = []

        # flush receive buffer
        self.buffer = b""
        try:
            while True:
                self.buffer += self.sock.recv(1024)
        except socket.timeout:
            pass
        self.buffer = b""

        # send command
        self.sock.sendall((frame + "\r\n").encode("utf-8"))

        # wait for matching response
        while time.time() < deadline:
            try:
                chunk = self.sock.recv(1024)
                if chunk:
                    self.buffer += chunk
            except socket.timeout:
                pass

            while b"\n" in self.buffer:
                idx  = self.buffer.index(b"\n")
                raw  = self.buffer[:idx]
                self.buffer = self.buffer[idx + 1:]
                line = raw.decode("utf-8", errors="ignore").strip()
                if not line:
                    continue
                seen_lines.append(line)
                if line.startswith(expected_prefix):
                    return line

        raise AssertionError(
            f"No response for seq={seq}. Seen lines: {seen_lines}"
        )

    def response_status_should_be_ok(self, response):
        fields = self._parse(response)
        if fields.get("status") != "OK":
            raise AssertionError(f"Expected status=OK but got: {response}")

    def response_field_should_equal(self, response, field, expected):
        fields = self._parse(response)
        actual = fields.get(field)
        if str(actual) != str(expected):
            raise AssertionError(
                f"Expected {field}={expected}, got {field}={actual}. Response: {response}"
            )

    def _parse(self, response):
        fields = {}
        for token in response.strip().split("|"):
            if "=" in token:
                k, v = token.split("=", 1)
                fields[k] = v
        return fields
