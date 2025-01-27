/*
// Copyright (c) 2018-2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#include "gpio_control.hpp"

#include <sys/sysinfo.h>
#include <systemd/sd-journal.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <gpiod.hpp>
#include <nlohmann/json.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/object_server.hpp>
 #include <boost/date_time/posix_time/posix_time.hpp>
 #include <boost/thread/thread.hpp>

 #include <iostream>
 #include <boost/asio.hpp>
 #include <boost/thread.hpp>
 #include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>

#include <filesystem>
#include <fstream>
#include <string_view>

#include <boost/bind/placeholders.hpp>


#include <thread>

extern "C"
{
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
}

namespace gpio_control
{
static boost::asio::io_service io;
std::shared_ptr<sdbusplus::asio::connection> conn;

static std::string node = "0";
static const std::string appName = "sophgo-gpio-control";

std::atomic<int> first_wdt_event_flag(false);

#define WDT_DELAY 60

namespace power_control
{
const static constexpr char* busname = "xyz.openbmc_project.State.Host";
const static constexpr char* path = "/xyz/openbmc_project/state/host0";
const static constexpr char* interface = "xyz.openbmc_project.State.Host";
const static constexpr char* property = "RequestedHostTransition";
const static constexpr char* forceRestart = "xyz.openbmc_project.State.Host.Transition.ForceWarmReboot";
} // namespace power_control

namespace properties
{
constexpr const char* interface = "org.freedesktop.DBus.Properties";
constexpr const char* get = "Get";
constexpr const char* set = "Set";
}


enum class DbusConfigType
{
    name = 1,
    path,
    interface,
    property
};
boost::container::flat_map<DbusConfigType, std::string> dbusParams = {
    {DbusConfigType::name, "DbusName"},
    {DbusConfigType::path, "Path"},
    {DbusConfigType::interface, "Interface"},
    {DbusConfigType::property, "Property"}};

enum class ConfigType
{
    GPIO = 1,
    DBUS,
    I2C
};

enum HostFlashPort
{
    BMC = 0,
    HOST
};

enum IdentifyLedSet
{
    ON = 0,
    OFF
};

enum SolPort
{
    H0U0 = 0,
    H0U1,
    H1U0,
    H1U1
};

struct ConfigData
{
    std::string name;
    std::string lineName;
    std::string dbusName;
    std::string path;
    std::string interface;
    bool polarity;
    ConfigType type;
};




static ConfigData sata1ExistConfig;
static ConfigData sata2ExistConfig;
static ConfigData fan4ExistConfig;
static ConfigData fan5ExistConfig;
static ConfigData fan6ExistConfig;
static ConfigData fan8ExistConfig;
static ConfigData fan3ExistConfig;
static ConfigData fan1ExistConfig;
static ConfigData PSUPowerOnConfig;
static ConfigData fan2ExistConfig;
static ConfigData riserCardThrottleConfig;
static ConfigData hdd1ExistConfig;
static ConfigData solUartV2Config;
static ConfigData solUartV3Config;
static ConfigData frontVgaExitConfig;
static ConfigData fan9ExistConfig;
static ConfigData fan7ExistConfig;
static ConfigData hdd2ExistConfig;
static ConfigData uidButtonConfig;
static ConfigData hdd3ExistConfig;
static ConfigData phyIrqConfig;
static ConfigData hdd4ExistConfig;
static ConfigData fan1PowerEnConfig;
static ConfigData fan2PowerEnConfig;
static ConfigData fan3PowerEnConfig;
static ConfigData fan4PowerEnConfig;
static ConfigData fan5PowerEnConfig;
static ConfigData fan6PowerEnConfig;
static ConfigData fan7PowerEnConfig;
static ConfigData fan8PowerEnConfig;
static ConfigData fan9PowerEnConfig;
static ConfigData cpuaFlashH0Config;
static ConfigData cpuaFlashH1Config;
static ConfigData cpubFlashH2Config;
static ConfigData cpubFlashH3Config;
static ConfigData identifyLedGetConfig;
static ConfigData identifyLedSetConfig;
static ConfigData hostWdtConfig;




// map for storing list of gpio parameters whose config are to be read from sophgo
// gpio control json config
boost::container::flat_map<std::string, ConfigData*> powerSignalMap = {
    {"sata1Exist",         &sata1ExistConfig},
    {"sata2Exis",          &sata2ExistConfig},
    {"fan4Exist",          &fan4ExistConfig},
    {"fan5Exist",          &fan5ExistConfig},
    {"fan6Exist",          &fan6ExistConfig},
    {"fan8Exist",          &fan8ExistConfig},
    {"fan3Exist",          &fan3ExistConfig},
    {"fan1Exist",          &fan1ExistConfig},
    {"PSUPowerOn",         &PSUPowerOnConfig},
    {"fan2Exist",          &fan2ExistConfig},
    {"riserCardThrottle",  &riserCardThrottleConfig},
    {"hdd1Exist",          &hdd1ExistConfig},
    {"solUartV2",          &solUartV2Config},
    {"solUartV3",          &solUartV3Config},
    {"frontVgaExist",      &frontVgaExitConfig},
    {"fan9Exist",          &fan9ExistConfig},
    {"fan7Exist",          &fan7ExistConfig},
    {"hdd2Exist",          &hdd2ExistConfig},
    {"uidButton",          &uidButtonConfig},
    {"hdd3Exist",          &hdd3ExistConfig},
    {"phyIrq",             &phyIrqConfig},
    {"hdd4Exist",          &hdd4ExistConfig},
    {"fan1PowerEn",        &fan1PowerEnConfig},
    {"fan2PowerEn",        &fan2PowerEnConfig},
    {"fan3PowerEn",        &fan3PowerEnConfig},
    {"fan4PowerEn",        &fan4PowerEnConfig},
    {"fan5PowerEn",        &fan5PowerEnConfig},
    {"fan6PowerEn",        &fan6PowerEnConfig},
    {"fan7PowerEn",        &fan7PowerEnConfig},
    {"fan8PowerEn",        &fan8PowerEnConfig},
    {"fan9PowerEn",        &fan9PowerEnConfig},
    {"cpuaFlashH0",        &cpuaFlashH0Config},
    {"cpuaFlashH1",        &cpuaFlashH1Config},
    {"cpubFlashH2",        &cpubFlashH2Config},
    {"cpubFlashH3",        &cpubFlashH3Config},
    {"identifyLedGet",     &identifyLedGetConfig},
    {"identifyLedSet",     &identifyLedSetConfig},
    {"hostWdt",            &hostWdtConfig}};



static std::string gpioDbusName = "xyz.openbmc_project.Gpio";
static std::shared_ptr<sdbusplus::asio::dbus_interface> sataExistIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> hddExistIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> fanExistIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> PSUPowerOnIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> riserCardIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> solUartIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> cpuaFlashIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> cpubFlashIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> phyIrqIface;//作用未知
static std::shared_ptr<sdbusplus::asio::dbus_interface> identifyLedIface;




// LED flashing cycle
boost::container::flat_map<std::string, int> TimerMap = {
    {"PowerPulseMs",    200  },
    {"ForceOffPulseMs", 15000},
    {"ResetPulseMs",    500  },
    {"LedSwitchPullMs", 300  }};


// Timers
// Time holding GPIOs asserted
static boost::asio::steady_timer gpioAssertTimer(io);
static boost::asio::deadline_timer hostWdtTimer(io);





// GPIO Lines and Event Descriptors
static gpiod::line sata1ExistLine;
static boost::asio::posix::stream_descriptor sata1ExistEvent(io);
static gpiod::line sata2ExisLine;
static boost::asio::posix::stream_descriptor sata2ExisEvent(io);
static gpiod::line fan4ExistLine;
static boost::asio::posix::stream_descriptor fan4ExistEvent(io);
static gpiod::line fan5ExistLine;
static boost::asio::posix::stream_descriptor fan5ExistEvent(io);
static gpiod::line fan6ExistLine;
static boost::asio::posix::stream_descriptor fan6ExistEvent(io);
static gpiod::line fan8ExistLine;
static boost::asio::posix::stream_descriptor fan8ExistEvent(io);
static gpiod::line fan3ExistLine;
static boost::asio::posix::stream_descriptor fan3ExistEvent(io);
static gpiod::line fan1ExistLine;
static boost::asio::posix::stream_descriptor fan1ExistEvent(io);
static gpiod::line PSUPowerOnLine;
static boost::asio::posix::stream_descriptor PSUPowerOEvent(io);
static gpiod::line fan2ExistLine;
static boost::asio::posix::stream_descriptor fan2ExistEvent(io);
static gpiod::line riserCardThLine;
static boost::asio::posix::stream_descriptor riserCardThrottleEvent(io);
static gpiod::line hdd1ExistLine;
static boost::asio::posix::stream_descriptor hdd1ExistEvent(io);
static gpiod::line solUartV2Line;
static gpiod::line solUartV3Line;
static gpiod::line frontVgaExiLine;
static boost::asio::posix::stream_descriptor frontVgaExistEvent(io);
static gpiod::line fan9ExistLine;
static boost::asio::posix::stream_descriptor fan9ExistEvent(io);
static gpiod::line fan7ExistLine;
static boost::asio::posix::stream_descriptor fan7ExistEvent(io);
static gpiod::line hdd2ExistLine;
static boost::asio::posix::stream_descriptor hdd2ExistEvent(io);
static gpiod::line uidButtonLine;
static boost::asio::posix::stream_descriptor uidButtonEvent(io);
static gpiod::line hdd3ExistLine;
static boost::asio::posix::stream_descriptor hdd3ExistEvent(io);
static gpiod::line phyIrqLine;
static boost::asio::posix::stream_descriptor phyIrqEvent(io);
static gpiod::line hdd4ExistLine;
static boost::asio::posix::stream_descriptor hdd4ExistEvent(io);
static gpiod::line fan1PowerEnLine;
static boost::asio::posix::stream_descriptor fan1PowerEnEvent(io);
static gpiod::line fan2PowerEnLine;
static boost::asio::posix::stream_descriptor fan2PowerEnEvent(io);
static gpiod::line fan3PowerEnLine;
static boost::asio::posix::stream_descriptor fan3PowerEnEvent(io);
static gpiod::line fan4PowerEnLine;
static boost::asio::posix::stream_descriptor fan4PowerEnEvent(io);
static gpiod::line fan5PowerEnLine;
static boost::asio::posix::stream_descriptor fan5PowerEnEvent(io);
static gpiod::line fan6PowerEnLine;
static boost::asio::posix::stream_descriptor fan6PowerEnEvent(io);
static gpiod::line fan7PowerEnLine;
static boost::asio::posix::stream_descriptor fan7PowerEnEvent(io);
static gpiod::line fan8PowerEnLine;
static boost::asio::posix::stream_descriptor fan8PowerEnEvent(io);
static gpiod::line fan9PowerEnLine;
static boost::asio::posix::stream_descriptor fan9PowerEnEvent(io);
static gpiod::line cpuaFlashH0Line;
static boost::asio::posix::stream_descriptor cpuaFlashH0Event(io);
static gpiod::line cpuaFlashH1Line;
static boost::asio::posix::stream_descriptor cpuaFlashH1Event(io);
static gpiod::line cpubFlashH2Line;
static boost::asio::posix::stream_descriptor cpubFlashH2Event(io);
static gpiod::line cpubFlashH3Line;
static boost::asio::posix::stream_descriptor cpubFlashH3Event(io);
static gpiod::line identifyLedGetLine;
static boost::asio::posix::stream_descriptor identifyLedGetEvent(io);
static gpiod::line identifyLedSetLine;

static gpiod::line hostWdtLine;
static boost::asio::posix::stream_descriptor hostWdtEvent(io);


enum class SetCpldPowerState
{
    off,
    on,
    clear,
};


static bool setGPIOOutput(const std::string& name, const int value,
                          gpiod::line& gpioLine)
{
    // Find the GPIO line
    gpioLine = gpiod::find_line(name);
    if (!gpioLine)
    {
        lg2::error("Failed to find the {GPIO_NAME} line", "GPIO_NAME", name);
        return false;
    }

    // Request GPIO output to specified value
    try
    {
        gpioLine.request({appName, gpiod::line_request::DIRECTION_OUTPUT, {}},
                         value);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to request {GPIO_NAME} output: {ERROR}", "GPIO_NAME",
                   name, "ERROR", e);
        return false;
    }

    lg2::info("{GPIO_NAME} set to {GPIO_VALUE}", "GPIO_NAME", name,
              "GPIO_VALUE", value);


    gpioLine.release();

    return true;
}

static bool setGPIOOutputNotRelease(const std::string& name, const int value,
                          gpiod::line& gpioLine)
{
    // Find the GPIO line
    gpioLine = gpiod::find_line(name);
    if (!gpioLine)
    {
        lg2::error("Failed to find the {GPIO_NAME} line", "GPIO_NAME", name);
        return false;
    }

    // Request GPIO output to specified value
    try
    {
        gpioLine.request({appName, gpiod::line_request::DIRECTION_OUTPUT, {}},
                         value);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to request {GPIO_NAME} output: {ERROR}", "GPIO_NAME",
                   name, "ERROR", e);
        return false;
    }

    lg2::info("{GPIO_NAME} set to {GPIO_VALUE}", "GPIO_NAME", name,
              "GPIO_VALUE", value);

    return true;
}


static int loadConfigValues()
{
    const std::string configFilePath = "/usr/share/sophgo-gpio-control/gpio-config.json";
    std::ifstream configFile(configFilePath.c_str());
    if (!configFile.is_open())
    {
        lg2::error("loadConfigValues: Cannot open config path \'{PATH}\'",
                   "PATH", configFilePath);
        return -1;
    }
    auto jsonData = nlohmann::json::parse(configFile, nullptr, true, true);

    if (jsonData.is_discarded())
    {
        lg2::error("Power config readings JSON parser failure");
        return -1;
    }
    auto gpios    = jsonData["gpio_configs"];
    auto timers   = jsonData["timing_configs"];

    ConfigData* tempGpioData;

    for (nlohmann::json& gpioConfig : gpios)
    {
        if (!gpioConfig.contains("Name"))
        {
            lg2::error("The 'Name' field must be defined in Json file");
            return -1;
        }

        // Iterate through the powersignal map to check if the gpio json config
        // entry is valid
        std::string gpioName = gpioConfig["Name"];
        auto signalMapIter = powerSignalMap.find(gpioName);
        if (signalMapIter == powerSignalMap.end())
        {
            lg2::error(
                "{GPIO_NAME} is not a recognized power-control signal name",
                "GPIO_NAME", gpioName);
            return -1;
        }

        // assign the power signal name to the corresponding structure reference
        // from map then fillup the structure with coressponding json config
        // value
        tempGpioData = signalMapIter->second;
        tempGpioData->name = gpioName;

        if (!gpioConfig.contains("Type"))
        {
            lg2::error("The \'Type\' field must be defined in Json file");
            return -1;
        }

        std::string signalType = gpioConfig["Type"];
        if (signalType == "GPIO")
        {
            tempGpioData->type = ConfigType::GPIO;
        }
        else if (signalType == "DBUS")
        {
            tempGpioData->type = ConfigType::DBUS;
        }
        else
        {
            lg2::error("{TYPE} is not a recognized power-control signal type",
                       "TYPE", signalType);
            return -1;
        }

        if (tempGpioData->type == ConfigType::GPIO)
        {
            if (gpioConfig.contains("LineName"))
            {
                tempGpioData->lineName = gpioConfig["LineName"];
            }
            else
            {
                lg2::error(
                    "The \'LineName\' field must be defined for GPIO configuration");
                return -1;
            }
            if (gpioConfig.contains("Polarity"))
            {
                std::string polarity = gpioConfig["Polarity"];
                if (polarity == "ActiveLow")
                {
                    tempGpioData->polarity = false;
                }
                else if (polarity == "ActiveHigh")
                {
                    tempGpioData->polarity = true;
                }
                else
                {
                    lg2::error(
                        "Polarity defined but not properly setup. Please only ActiveHigh or ActiveLow. Currently set to {POLARITY}",
                        "POLARITY", polarity);
                    return -1;
                }
            }
            else
            {
                lg2::error("Polarity field not found for {GPIO_NAME}",
                           "GPIO_NAME", tempGpioData->lineName);
                return -1;
            }
        }
        else
        {
            // if dbus based gpio config is defined read and update the dbus
            // params corresponding to the gpio config instance
            for (auto& [key, dbusParamName] : dbusParams)
            {
                if (!gpioConfig.contains(dbusParamName))
                {
                    lg2::error(
                        "The {DBUS_NAME} field must be defined for Dbus configuration ",
                        "DBUS_NAME", dbusParamName);
                    return -1;
                }
            }
            tempGpioData->dbusName =
                gpioConfig[dbusParams[DbusConfigType::name]];
            tempGpioData->path = gpioConfig[dbusParams[DbusConfigType::path]];
            tempGpioData->interface =
                gpioConfig[dbusParams[DbusConfigType::interface]];
            tempGpioData->lineName =
                gpioConfig[dbusParams[DbusConfigType::property]];
        }
    }

     // read and store the timer values from json config to Timer Map
    for (auto& [key, timerValue] : TimerMap)
    {
        if (timers.contains(key.c_str()))
        {
            timerValue = timers[key.c_str()];
        }
    }

    return 0;
}




static int checkGpioLineName()
{
    if ((sata1ExistConfig.lineName.empty())          || \
       (sata2ExistConfig.lineName.empty())           || \
       (fan4ExistConfig.lineName.empty())            || \
       (fan5ExistConfig.lineName.empty())            || \
       (fan6ExistConfig.lineName.empty())            || \
       (fan8ExistConfig.lineName.empty())            || \
       (fan3ExistConfig.lineName.empty())            || \
       (fan1ExistConfig.lineName.empty())            || \
       (PSUPowerOnConfig.lineName.empty())           || \
       (fan2ExistConfig.lineName.empty())            || \
       (riserCardThrottleConfig.lineName.empty())    || \
       (hdd1ExistConfig.lineName.empty())            || \
       (solUartV2Config.lineName.empty())            || \
       (solUartV3Config.lineName.empty())            || \
       (frontVgaExitConfig.lineName.empty())         || \
       (fan9ExistConfig.lineName.empty())            || \
       (fan7ExistConfig.lineName.empty())            || \
       (hdd2ExistConfig.lineName.empty())            || \
       (uidButtonConfig.lineName.empty())            || \
       (hdd3ExistConfig.lineName.empty())            || \
       (phyIrqConfig.lineName.empty())               || \
       (hdd4ExistConfig.lineName.empty())            || \
       (fan1PowerEnConfig.lineName.empty())          || \
       (fan2PowerEnConfig.lineName.empty())          || \
       (fan3PowerEnConfig.lineName.empty())          || \
       (fan4PowerEnConfig.lineName.empty())          || \
       (fan5PowerEnConfig.lineName.empty())          || \
       (fan6PowerEnConfig.lineName.empty())          || \
       (fan7PowerEnConfig.lineName.empty())          || \
       (fan8PowerEnConfig.lineName.empty())          || \
       (fan9PowerEnConfig.lineName.empty())          || \
       (cpuaFlashH0Config.lineName.empty())          || \
       (cpuaFlashH1Config.lineName.empty())          || \
       (cpubFlashH2Config.lineName.empty())          || \
       (cpubFlashH3Config.lineName.empty())          || \
       (identifyLedGetConfig.lineName.empty())       || \
       (identifyLedSetConfig.lineName.empty())       || \
       (hostWdtConfig.lineName.empty()))
    {
        return -1;
    }

    return 0;


}




void forcePowerRestart(const boost::system::error_code& ec)
{

    if (ec == boost::asio::error::operation_aborted) {
        std::cout << "Wdt timer is canceled!" << std::endl;
        return;
    }

    conn->async_method_call(
        [](const boost::system::error_code ec) {
            if (ec)
            {
                std::cout << "Force power off D-Bus responses error: " << std::endl;
                return;
            }
            std::cout << "Force power off" << std::endl;
            first_wdt_event_flag.store(false);
        },
        power_control::busname,
        power_control::path,
        properties::interface, properties::set,
        power_control::interface,
        power_control::property,
        dbus::utility::DbusVariantType{power_control::forceRestart});

}



void set_wdt_timer(int sec)
{
    hostWdtTimer.expires_from_now( boost::posix_time::seconds(sec));
    hostWdtTimer.async_wait(forcePowerRestart);
}

int cancel_auto_timer(void)
{
    boost::system::error_code ec;
    hostWdtTimer.cancel(ec);
    // 检查是否发生错误
    if (ec) {
        // 处理错误
        std::cerr << "Error on canceling the timer: " << ec.message() << std::endl;
        return -1;
    } else {
        // 定时器取消成功
        std::cout << "Wdt timer cancelled successfully." << std::endl;
        return 0;
    }
}



static void waitForGPIOEvent(const std::string& name,
                             const std::function<void(bool)>& eventHandler,
                             gpiod::line& line,
                             boost::asio::posix::stream_descriptor& event)
{
    event.async_wait(boost::asio::posix::stream_descriptor::wait_read,
                     [&name, eventHandler, &line,
                      &event](const boost::system::error_code ec) {
                         if (ec)
                         {
                             lg2::error(
                                 "{GPIO_NAME} fd handler error: {ERROR_MSG}",
                                 "GPIO_NAME", name, "ERROR_MSG", ec.message());
                             // TODO: throw here to force power-control to
                             // restart?
                             return;
                         }
                         gpiod::line_event line_event = line.event_read();
                         eventHandler(line_event.event_type ==
                                      gpiod::line_event::RISING_EDGE);
                         waitForGPIOEvent(name, eventHandler, line, event);
                     });
}

static bool requestGPIOEvents(
    const std::string& name, const std::function<void(bool)>& handler,
    gpiod::line& gpioLine,
    boost::asio::posix::stream_descriptor& gpioEventDescriptor)
{
    // Find the GPIO line
    lg2::info("requestGPIOEvents : {GPIO_NAME} ", "GPIO_NAME", name);
    gpioLine = gpiod::find_line(name);
    if (!gpioLine)
    {
        lg2::error("Failed to find the {GPIO_NAME} line", "GPIO_NAME", name);
        return false;
    }

    try
    {
        gpioLine.request({appName, gpiod::line_request::EVENT_BOTH_EDGES, {}});
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed to request events for {GPIO_NAME}: {ERROR}",
                   "GPIO_NAME", name, "ERROR", e);
        return false;
    }

    int gpioLineFd = gpioLine.event_get_fd();
    if (gpioLineFd < 0)
    {
        lg2::error("Failed to get {GPIO_NAME} fd", "GPIO_NAME", name);
        return false;
    }

    gpioEventDescriptor.assign(gpioLineFd);

    waitForGPIOEvent(name, handler, gpioLine, gpioEventDescriptor);
    return true;
}

static void sata1ExistHandler(bool state)
{
    sataExistIface->set_property("sata1ExistState", !state);
}
static void sata2ExistHandler(bool state)
{
    sataExistIface->set_property("sata2ExistState", !state);
}
static void fan1ExistHandler(bool state)
{
    fanExistIface->set_property("fan1ExistState", !state);
}
static void fan2ExistHandler(bool state)
{
    fanExistIface->set_property("fan2ExistState", !state);
}
static void fan3ExistHandler(bool state)
{
    fanExistIface->set_property("fan3ExistState", !state);
}
static void fan4ExistHandler(bool state)
{
    fanExistIface->set_property("fan4ExistState", !state);
}
static void fan5ExistHandler(bool state)
{
    fanExistIface->set_property("fan5ExistState", !state);
}
static void fan6ExistHandler(bool state)
{
    fanExistIface->set_property("fan6ExistState", !state);
}
static void fan7ExistHandler(bool state)
{
    fanExistIface->set_property("fan7ExistState", !state);
}
static void fan8ExistHandler(bool state)
{
    fanExistIface->set_property("fan8ExistState", !state);
}
static void fan9ExistHandler(bool state)
{
    fanExistIface->set_property("fan9ExistState", !state);
}

static void hdd1ExistHandler(bool state)
{
    hddExistIface->set_property("hdd1ExistState", !state);
}
static void hdd2ExistHandler(bool state)
{
    hddExistIface->set_property("hdd2ExistState", !state);
}
static void hdd3ExistHandler(bool state)
{
    hddExistIface->set_property("hdd3ExistState", !state);
}

static void hdd4ExistHandler(bool state)
{
    hddExistIface->set_property("hdd4ExistState", !state);
}

static void identifyLedStateHandler(bool state)
{
    identifyLedIface->set_property("identifyLedState", state);
}


static void hostWdtHandler(bool state)
{
#if 0
    if (state) {
        if (first_wdt_event_flag.load()) {
            //update timer
            set_wdt_timer(WDT_DELAY);
            std::cout << "Wdt signal" << std::endl;
        } else {
            first_wdt_event_flag.store(true);
            //enable timer
            set_wdt_timer(WDT_DELAY);
            std::cout << "First wdt signal" << std::endl;
        }
    }
#else
    std::cout << "Wdt signal" << std::endl;
#endif
}

enum class SolUartPort
{
    CPU0_UART0,
    CPU0_UART1,
    CPU1_UART0,
    CPU1_UART1,
};

static void transitionSolUarttPort(SolUartPort port)
{
    switch (port)
    {
        case SolUartPort::CPU0_UART0:
            setGPIOOutput(solUartV2Config.lineName,0,solUartV2Line);
            setGPIOOutput(solUartV3Config.lineName,0,solUartV3Line);
            break;
        case SolUartPort::CPU0_UART1:
            setGPIOOutput(solUartV2Config.lineName,0,solUartV2Line);
            setGPIOOutput(solUartV3Config.lineName,1,solUartV3Line);
            break;
        case SolUartPort::CPU1_UART0:
            setGPIOOutput(solUartV2Config.lineName,1,solUartV2Line);
            setGPIOOutput(solUartV3Config.lineName,0,solUartV3Line);
            break;
        case SolUartPort::CPU1_UART1:
            setGPIOOutput(solUartV2Config.lineName,1,solUartV2Line);
            setGPIOOutput(solUartV3Config.lineName,1,solUartV3Line);
            break;
        default:
            break;
    }
}


enum class CpuFlashPort
{
    FLASH_TO_HOST,
    FLASH_TO_BMC,
};

static int setGPIOOutputForMs(const ConfigData& config, const int value,
                              const int durationMs,gpiod::line gpioLine)
{
    ;
    if (!setGPIOOutputNotRelease(config.lineName, value, gpioLine))
    {
        return -1;
    }
    const std::string name = config.lineName;

    gpioAssertTimer.expires_after(std::chrono::milliseconds(durationMs));
    gpioAssertTimer.async_wait(
        [gpioLine, value, name](const boost::system::error_code ec) {
            // Set the GPIO line back to the opposite value
            gpioLine.set_value(!value);
            lg2::info("{GPIO_NAME} released", "GPIO_NAME", name);
            if (ec)
            {
                // operation_aborted is expected if timer is canceled before
                // completion.
                if (ec != boost::asio::error::operation_aborted)
                {
                    lg2::error("{GPIO_NAME} async_wait failed: {ERROR_MSG}",
                               "GPIO_NAME", name, "ERROR_MSG", ec.message());
                }
            }
        });
    return 0;
}

static void switchIdentifyLed()
{
    setGPIOOutputForMs(identifyLedSetConfig, IdentifyLedSet::ON, TimerMap["LedSwitchPullMs"], identifyLedSetLine);
}

#ifdef FLASH_CONTROL_VIA_DBUS
static void transitionCpuAFlashPort(CpuFlashPort port)
{
    switch (port)
    {
        case CpuFlashPort::FLASH_TO_HOST:
            setGPIOOutput(cpuaFlashH0Config.lineName,1,cpuaFlashH0Line);
            setGPIOOutput(cpuaFlashH1Config.lineName,1,cpuaFlashH1Line);
            break;
        case CpuFlashPort::FLASH_TO_BMC:
            setGPIOOutput(cpuaFlashH0Config.lineName,0,cpuaFlashH0Line);
            setGPIOOutput(cpuaFlashH1Config.lineName,0,cpuaFlashH1Line);
            break;
        default:
            break;
    }
}
static void transitionCpuBFlashPort(CpuFlashPort port)
{
    switch (port)
    {
        case CpuFlashPort::FLASH_TO_HOST:
            setGPIOOutput(cpubFlashH2Config.lineName,1,cpubFlashH2Line);
            setGPIOOutput(cpubFlashH3Config.lineName,1,cpubFlashH3Line);
            break;
        case CpuFlashPort::FLASH_TO_BMC:
            setGPIOOutput(cpubFlashH2Config.lineName,0,cpubFlashH2Line);
            setGPIOOutput(cpubFlashH3Config.lineName,0,cpubFlashH3Line);
            break;
        default:
            break;
    }
}
#endif

} // namespace gpio_control





int main(int argc, char* argv[])
{
    using namespace gpio_control;
    using namespace boost::placeholders;


    // Load GPIO's through json config file
    if (loadConfigValues() == -1)
    {
        lg2::error(" Error in Parsing...");
        return -1;
    }
    //check gpio lineName
    if (checkGpioLineName() == -1)
    {
        lg2::error(" MIssing gpio line name ...");
        return -1;
    }

    // Request the dbus names
    conn = std::make_shared<sdbusplus::asio::connection>(io);
    // auto gpioBus = std::make_shared<sdbusplus::asio::connection>(io);
    conn->request_name(gpioDbusName.c_str());
    sdbusplus::asio::object_server objectServer(conn);
    sataExistIface  = objectServer.add_interface("/xyz/openbmc_project/gpio/sata", "xyz.openbmc_project.Gpio.Sata");
    hddExistIface   = objectServer.add_interface("/xyz/openbmc_project/gpio/hdd", "xyz.openbmc_project.Gpio.Hdd");
    fanExistIface   = objectServer.add_interface("/xyz/openbmc_project/gpio/fan", "xyz.openbmc_project.Gpio.Fan");
    riserCardIface  = objectServer.add_interface("/xyz/openbmc_project/gpio/riser", "xyz.openbmc_project.Gpio.RiserCard");
    PSUPowerOnIface = objectServer.add_interface("/xyz/openbmc_project/gpio/psupower", "xyz.openbmc_project.Gpio.PsuPOwerOn");
    solUartIface    = objectServer.add_interface("/xyz/openbmc_project/gpio/soluart", "xyz.openbmc_project.Gpio.SolUart");
#ifdef FLASH_CONTROL_VIA_DBUS
    cpuaFlashIface  = objectServer.add_interface("/xyz/openbmc_project/gpio/cpuaflash", "xyz.openbmc_project.Gpio.CpuA");
    cpubFlashIface  = objectServer.add_interface("/xyz/openbmc_project/gpio/cpubflash", "xyz.openbmc_project.Gpio.CpuB");
#endif
    identifyLedIface = objectServer.add_interface("/xyz/openbmc_project/gpio/identifyLed", "xyz.openbmc_project.Gpio.identifyLed");

    // Request GPIO events 初始化过程中先获取一次在位状态
    if (!requestGPIOEvents(sata1ExistConfig.lineName, sata1ExistHandler,
                            sata1ExistLine, sata1ExistEvent))
    {
        return -1;
    }
    sataExistIface->register_property("sata1ExistState",
                    (sata1ExistLine.get_value() == sata1ExistConfig.polarity));


    if (!requestGPIOEvents(sata2ExistConfig.lineName, sata2ExistHandler,
                            sata2ExisLine, sata2ExisEvent))
    {
        return -1;
    }
    sataExistIface->register_property("sata2ExistState",
                    (sata2ExisLine.get_value() == sata2ExistConfig.polarity));


    if (!requestGPIOEvents(fan1ExistConfig.lineName, fan1ExistHandler,
                            fan1ExistLine, fan1ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan1ExistState",
                    (fan1ExistLine.get_value() == fan1ExistConfig.polarity));


    if (!requestGPIOEvents(fan2ExistConfig.lineName, fan2ExistHandler,
                            fan2ExistLine, fan2ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan2ExistState",
                    (fan2ExistLine.get_value() == fan2ExistConfig.polarity));

    if (!requestGPIOEvents(fan3ExistConfig.lineName, fan3ExistHandler,
                            fan3ExistLine, fan3ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan3ExistState",
                    (fan3ExistLine.get_value() == fan3ExistConfig.polarity));



    if (!requestGPIOEvents(fan4ExistConfig.lineName, fan4ExistHandler,
                            fan4ExistLine, fan4ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan4ExistState",
                    (fan4ExistLine.get_value() == fan4ExistConfig.polarity));



    if (!requestGPIOEvents(fan5ExistConfig.lineName, fan5ExistHandler,
                            fan5ExistLine, fan5ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan5ExistState",
                    (fan5ExistLine.get_value() == fan5ExistConfig.polarity));



    if (!requestGPIOEvents(fan6ExistConfig.lineName, fan6ExistHandler,
                            fan6ExistLine, fan6ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan6ExistState",
                    (fan6ExistLine.get_value() == fan6ExistConfig.polarity));



    if (!requestGPIOEvents(fan7ExistConfig.lineName, fan7ExistHandler,
                            fan7ExistLine, fan7ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan7ExistState",
                    (fan7ExistLine.get_value() == fan7ExistConfig.polarity));



    if (!requestGPIOEvents(fan8ExistConfig.lineName, fan8ExistHandler,
                            fan8ExistLine, fan8ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan8ExistState",
                    (fan8ExistLine.get_value() == fan8ExistConfig.polarity));



    if (!requestGPIOEvents(fan9ExistConfig.lineName, fan9ExistHandler,
                            fan9ExistLine, fan9ExistEvent))
    {
        return -1;
    }
    fanExistIface->register_property("fan9ExistState",
                    (fan9ExistLine.get_value() == fan9ExistConfig.polarity));


    if (!requestGPIOEvents(hdd1ExistConfig.lineName, hdd1ExistHandler,
                            hdd1ExistLine, hdd1ExistEvent))
    {
        return -1;
    }
    hddExistIface->register_property("hdd1ExistState",
                    (hdd1ExistLine.get_value() == hdd1ExistConfig.polarity));



    if (!requestGPIOEvents(hdd2ExistConfig.lineName, hdd2ExistHandler,
                            hdd2ExistLine, hdd2ExistEvent))
    {
        return -1;
    }
    hddExistIface->register_property("hdd2ExistState",
                    (hdd2ExistLine.get_value() == hdd2ExistConfig.polarity));


    if (!requestGPIOEvents(hdd3ExistConfig.lineName, hdd3ExistHandler,
                            hdd3ExistLine, hdd3ExistEvent))
    {
        return -1;
    }
    hddExistIface->register_property("hdd3ExistState",
                    (hdd3ExistLine.get_value() == hdd3ExistConfig.polarity));




    if (!requestGPIOEvents(hdd4ExistConfig.lineName, hdd4ExistHandler,
                            hdd4ExistLine, hdd4ExistEvent))
    {
        return -1;
    }
    hddExistIface->register_property("hdd4ExistState",
                    (hdd4ExistLine.get_value() == hdd4ExistConfig.polarity));


    //identifyLed
    if (!requestGPIOEvents(identifyLedGetConfig.lineName, identifyLedStateHandler,
                            identifyLedGetLine, identifyLedGetEvent))
    {
        return -1;
    }


    if (!requestGPIOEvents(hostWdtConfig.lineName, hostWdtHandler,
                            hostWdtLine, hostWdtEvent))
    {
        return -1;
    }

    identifyLedIface->register_property("identifyLedState",
                    (identifyLedGetLine.get_value() == identifyLedGetConfig.polarity));



    //Initialize out gpio
    if(!setGPIOOutput(solUartV2Config.lineName, 0/* (SolPort::H0U0 >> 1) & 1 */, solUartV2Line))
    {
        return -1;
    }
    if(!setGPIOOutput(solUartV3Config.lineName, 0/* (SolPort::H0U0 & 1) */, solUartV3Line))
    {
        return -1;
    }
    if(!setGPIOOutput(cpuaFlashH0Config.lineName, HostFlashPort::HOST, cpuaFlashH0Line))
    {
        return -1;
    }
    if(!setGPIOOutput(cpuaFlashH1Config.lineName, HostFlashPort::HOST, cpuaFlashH1Line))
    {
        return -1;
    }
    if(!setGPIOOutput(cpubFlashH2Config.lineName, HostFlashPort::HOST, cpubFlashH2Line))
    {
        return -1;
    }
    if(!setGPIOOutput(cpubFlashH3Config.lineName, HostFlashPort::HOST, cpubFlashH3Line))
    {
        return -1;
    }
    if(!setGPIOOutput(identifyLedSetConfig.lineName, IdentifyLedSet::OFF, identifyLedSetLine))
    {
        return -1;
    }


    // Request DBUS events
    solUartIface->register_property(
        "SolUartPortTransition",
        std::string("CPU0UART0"),
        [](const std::string& requested, std::string& resp) {
            if (requested == "CPU0UART0")
            {
                transitionSolUarttPort(SolUartPort::CPU0_UART0);
            }
            else if (requested == "CPU0UART1")
            {
                transitionSolUarttPort(SolUartPort::CPU0_UART1);
            }
            else if (requested == "CPU1UART0")
            {
                transitionSolUarttPort(SolUartPort::CPU1_UART0);
            }
            else if (requested == "CPU1UART1")
            {
                transitionSolUarttPort(SolUartPort::CPU1_UART1);
            }
            resp = requested;
            return 1;
        });

    // identifyLedIface->register_method("identifyLedSwitch", switchIdentifyLed);
    identifyLedIface->register_property(
        "identifyLedSwitch",
        bool(1),
        [](const bool requested, bool resp) {
            switchIdentifyLed();
            resp = requested;
            return 1;
        });



