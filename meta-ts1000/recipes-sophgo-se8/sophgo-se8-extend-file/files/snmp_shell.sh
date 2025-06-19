#!/bin/bash

opt=$1
oid=$2
base_oid=".1.3.6.1.4.1.53367."
OIDS=(
  ".1.3.6.1.4.1.53367.1.1.0"
  ".1.3.6.1.4.1.53367.1.2.0"
  ".1.3.6.1.4.1.53367.1.3.0"
  ".1.3.6.1.4.1.53367.2.1.0"
  ".1.3.6.1.4.1.53367.2.2.0"
  ".1.3.6.1.4.1.53367.2.3.0"
  ".1.3.6.1.4.1.53367.2.4.0"
  ".1.3.6.1.4.1.53367.2.5.0"
  ".1.3.6.1.4.1.53367.2.6.0"
  ".1.3.6.1.4.1.53367.2.7.0"
  ".1.3.6.1.4.1.53367.2.8.0"
  ".1.3.6.1.4.1.53367.2.9.0"
  ".1.3.6.1.4.1.53367.2.10.0"
  ".1.3.6.1.4.1.53367.3.1.0"
  ".1.3.6.1.4.1.53367.3.2.0"
  ".1.3.6.1.4.1.53367.3.3.0"
  ".1.3.6.1.4.1.53367.3.4.0"
  ".1.3.6.1.4.1.53367.3.5.0"
  ".1.3.6.1.4.1.53367.3.6.0"
  ".1.3.6.1.4.1.53367.3.7.0"
  ".1.3.6.1.4.1.53367.3.8.0"
  ".1.3.6.1.4.1.53367.4.1.0"
  ".1.3.6.1.4.1.53367.4.2.0"
  ".1.3.6.1.4.1.53367.4.3.0"
  ".1.3.6.1.4.1.53367.4.4.0"
  ".1.3.6.1.4.1.53367.5.1.0"
  ".1.3.6.1.4.1.53367.5.2.0"
  ".1.3.6.1.4.1.53367.6.1.0"
  ".1.3.6.1.4.1.53367.6.2.0"
  ".1.3.6.1.4.1.53367.7.1.0"
)

