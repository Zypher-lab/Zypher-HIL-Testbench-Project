import serial
import yaml

class serial_lib:

    ROBOT_LIBRARY_SCOPE = 'SUITE'

    def __init__(self):
        self._ser = None

    def open_bench(self, config_path):
        with open(config_path) as f:
            cfg = yaml.safe_load(f)
        self._ser = serial.Serial(
            port     = cfg['port'],
            baudrate = cfg['baud'],
            timeout  = cfg['timeout']
        )

    def close_bench(self):
        if self._ser and self._ser.is_open:
            self._ser.close()

    def send_command(self, frame):
        self._ser.write((frame + '\n').encode())
        response = self._ser.readline().decode().strip()
        return response

    def response_should_be_ok(self, response):
        if response != 'OK':
            raise AssertionError(f'Expected OK but got: {response}')