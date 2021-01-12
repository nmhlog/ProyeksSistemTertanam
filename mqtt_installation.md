
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo apt-get install mosquitto mosquitto-clients python-mosquitto

kalo gagal : 
# 1.  Installation Guide MQTT 

### Source : https://mosquitto.org/blog/2013/01/mosquitto-debian-repository/

```
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key
```

## check release first : cd /etc/apt/sources.list.d/
    - choice mosquitto : 
  
```
sudo wget http://repo.mosquitto.org/debian/mosquitto-jessie.list
sudo wget http://repo.mosquitto.org/debian/mosquitto-stretch.list
sudo wget http://repo.mosquitto.org/debian/mosquitto-buster.list
```

sudo apt-get update

apt-cache search mosquitto 
apt-get install mosquitto

# 2. masukan config.
sudo /etc/init.d/mosquitto stop

# Config file for mosquitto
##  src :https://mosquitto.org/man/mosquitto-conf-5.html
## See mosquitto.conf(5) for more information.
## Example from adafruit :https://learn.adafruit.com/diy-esp8266-home-security-with-lua-and-mqtt/configuring-mqtt-on-the-raspberry-pi
 

## Masukan dalam /etc/mosquitto/conf.d/*nama file bebas liat readme*.conf
```user mosquitto
max_queued_messages 200
message_size_limit 0
allow_zero_length_clientid true
allow_duplicate_messages false

listener 1883
autosave_interval 900
autosave_on_changes false
persistence true
persistence_file mosquitto.db
allow_anonymous true
password_file /etc/mosquitto/passwd
```

sudo /etc/init.d/mosquitto start

### kalo gagal : https://github.com/eclipse/mosquitto/issues/310

sudo update-rc.d mosquitto remove

sudo nano /etc/systemd/system/mosquitto.service

Paste and save this script
```
[Unit]
Description=Mosquitto MQTT Broker
Documentation=man:mosquitto(8)
Documentation=man:mosquitto.conf(5)
ConditionPathExists=/etc/mosquitto/mosquitto.conf
After=xdk-daemon.service

[Service]
ExecStart=/usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf
ExecReload=/bin/kill -HUP $MAINPID
User=mosquitto
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

sudo systemctl enable mosquitto.service
sudo reboot

check if mosquitto is running

sudo mosquitto -v




