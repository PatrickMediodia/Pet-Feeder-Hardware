import mysql.connector

config = {
    'user': 'root',
    'password': '',
    'host': 'localhost',
    'database': 'petfeeder',
    'raise_on_warnings': True
}

def connect():
    return mysql.connector.connect(**config)