# import time
# import serial


# class TestbenchSerial:
#     ROBOT_LIBRARY_SCOPE = "SUITE"

#     def __init__(self):
#         self.ser = None

#     def open_testbench(self, port="/dev/ttyUSB0", baudrate=115200, timeout=2.0):
#         self.ser = serial.Serial(
#             port=port,
#             baudrate=int(baudrate),
#             timeout=0.1,
#             write_timeout=1,
#         )

#         time.sleep(0.5)
#         self.ser.reset_input_buffer()

#     def close_testbench(self):
#         if self.ser is not None and self.ser.is_open:
#             self.ser.close()

#     def send_ztb_command(self, frame, seq, timeout=2.0):
#         if self.ser is None or not self.ser.is_open:
#             raise AssertionError("Serial port is not open")

#         expected_prefix = f"ZTB|seq={seq}|"
#         deadline = time.time() + float(timeout)
#         seen_lines = []

#         self.ser.reset_input_buffer()
#         self.ser.write((frame + "\r\n").encode("utf-8"))
#         self.ser.flush()

#         while time.time() < deadline:
#             raw_line = self.ser.readline()

#             if not raw_line:
#                 continue

#             line = raw_line.decode("utf-8", errors="ignore").strip()

#             if not line:
#                 continue

#             seen_lines.append(line)

#             if line.startswith(expected_prefix):
#                 return line

#         raise AssertionError(
#             f"No response for seq={seq}. Seen lines: {seen_lines}"
#         )

#     def response_status_should_be_ok(self, response):
#         fields = self._parse_response(response)
#         status = fields.get("status")

#         if status != "OK":
#             raise AssertionError(f"Expected status=OK but got: {response}")

#     def response_field_should_equal(self, response, field, expected):
#         fields = self._parse_response(response)
#         actual = fields.get(field)

#         if str(actual) != str(expected):
#             raise AssertionError(
#                 f"Expected {field}={expected}, got {field}={actual}. Response: {response}"
#             )

#     def _parse_response(self, response):
#         fields = {}

#         for token in response.strip().split("|"):
#             if "=" in token:
#                 key, value = token.split("=", 1)
#                 fields[key] = value

#         return fields
import time
import serial

class TestbenchSerial:
    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self):
        self.ser = None

    def open_testbench(self, port="/dev/ttyUSB0", baudrate=115200, timeout=2.0):
        self.ser = serial.Serial(
            port=port,
            baudrate=int(baudrate),
            timeout=0.1,
            write_timeout=1,
        )
        time.sleep(0.5)
        self.ser.reset_input_buffer()

    def close_testbench(self):
        if self.ser is not None and self.ser.is_open:
            self.ser.close()

    def send_ztb_command(self, frame, seq, timeout=5.0):
        if self.ser is None or not self.ser.is_open:
            raise AssertionError("Serial port is not open")

        expected_prefix = f"ZTB|seq={seq}|"
        deadline = time.time() + float(timeout)
        seen_lines = []

        self.ser.reset_input_buffer()
        self.ser.write((frame + "\r\n").encode("utf-8"))
        self.ser.flush()

        while time.time() < deadline:
            raw_line = self.ser.readline()
            if not raw_line:
                continue
            line = raw_line.decode("utf-8", errors="ignore").strip()
            if not line:
                continue
            seen_lines.append(line)
            if line.startswith(expected_prefix):
                return line

        raise AssertionError(
            f"No response for seq={seq}. Seen lines: {seen_lines}"
        )

    def response_status_should_be_ok(self, response):
        fields = self._parse_response(response)
        status = fields.get("status")
        if status != "OK":
            raise AssertionError(f"Expected status=OK but got: {response}")

    def response_field_should_equal(self, response, field, expected):
        fields = self._parse_response(response)
        actual = fields.get(field)
        if str(actual) != str(expected):
            raise AssertionError(
                f"Expected {field}={expected}, got {field}={actual}. Response: {response}"
            )

    def _parse_response(self, response):
        fields = {}
        for token in response.strip().split("|"):
            if "=" in token:
                key, value = token.split("=", 1)
                fields[key] = value
        return fields