#ifdef FLASH_CONTROL_VIA_DBUS
    cpuaFlashIface->register_property(
        "CpuAFlashTransition",
        std::string("xyz.openbmc_project.Gpio.CpuAFlash.Port.Host"),
        [](const std::string& requested, std::string& resp) {
            if (requested == "xyz.openbmc_project.Gpio.CpuAFlash.Port.Host")
            {
                transitionCpuAFlashPort(CpuFlashPort::FLASH_TO_HOST);
            }
            else if (requested == "xyz.openbmc_project.Gpio.CpuAFlash.Port.Bmc")
            {
                transitionCpuAFlashPort(CpuFlashPort::FLASH_TO_BMC);
            }
            resp = requested;
            return 1;
        });
    cpubFlashIface->register_property(
        "CpuBFlashTransition",
        std::string("xyz.openbmc_project.Gpio.CpuBFlash.Port.Host"),
        [](const std::string& requested, std::string& resp) {
            if (requested == "xyz.openbmc_project.Gpio.CpuBFlash.Port.Host")
            {
                transitionCpuBFlashPort(CpuFlashPort::FLASH_TO_HOST);
            }
            else if (requested == "xyz.openbmc_project.Gpio.CpuBFlash.Port.Bmc")
            {
                transitionCpuBFlashPort(CpuFlashPort::FLASH_TO_BMC);
            }
            resp = requested;
            return 1;
        });
#endif


    sataExistIface->initialize();
    hddExistIface->initialize();
    fanExistIface->initialize();
    riserCardIface->initialize();
    PSUPowerOnIface->initialize();
    solUartIface->initialize();
    identifyLedIface->initialize();
#ifdef FLASH_CONTROL_VIA_DBUS
    cpuaFlashIface->initialize();
    cpubFlashIface->initialize();
#endif

    io.run();

    return 0;
}
