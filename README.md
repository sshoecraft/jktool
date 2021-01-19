
JIKONG BMS linux-based utility

requires MYBMM for the modules

requires gattlib https://github.com/labapart/gattlib to be installed for bluetooth

Transports specified exactly as in mybmm.conf

jktool -t <transport:target,opt1[,optN]>


For CAN:

jktool -t can:<device>[,speed]

example:

	jktool -t can:can0,500000

For Serial:

jktool -t serial:<device>[,speed]

example:

	jktool -t serial:/dev/ttyS0,9600

For Bluetooth:

jktool -t bt:[mac addr][,desc]

exmples:

	jktool -t bt:01:02:03:04:05,06

	jktool -t bt:01:02:03:04:05:06,ff01

For IP/esplink:

jktool -t ip:<ip addr>[,port]

example:

	jktool -t ip:10.0.0.1,23

for CANServer/Can-over-ip

jktool -t can_ip:<ip addr>,[port],<interface>,[speed]

example:

	jktool -t can_ip:10.0.0.1,3930,can0,500000


>>> CAN bus cannot read/write parameters


to read all parameters using bluetooth:

jktool -t bt:01:02:03:04:06 -r -a

to list the params the program supports, use -l

to specify single params, specify them after -r

jktool -t bt:01:02:03:04:06 -r BalanceStartVoltage BalanceWindow

to read a list of parameters using a file use -f:

jktool -t serial:/dev/ttyS0,9600 -r -f jbd_settings.fig

use -j and -J (pretty) to specify filename is a json file


to write parameters, specify a key value pair after -w

jktool -t ip:10.0.0.1 -w BalanceStartVoltage 4050 BalanceWindow 20


to send all output to a file, use -o.   If the filename ends with .json, file will be written in JSON format:

jktool -t can:can0,500000 -j -o pack_1.json
