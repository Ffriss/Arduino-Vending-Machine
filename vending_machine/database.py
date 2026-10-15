import sqlite3

DB_NAME = "vending.db"


def get_connection():
    return sqlite3.connect(DB_NAME)


def init_database():
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS products (
            product TEXT PRIMARY KEY,
            price INTEGER,
            stock INTEGER
        )
    """)

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS sales (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            product TEXT,
            price INTEGER,
            status TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)

    products = [
        ("A", 10, 10),
        ("B", 10, 10),
        ("C", 20, 10),
        ("D", 30, 10)
    ]

    cursor.executemany("""
        INSERT OR IGNORE INTO products
        (product, price, stock)
        VALUES (?, ?, ?)
    """, products)

    conn.commit()
    conn.close()


def get_products():
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT product, price, stock
        FROM products
        ORDER BY product
    """)

    data = cursor.fetchall()
    conn.close()

    return data


def get_stock(product):
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT stock
        FROM products
        WHERE product = ?
    """, (product,))

    row = cursor.fetchone()
    conn.close()

    return row[0] if row else None


def update_stock(product, stock):
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        UPDATE products
        SET stock = ?
        WHERE product = ?
    """, (stock, product))

    conn.commit()
    conn.close()


def add_stock(product, amount):
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        UPDATE products
        SET stock = stock + ?
        WHERE product = ?
    """, (amount, product))

    conn.commit()
    conn.close()


def add_sale(product, price, status):
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        INSERT INTO sales
        (product, price, status)
        VALUES (?, ?, ?)
    """, (product, price, status))

    conn.commit()
    conn.close()


def get_sales():
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT id, product, price, status, created_at
        FROM sales
        ORDER BY id DESC
        LIMIT 20
    """)

    data = cursor.fetchall()
    conn.close()

    return data


def get_total_sales():
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT COUNT(*)
        FROM sales
        WHERE status = 'SUCCESS'
    """)

    total = cursor.fetchone()[0]
    conn.close()

    return total


def get_total_revenue():
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT COALESCE(SUM(price), 0)
        FROM sales
        WHERE status = 'SUCCESS'
    """)

    total = cursor.fetchone()[0]
    conn.close()

    return total


def get_product_sales():
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT product, COUNT(*)
        FROM sales
        WHERE status = 'SUCCESS'
        GROUP BY product
        ORDER BY product
    """)

    data = cursor.fetchall()
    conn.close()

    return data


def set_stock(product, stock):
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        UPDATE products
        SET stock = ?
        WHERE product = ?
    """, (stock, product))

    conn.commit()
    conn.close()


def decrease_stock(product, amount):
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        UPDATE products
        SET stock = MAX(stock - ?, 0)
        WHERE product = ?
    """, (amount, product))

    conn.commit()
    conn.close()


def update_price(product, price):
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        UPDATE products
        SET price = ?
        WHERE product = ?
    """, (price, product))

    conn.commit()
    conn.close()