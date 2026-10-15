from flask import Flask, render_template, request, redirect
import database
import serial_reader

app = Flask(__name__)


@app.route("/")
def index():

    products = database.get_products()
    sales = database.get_sales()
    total_sales = database.get_total_sales()
    total_revenue = database.get_total_revenue()
    product_sales = database.get_product_sales()

    return render_template(
        "index.html",
        products=products,
        sales=sales,
        total_sales=total_sales,
        total_revenue=total_revenue,
        product_sales=product_sales
    )


@app.route("/add_stock", methods=["POST"])
def add_stock():

    product = request.form["product"]
    amount = int(request.form["amount"])

    if amount > 0:
        database.add_stock(product, amount)
        new_stock = database.get_stock(product)
        if new_stock is not None:
            serial_reader.send_stock(product, new_stock)

    return redirect("/")


@app.route("/decrease_stock", methods=["POST"])
def decrease_stock():

    product = request.form["product"]
    amount = int(request.form["amount"])

    if amount > 0:
        database.decrease_stock(product, amount)
        new_stock = database.get_stock(product)
        if new_stock is not None:
            serial_reader.send_stock(product, new_stock)

    return redirect("/")


@app.route("/set_stock", methods=["POST"])
def set_stock():

    product = request.form["product"]
    stock = int(request.form["stock"])

    if stock >= 0:
        database.set_stock(product, stock)
        serial_reader.send_stock(product, stock)

    return redirect("/")

if __name__ == "__main__":

    database.init_database()

    serial_reader.start_serial_reader()

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False
    )
