import serial
import json
import time

from Controller import updateTemperatureHumidityStatus, updateFoodWaterLevelStatus, getMannualDispenseStatus, setManualDispenseFoodStatus, getAutomaticDispenseStatus, postDispenseLog

serialObject = serial.Serial('COM3', 9600, timeout = 1)

startTime = time.time()
time.sleep(3)

def getTempHum():
    sendRequest("TempHum")
    response = getResponse()
    responseJSON = json.loads(response)
    updateTemperatureHumidityStatus(responseJSON['temp'], responseJSON['hum'])

def getFoodWaterLevel():
    sendRequest("FoodWater")
    response = getResponse()
    responseJSON = json.loads(response)
    updateFoodWaterLevelStatus(responseJSON['food'], responseJSON['water'])

def ManualDispenseFood():
    dispenseStatus = getMannualDispenseStatus()

    dispenseCondition = dispenseStatus['dispenseCondition']
    serving = dispenseStatus['serving']

    if (dispenseCondition == 1):
        sendRequest(f"Dispense,{serving}")
        getResponse()
        setManualDispenseFoodStatus(False)
        postDispenseLog(serving, 'Manual')

def AutomaticDispenseFood():
    automaticDispenseFoodStatus = getAutomaticDispenseStatus()

    for dispenseSlot in automaticDispenseFoodStatus:
        dispenseTime = dispenseSlot['dispenseTime']
        serving = dispenseSlot['serving']
        
        currrTime = time.localtime()
        currrTimeSecs = 3600 * currrTime.tm_hour + 60 * currrTime.tm_min + currrTime.tm_sec
        difference = dispenseTime.total_seconds() - currrTimeSecs
        
        if (difference <= 1):
            sendRequest(f"Dispense,{serving}")
            getResponse()
            postDispenseLog(serving, 'Automatic')

def sendRequest(request):
    serialObject.write(request.encode('utf'))

def getResponse():
    response = ""

    while response == "":
        response = serialObject.readline().decode('utf-8').strip('\n')
    print(response)

    return response

getTempHum()
getFoodWaterLevel()

while True:
    currentTime = time.time()
    difference = currentTime - startTime

    if (difference >= 60 * 5):
        getTempHum()
        getFoodWaterLevel()
        
        #restart timer
        startTime = time.time()

    ManualDispenseFood()
    AutomaticDispenseFood()



