import serial
import time
import requests

# Modify this
port = '/dev/cu.usbserial-DJ00S21M'
baud_rate = 115200

# Start serial communication
serialcomm = serial.Serial(port, baud_rate, timeout=1)
print("Serial communication running on Port " + port + " at a baud rate of", baud_rate)
print("   - Press Q to turn on the LED.")
print("   - Press W to turn off the LED.")
print("   - Press E to exit.\r\n")

previous_pressed_q = False
previous_pressed_w = False

current_weather = {}

def get_weather():
    try:
        response = requests.get(
            url="https://api.weatherapi.com/v1/current.json",
            params={
                "q": "Stanford",
                "key": "xxx",
            },
            headers={
                "Accept": "application/json",
            },
        )
        return response.json()
    except requests.exceptions.RequestException:
        print('HTTP Request failed')

def provide_rain_data():
    current_weather = get_weather()
    if current_weather:
        rain = current_weather["current"]["precip_mm"]
        message = f"RAIN_DATA|{rain:.1f}\n"
        print("SENDING: ", message)
        serialcomm.write(message.encode())

def get_astronomy():
    try:
        response = requests.get(
            url="https://api.weatherapi.com/v1/astronomy.json",
            params={
                "q": "Stanford",
                "key": "xxx",
            },
            headers={
                "Accept": "application/json",
            },
        )
        return response.json()
    except requests.exceptions.RequestException:
        print('HTTP Request failed')

def provide_current_temp():
    current_weather = get_weather()
    temp = current_weather["current"]["temp_c"]
    humidity = current_weather["current"]["humidity"]
    message = f"CURRENT_TEMP|{temp:.0f}|{humidity}\n"
    print("SENDING: ", message)
    serialcomm.write(message.encode())

def provide_sun_data():
    current_weather = get_astronomy()
    astro_data = current_weather["astronomy"]["astro"]
    sunrise = astro_data["sunrise"].split(":")
    sunset = astro_data["sunset"].split(":")

    sunrise_num1 = int(sunrise[0])
    sunrise_num2 = int(sunrise[1][:2])
    sunset_num1 = int(sunset[0])
    sunset_num2 = int(sunset[1][:2])
    if "PM" in sunset[1]:
        sunset_num1 += 12

    message = f"SUN_DATA|{sunrise_num1}|{sunrise_num2}|{sunset_num1}|{sunset_num2}\n"
    print("SENDING: ", message)
    serialcomm.write(message.encode())

keep_going = True
while keep_going:
    available = serialcomm.inWaiting()
    if available > 0:
        callback = serialcomm.readline().decode('ascii').strip().split("|")
        if (callback[0] == "REQ"):
            req_type = callback[1]
            if req_type == "CURRENT_TEMP":
                provide_current_temp()
            elif req_type == "SUN_DATA":
                provide_sun_data()
            elif req_type == "RAIN_DATA":
                provide_rain_data()
            
        print("RECEIVED:", callback)

# serialcomm.close()