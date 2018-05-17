from flask import Flask
import ssl

def conf(app):
    app.config['MQTT_BROKER_URL'] = 'localhost'
    app.config['MQTT_BROKER_PORT'] = 1883
    app.config['MQTT_KEEPALIVE'] = 5
    app.config['MQTT_TLS_ENABLED'] = False
    app.config['MQTT_PASSWORD'] = '***********'
    app.config['MQTT_USERNAME'] = '***********'
    app.config['MQTT_REFRESH_TIME'] = 1.0  # refresh time in seconds
    return app

