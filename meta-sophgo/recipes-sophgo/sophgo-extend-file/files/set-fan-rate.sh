#!/bin/bash
function write_pwm(){
    for file in `ls $1`
    do
        if [ -d $1"/"$file ]
        then
            cd $1"/"$file
            if [ -e $2 ]
            then
            echo "$3" > $2
            fi
        fi
    done
}

value=$(ps | grep sophgo-fan-control.sh |wc -l)
if [ $value -ge 2 ]; then
    echo "stop sophgo-fan-control.service"
    systemctl stop sophgo-fan-control.service
fi


if [ $# -lt 1 ]; then
    echo "ERROR:  No command line arguments "
    exit 1
fi

case "$1" in
    [1-9][0-9]*)
        echo "$1 is number."
        ;;
    *)
        echo "Usage: $0 number {0~255}"
        exit 1
        ;;
esac

if [[ $1 -ge 0 && $1 -le 255 ]]; then
    write_pwm /sys/class/hwmon pwm1 $1
else
   echo "Usage: $0 number {0~255}"
   exit 1
fi




