from flask import Flask
from flask_ask import Ask, statement, question, session
from flask_mqtt import Mqtt
from threading import Timer
from threading import Event

import json
import config
import json
import requests
import time
import unidecode
import os


app = Flask(__name__)
config.conf(app)
mqtt = Mqtt(app)


def getIntent():
    with open('intentsData.json') as json_data:
        d = json.load(json_data)
        return d


intentsData = getIntent()


@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe('demo/response/#')


@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    data = dict(
        topic=message.topic,
        payload=message.payload.decode()
    )
    print(data)


ask = Ask(app, "/")


def get_String():
    return "David Ramirez Campoy"


@app.route('/')
def homepage():
    return "hi there, how ya doin?"


@ask.launch
def start_skill():
    welcome_message = 'say yes or not '
    return question(welcome_message)


@ask.intent("yesIntent")
def share_headlines():
    string_msg = 'oh you say yes'
    return statement(string_msg)


@ask.intent("noIntent")
def no_intent():
    bye_text = 'oh okay... bye'
    return statement(bye_text)


sensors = {
    'a1': 10,
    'a2': 15,
    'a3': 20
}
""" demo value {device} """


@ask.intent("valueIntent")
def get_value(device):
    global text
    text = ''
    exit = Event()

    def quit():
        exit.set()
    intentData = getIntentValue("valueIntent")
    slotData = getSlotValue(device)
    

    if slotData and intentData :

        topicResp = intentData["response"].format(slotData["name"])
        topicReq = intentData["request"].format(slotData["name"])
        print(str(topicResp))
        print(str(topicReq))
        mqtt.subscribe(topicResp)
        @mqtt.on_topic(topicResp)
        def response(client, userdata, message):
            global text
            text = str(message.payload.decode()) + " degrees"
            print(text)
            mqtt.unsubscribe(topicResp)
            quit()


        #temp or hum or pres
        mqtt.publish(topicReq, "temp")

        i = 0
        while (not exit.is_set()) and (i < 5):
            i = i + 1
            exit.wait(1)
        if i >= 5:
            text = 'device ' + device + ' response timeout'

        return statement(text)
    else:
        text = 'sorry but you say device ' + device
        return statement(text)

"""
alexa ask demo start l14 in 5 seconds
"""

@ask.intent("startIntent")
def setIntent(device, number, time):
    text = "ok";
    print(device, number, time)  
    seconds = toSeconds(number, time)
    def callbackAction():
        print(device)
        mqtt.publish("alx/value/request/a1",device)
    t = Timer(seconds, callbackAction)
    t.start()
 
    return statement(text)

def toSeconds(number, time):
    if time == "seconds" or time == "second":
        return float(number)
    if time == "minutes":
        return float(number * 60)
    if time == "hours" or time == "hour":
        return float(number * 3600)


@ask.intent("setIntent")
def setIntent(value):
    text = "ok"
    mqtt.publish("alx/set/request", value)

    return statement(text)




""" demo ip """


@ask.intent("ipIntent")
def ip(device):
    f = os.popen('hostname -I | cut -d\' \' -f1')
    ip = str(f.read())
    text = 'the ip server ' + str(ip)
    return statement(text)




def getSlotValue(slot):
    for element in intentsData["slots"]:
        if slot == element["name"]:
            return element
    return False

def getIntentValue(intent):
    for element in intentsData["intents"]:
        if intent == element["intentName"]:
            return element
    return False

if __name__ == '__main__':
    app.run(debug=True)
