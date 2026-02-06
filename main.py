from arduino_iot_cloud import ArduinoCloudClient
import serial
import glob
import re
import time
import subprocess
import asyncio
import serial_asyncio
import nest_asyncio

DEVICE_ID  = b"e788959f-1cd6-4c9f-a474-64ffcff6d966"
SECRET_KEY = b"!RH7GzTmC5mSwMIMA1XH?J@RD"

home_temp  = 0.0
home_hum   = 0.0

def get_cpu_temp():
    temp = subprocess.check_output(["vcgencmd", "measure_temp"]).decode()
    return float(temp.replace("temp=", "").replace("'C\n", ""))

def read_cpu_temp(client):
    cpu_temp = get_cpu_temp()
    print("[+] CPU temperature:", cpu_temp)
    return cpu_temp

def read_home_temp(client):
    print("[+] Home temperature:", home_temp)
    return home_temp

def read_home_hum(client):
    print("[+] Home humidity:", home_hum)
    return home_hum

# -----------------------------
def parse_temp_hum(line):
    try:
        parts = line.split(",")
        temp = float(parts[0])
        hum = float(parts[1])
        return temp, hum
    except Exception:
        return None, None

def find_arduino_port():
    ports = glob.glob('/dev/ttyUSB*')
    for port in ports:
        try:
            import serial
            ser = serial.Serial(port, 115200, timeout=1)
            ser.close()
            print(f"[+] Arduino found on {port}")
            return port
        except:
            pass
    return None

async def read_serial(port="/dev/ttyUSB0", baudrate=115200):
    global home_temp, home_hum

    reader, writer = await serial_asyncio.open_serial_connection(url=port, baudrate=baudrate)
    buffer = ""
    print("[+] Serial connection opened")

    while True:
        data = await reader.read(100)
        print("+++ Serial data: ", data)
        if not data:
            await asyncio.sleep(0.1)
            continue

        buffer += data.decode(errors='ignore')
        while '\n' in buffer:
            line, buffer = buffer.split('\n', 1)
            line = line.strip()
            if line:
                temp, hum = parse_temp_hum(line)
                if temp is not None and hum is not None:
                    home_temp = temp
                    home_hum = hum
                    print(f"[+] Home Temp={home_temp}, Hum={home_hum}")


# -----------------------------
import nest_asyncio
nest_asyncio.apply()
async def main():
    global home_temp, home_hum

    # Arduino IoT Cloud клиент
    client = ArduinoCloudClient(
        device_id=DEVICE_ID,
        username=DEVICE_ID,
        password=SECRET_KEY
    )

    port = find_arduino_port()
    if not port:
        print("No Arduino found!")
        return


    client.register("cpu_temperature", on_read=read_cpu_temp, interval=1.0)
    client.register("home_temperature", on_read=read_home_temp, interval=1.0)
    client.register("home_humidity", on_read=read_home_hum, interval=1.0)

    # client.start()

    task_serial = asyncio.create_task(read_serial(port, 115200))
    task_client = asyncio.create_task(client.run(interval=1.0, backoff=0.5))

    await asyncio.gather(task_serial, task_client)


# -----------------------------
if __name__ == "__main__":
    asyncio.run(main())