handle_oid() {
    local last_three_oid=$1
    echo $base_oid$last_three_segments

    case "$last_three_oid" in
        "1.1.0")
            echo "Integer32"
            cat /sys/class/hwmon/hwmon0/temp1_input | sed 's/...$//' 2>/dev/null
            ;;
        "1.2.0")
            echo "Integer32"
            cat /sys/class/hwmon/hwmon1/temp1_input | sed 's/...$//' 2>/dev/null
            ;;
        "1.3.0")
            echo "Integer32"
            cat /sys/class/hwmon/hwmon2/temp1_input | sed 's/...$//' 2>/dev/null
            ;;
        "2.1.0")
            echo "String"
            awk '{result = $1 * 0.0013325; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in1_input 2>/dev/null
            ;;
        "2.2.0")
            echo "String"
            awk '{result = $1 * 0.001; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in3_input 2>/dev/null
            ;;
        "2.3.0")
            echo "String"
            awk '{result = $1 * 0.001; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in5_input 2>/dev/null
            ;;
        "2.4.0")
            echo "String"
            awk '{result = $1 * 0.0013325; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in7_input 2>/dev/null
            ;;
        "2.5.0")
            echo "String"
            awk '{result = $1 * 0.002435; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in10_input 2>/dev/null
            ;;
        "2.6.0")
            echo "String"
            awk '{result = $1 * 0.0013325; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in12_input 2>/dev/null
            ;;
        "2.7.0")
            echo "String"
            awk '{result = $1 * 0.0089; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in13_input 2>/dev/null
            ;;
        "2.8.0")
            echo "String"
            awk '{result = $1 * 0.00368; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in14_input 2>/dev/null
            ;;
        "2.9.0")
            echo "String"
            awk '{result = $1 * 0.002435; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in15_input 2>/dev/null
            ;;
        "2.10.0")
            echo "String"
            awk '{result = $1 * 0.003; if ($1 < 0) printf "-2.00\n"; else printf "%.2f\n", result}' /sys/class/hwmon/hwmon3/in16_input 2>/dev/null
            ;;
        "3.1.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/bmc_present_n xyz.openbmc_project.Gpio.bmc_present_n bmc_present_nState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "3.2.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/cpu0_m_a_event_lvt_n xyz.openbmc_project.Gpio.cpu0_m_a_event_lvt_n cpu0_m_a_event_lvt_nState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "3.3.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/cpu0_m_b_event_lvt_n xyz.openbmc_project.Gpio.cpu0_m_b_event_lvt_n cpu0_m_b_event_lvt_nState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "3.4.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/cpu0_memhot_n xyz.openbmc_project.Gpio.cpu0_memhot_n cpu0_memhot_nState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "3.5.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/cpu0_present xyz.openbmc_project.Gpio.cpu0_present cpu0_presentState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "3.6.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/cpu0_prochot_n xyz.openbmc_project.Gpio.cpu0_prochot_n cpu0_prochot_nState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "3.7.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/id_button xyz.openbmc_project.Gpio.id_button id_buttonState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "3.8.0")
            echo "Integer32"
            busctl get-property xyz.openbmc_project.Gpio /xyz/openbmc_project/gpio/post_complete xyz.openbmc_project.Gpio.post_complete post_completeState | awk '{print ($2=="true")?1:0}' 2>/dev/null
            ;;
        "4.1.0")
            echo "String"
            busctl get-property xyz.openbmc_project.State.Host /xyz/openbmc_project/state/POH xyz.openbmc_project.State.PowerOnHours POHCounter | awk '{result = $2*5; printf "%d\n", result}' 2>/dev/null
            ;;
        "4.2.0")
            echo "String"
            busctl get-property xyz.openbmc_project.FruDevice $(busctl tree xyz.openbmc_project.FruDevice --list | grep -m1 '/TS1000') xyz.openbmc_project.FruDevice PRODUCT_MANUFACTURER | awk -F'"' '/^s "/ {print $2}' 2>/dev/null
            ;;
        "4.3.0")
            echo "String"
            busctl get-property xyz.openbmc_project.FruDevice $(busctl tree xyz.openbmc_project.FruDevice --list | grep -m1 '/TS1000') xyz.openbmc_project.FruDevice PRODUCT_PRODUCT_NAME | awk -F'"' '/^s "/ {print $2}' 2>/dev/null
            ;;
        "4.4.0")
            echo "String"
            busctl get-property xyz.openbmc_project.FruDevice $(busctl tree xyz.openbmc_project.FruDevice --list | grep -m1 '/TS1000') xyz.openbmc_project.FruDevice PRODUCT_SERIAL_NUMBER | awk -F'"' '/^s "/ {print $2}' 2>/dev/null
            ;;
        "5.1.0")
            echo "Integer32"
            awk '{print $1}' /tmp/se8systeminfo 2>/dev/null
            ;;
        "5.2.0")
            echo "Integer32"
            awk '{print 100-$2}' /tmp/se8systeminfo 2>/dev/null
            ;;
        "6.1.0")
            echo "Integer32"
            awk '{print $3}' /tmp/se8systeminfo 2>/dev/null
            ;;
        "6.2.0")
            echo "Integer32"
            awk '{print 100-$4}' /tmp/se8systeminfo 2>/dev/null
            ;;
        "7.1.0")
            echo "Integer32"
            awk '{print $5}' /tmp/se8systeminfo 2>/dev/null
            ;;
        *)
            exit 0
            ;;
    esac
}

if [[ $opt == "-g" ]]; then
    last_three_segments=$(echo $oid | awk -F '.' '{for(i=NF-2;i<=NF;i++) printf $i (i<NF?".":"")}')
    handle_oid $last_three_segments
elif [[ $opt == "-n" ]]; then
    found=0
    for ((i=0; i<${#OIDS[@]}; i++)); do
        if [[ "$oid" < "${OIDS[$i]}" ]]; then
            last_three_segments=$(echo ${OIDS[$i]} | awk -F '.' '{for(i=NF-2;i<=NF;i++) printf $i (i<NF?".":"")}')
            handle_oid $last_three_segments
            exit 0
        fi
    done
    exit 0
else
    exit 0
fi
