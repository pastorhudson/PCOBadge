# PCOBadge
E-Paper name badge for Planning Center Check-ins

PCOBadge is a fun project by Ron Hudson.
PCOBadge pulls an individual's checkin info from the Planning Center Check-ins api and displays it on an e-paper display.
It lives at https://github.com/pastorhudson/pcobadge
It is Liscenced under the: MIT License Copyright (c) 2018 Ron Hudson


## Libraries You Need

* e-paper display lib Version 3.x
https://github.com/ZinggJM/GxEPD/releases

* Adafruit GFX
* ESP8266 Core Latest
* ArduinoJson


# EDIT THIS! in the PCOBadge.ino
* `String userID = "xxxxxx";` Your User ID Number from check-ins. Use Just the Numerical Portion.

* Generate your Application ID and Secret personal access token pair at https://api.planningcenteronline.com/personal_access_tokens/new
Be sure to allow access to Check-ins Version 2018-08-01
```
String applicationID = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxAPPLICATION IDxxxxxxxxxxxxxxxxxxxxxx"; // Application ID
String secret = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxSecretxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // Secret
```

* Maybe edit this ¯\_(ツ)_/¯ SHA1 fingerprint of the certificate for api.planningcenteronline.com Expires Nov 24, 2019
`String fingerPRINT = "95 54 E1 9C 51 F1 9E 0A 2E 2F 41 51 9D A4 E4 83 26 80 71 7C";`
