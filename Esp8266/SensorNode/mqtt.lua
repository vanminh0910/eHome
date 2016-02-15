BROKER = "192.168.1.1"
BRPORT = 1883
BRUSER = "user"
BRPWD = "pwd"
CLIENTID = "ESP8266-" .. node.chipid()
PIN = 6 -- GPIO 12

dht22 = require("dht22")
dht22.read(PIN)
t = dht22.getTemperature() /10
h = dht22.getHumidity() /10

-- print "Connecting to MQTT broker. Please wait..."
m = mqtt.Client( CLIENTID, 120, BRUSER, BRPWD)
m:connect( BROKER , BRPORT, 0, function(conn)
    -- print("Connected to MQTT:" .. BROKER .. ":" .. BRPORT .." as " .. CLIENTID )
    m:publish("sensors/".. CLIENTID .. "/temperature",t,0,0, function(conn)
        -- print ("temp published")
        -- tmr.delay(10000)
        m:publish("sensors/".. CLIENTID .. "/humidity",h,0,0, function(conn)
            -- print ("pressure published")
            node.dsleep(5*1000000)
        end)
    end)
end)