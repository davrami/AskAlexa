from flask import Flask
from flask_ask import Ask, statement, question, session
from flask_mqtt import Mqtt
from threading import Timer
from threading import Event

import config
import json
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

#method for parse de data from intents structure
intentsData = getIntent()


@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe('alx/response/#')


@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    data = dict(
        topic=message.topic,
        payload=message.payload.decode()
    )
    print(data)

ask = Ask(app, "/")

@app.route('/')
def homepage():
    return "hi there, how ya doin?"

@ask.launch
def start_skill():
    speech_output = 'Hello, what you need to know?'
    reprompt_text = "hello?"
    return question(speech_output).reprompt(reprompt_text)

@ask.intent("yesIntent")
def share_headlines():
    string_msg = 'oh you say yes'
    return statement(string_msg)

@ask.intent("noIntent")
def no_intent():
    bye_text = 'oh okay... bye'
    return statement(bye_text)

""" set {value} or set {value} temperature"""
@ask.intent("setIntent")
def setIntent(value):
    intentData = getIntentValue("setIntent")
    text = intentData["responseText"]
    mqtt.publish(intentData["request"], intentData["requestPayload"].format(value))
    return statement(text)

""" value {device} """
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
            text = intentData["responseText"].format(str(message.payload.decode()));
            print(text)
            mqtt.unsubscribe(topicResp)
            quit()

        #temp or hum or pres
        mqtt.publish(topicReq, intentData["requestPayload"])

        i = 0
        while (not exit.is_set()) and (i < 5):
            i = i + 1
            exit.wait(1)
        if i >= 5:
            text = 'device ' + device + ' response timeout'

        return statement(text)
    else:
        
        text = intentData["statementError"].format(device)
        return statement(text)


""" tell me {item} or get {item}"""
@ask.intent("getValueIntent")
def get_valueItem(item):
    global text
    text = ''
    exit = Event()
    def quit():
        exit.set()
    intentData = getIntentValue("getValueIntent")
    slotData = getSlotValue("a1")
    
    if slotData and intentData :
        topicResp = intentData["response"].format(slotData["name"])
        topicReq = intentData["request"].format(slotData["name"])
        print(str(topicResp))
        print(str(topicReq))
        mqtt.subscribe(topicResp)
        @mqtt.on_topic(topicResp)
        def response(client, userdata, message):
            global text
            print(str(message.payload.decode()))
            text = intentData["responseText"].format(item,str(message.payload.decode()));
            print(text)
            mqtt.unsubscribe(topicResp)
            quit()

        #temp or hum or pres
        mqtt.publish(topicReq, item)

        i = 0
        while (not exit.is_set()) and (i < 5):
            i = i + 1
            exit.wait(1)
        if i >= 5:
            text = 'device response timeout'

        return statement(text)
    else:
        
        text = intentData["statementError"].format(item)
        return statement(text)


"""alexa ask pi start l14 in 5 seconds"""
@ask.intent("startIntent")
def startIntent(device, number, time):
    intentData = getIntentValue("startIntent")
    text = intentData["responseText"];
    print(device, number, time)  
    seconds = toSeconds(number, time)
    def callbackAction():
        print(device)
        mqtt.publish(intentData["request"],device)
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




""" pi ip """
@ask.intent("ipIntent")
def ip(device):
    f = os.popen('hostname -I | cut -d\' \' -f1')
    ip = str(f.read())
    text = 'the ip server ' + str(ip)
    return statement(text)


#Utility
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
