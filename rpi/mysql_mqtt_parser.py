import mysql.connector as database
from mysql.connector import Error
import paho.mqtt.client as mqttClient
import time
import json
 
def on_connect(client, userdata, flags, rc):
 
    if rc == 0:
 
        print("Connected to broker")
 
        global Connected                #Use global variable
        Connected = True                #Signal connection 
 
    else:
 
        print("Connection failed")
 
Connected = False   #global variable for the state of the connection
 
broker_address= "localhost"
port = 1883
user = "espmqtt"
password = "lego1337"
 
client = mqttClient.Client("Python")               #create new instance
client.username_pw_set(user, password=password)    #set username and password
client.on_connect= on_connect                      #attach function to callback
client.connect(broker_address, port=port)          #connect to broker
 
client.loop_start()        #start the loop

def callback(client, userdata, message):
    parseit(str(message.payload.decode("utf-8"))) 

def parseit( location ):
    try:
        db = database.connect(
        host = "localhost",
        user = "admin",
        passwd = "lego1337",
        database = "test"
        )
        cursor = db.cursor()
        ##query = ("SELECT * FROM dbc_1 where row = %s")
        query = ("SELECT JSON_OBJECT('id', id, 'row', row, 'width', width, 'outerdiam', outerdiam, 'segs', segs) FROM dbc_4 where id = %s")

        ## getting records from the table
        cursor.execute(query, (location, ))

        ## fetching all records from the 'cursor' object
        records = cursor.fetchone()
        
        ##s = str(record)
        s = (str(records))
        
        # Strip the JSON to skip some overhead on the IC
        s = s.replace(',)', '')
        s = s.replace('(', '')
        s = s.replace("'", '')
        
        
        # Send SQL data to MQTT-client
        client.publish("esp/test",s)
            
            
        ## Showing the data
    except mysql.connector.Error as error:
            return ((error))
            



while Connected != True:    #Wait for connection
    time.sleep(0.1)
 
client.on_message = callback

# Quality of serice level 2, four part handshake. Messages is sent exactly once.

client.subscribe("rpi", qos=2) 
client.loop_forever()