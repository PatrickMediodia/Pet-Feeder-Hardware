import Connection

def updateTemperatureHumidityStatus(temperature, humidity):
    connection = Connection.connect()

    cursor = connection.cursor()
    updateString = f'UPDATE temperaturehumiditystatus SET temperature = %s, humidity = %s, timestamp = CURRENT_TIMESTAMP()'
    cursor.execute(updateString, (temperature, humidity))
    connection.commit()
    
    connection.close()

def updateFoodWaterLevelStatus(foodLevel, waterLevel):
    connection = Connection.connect()

    cursor = connection.cursor()
    updateString = f'UPDATE foodwaterstatus SET foodLevel = %s, waterLevel = %s, timestamp = CURRENT_TIMESTAMP()'
    cursor.execute(updateString, (foodLevel, waterLevel))
    connection.commit()

    connection.close()

def getMannualDispenseStatus():
    connection = Connection.connect()

    cursor = connection.cursor()
    getString = f'SELECT * FROM dispense'
    cursor.execute(getString)

    record = cursor.fetchone()
    dispense_dict = {
        "dispenseCondition" : record[0], 
        "serving" : record[1],
        "lastDispense" : record[2]
    }

    connection.close()
    return dispense_dict

def setManualDispenseFoodStatus(dispenseCondition):
    connection = Connection.connect()

    cursor = connection.cursor()
    updateString = f'UPDATE dispense SET dispenseCondition = {dispenseCondition}, lastDispense = CURRENT_TIMESTAMP()'
    cursor.execute(updateString)
    connection.commit()

    connection.close()

def getAutomaticDispenseStatus():
    connection = Connection.connect()

    cursor = connection.cursor()
    getString = f'SELECT * FROM `dispenseslots` WHERE dispenseTime - CURRENT_TIME() <= 15 AND CURRENT_TIME() <= dispenseTime;'
    cursor.execute(getString)
    
    listOfTime = []
    for (dispenseTime, quantity) in cursor:
        listOfTime.append({
            "dispenseTime": dispenseTime, 
            "serving": quantity
        })
        
    connection.close()
    return listOfTime

def postDispenseLog(serving, mode):
    connection = Connection.connect()

    cursor = connection.cursor()
    getString = f'SELECT temperature, humidity FROM temperaturehumiditystatus;'
    cursor.execute(getString)
    record = cursor.fetchone()

    postString = f'INSERT INTO dispenselogs VALUES(CURRENT_TIMESTAMP(), %s, %s, %s, %s);'
    cursor.execute(postString, (record[0], record[1], serving, mode))

    connection.commit()
    connection.close()