import serial
import threading
import database
import time

SERIAL_PORT = "/dev/ttyUSB1"
BAUD_RATE = 9600

ser = None

serial_lock = threading.Lock()


def send_command(command):
    global ser

    if ser is None:
        print("Serial not connected")
        return False

    try:
        with serial_lock:
            ser.write((command + "\n").encode("utf-8"))

        print("Pi -> Arduino:", command)
        return True

    except Exception as e:
        print("Send error:", e)
        return False


def send_price(product, price):
    command = f"PRICE,{product},{price}"
    return send_command(command)


def send_all_prices():
    products = database.get_products()

    for product, price, stock in products:
        send_price(product, price)
        time.sleep(1.4)


def send_stock(product, stock):
    command = f"STOCK,{product},{stock}"
    return send_command(command)


def send_all_stocks():
    products = database.get_products()

    for product, price, stock in products:
        send_stock(product, stock)
        time.sleep(0.7)


def process_message(message):
    print("Arduino:", message)

    if message == "STATUS,ONLINE":
        print("Arduino1 online - resyncing price & stock")
        send_all_prices()
        send_all_stocks()
        return

    parts = message.split(",")

    if len(parts) != 4:
        return

    command = parts[0]
    product = parts[1]

    try:
        price = int(parts[2])
    except ValueError:
        return

    status = parts[3]

    if command == "SALE":
        database.add_sale(product, price, status)

        if status == "SUCCESS":
            database.decrease_stock(product, 1)
            new_stock = database.get_stock(product)

            if new_stock is not None:
                send_stock(product, new_stock)

            print(f"Sale: {product} {price} BAHT SUCCESS")
            print(f"Stock {product} - 1")

        elif status == "FAILED":
            print(f"Sale: {product} FAILED")


def serial_loop():
    global ser

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

        print("Serial connected:", SERIAL_PORT)

        time.sleep(2)

        send_all_prices()
        send_all_stocks()

        while True:
            line = (
                ser.readline().decode("utf-8", errors="ignore").strip()
            )

            if line:
                process_message(line)

    except Exception as e:
        print("Serial error:", e)


def start_serial_reader():
    thread = threading.Thread(target=serial_loop, daemon=True)
    thread.start()
    return thread