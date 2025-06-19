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


static ConfigData cpu0_m_b_event_lvt_nConfig;
static ConfigData cpu0_m_a_event_lvt_nConfig;
static ConfigData id_buttonConfig;
static ConfigData cpu0_prochot_nConfig;
static ConfigData cpu0_memhot_nConfig;
static ConfigData cpu0_presentConfig;
static ConfigData vbat_gpio_ctlConfig;
static ConfigData bmc_present_nConfig;
static ConfigData post_completeConfig;


// map for storing list of gpio parameters whose config are to be read from sophgo
// gpio control json config
boost::container::flat_map<std::string, ConfigData*> powerSignalMap = {
    {"cpu0_m_b_event_lvt_n",          &cpu0_m_b_event_lvt_nConfig},
    {"cpu0_m_a_event_lvt_n",          &cpu0_m_a_event_lvt_nConfig},
    {"id_button",                     &id_buttonConfig},
    {"cpu0_prochot_n",                &cpu0_prochot_nConfig},
    {"cpu0_memhot_n",                 &cpu0_memhot_nConfig},
    {"cpu0_present",                  &cpu0_presentConfig},
    {"vbat_gpio_ctl",                 &vbat_gpio_ctlConfig},
    {"bmc_present_n",                 &bmc_present_nConfig},
    {"post_complete",                 &post_completeConfig}};



static std::string gpioDbusName = "xyz.openbmc_project.Gpio";
static std::shared_ptr<sdbusplus::asio::dbus_interface> cpu0_m_b_event_lvt_nIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> cpu0_m_a_event_lvt_nIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> id_buttonIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> cpu0_prochot_nIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> cpu0_memhot_nIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> cpu0_presentIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> vbat_gpio_ctlIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> bmc_present_nIface;//输出
static std::shared_ptr<sdbusplus::asio::dbus_interface> post_completeIface;//输出


// GPIO Lines and Event Descriptors
static gpiod::line cpu0_m_b_event_lvt_nLine;
static boost::asio::posix::stream_descriptor cpu0_m_b_event_lvt_nEvent(io);
static gpiod::line cpu0_m_a_event_lvt_nLine;
static boost::asio::posix::stream_descriptor cpu0_m_a_event_lvt_nEvent(io);
static gpiod::line id_buttonLine;
static boost::asio::posix::stream_descriptor id_buttonEvent(io);
static gpiod::line cpu0_prochot_nLine;
static boost::asio::posix::stream_descriptor cpu0_prochot_nEvent(io);
static gpiod::line cpu0_memhot_nLine;
static boost::asio::posix::stream_descriptor cpu0_memhot_nEvent(io);
static gpiod::line cpu0_presentLine;
static boost::asio::posix::stream_descriptor cpu0_presentEvent(io);
static gpiod::line vbat_gpio_ctlLine;
static gpiod::line bmc_present_nLine;
static boost::asio::posix::stream_descriptor bmc_present_nEvent(io);
static gpiod::line post_completeLine;
static boost::asio::posix::stream_descriptor post_completeEvent(io);


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
    const std::string configFilePath = "/usr/share/sophgo-se8-gpio-control/gpio-config.json";
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

    return 0;
}




static int checkGpioLineName()
{
    if ((cpu0_m_b_event_lvt_nConfig.lineName.empty())          || \
       (cpu0_m_a_event_lvt_nConfig.lineName.empty())           || \
       (id_buttonConfig.lineName.empty())            || \
       (cpu0_prochot_nConfig.lineName.empty())            || \
       (cpu0_memhot_nConfig.lineName.empty())            || \
       (cpu0_presentConfig.lineName.empty())            || \
       (vbat_gpio_ctlConfig.lineName.empty())            || \
       (bmc_present_nConfig.lineName.empty())            || \
       (post_completeConfig.lineName.empty()))
    {
        return -1;
    }

    return 0;


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

static void cpu0_m_b_event_lvt_nHandler(bool state)
{
    cpu0_m_b_event_lvt_nIface->set_property("cpu0_m_b_event_lvt_nState", !state);
}
static void cpu0_m_a_event_lvt_nHandler(bool state)
{
    cpu0_m_a_event_lvt_nIface->set_property("cpu0_m_a_event_lvt_nState", !state);
}
static void id_buttonHandler(bool state)
{
    id_buttonIface->set_property("id_buttonState", !state);
}
static void cpu0_prochot_nHandler(bool state)
{
    cpu0_prochot_nIface->set_property("cpu0_prochot_nState", !state);
}
static void cpu0_memhot_nHandler(bool state)
{
    cpu0_memhot_nIface->set_property("cpu0_memhot_nState", !state);
}
static void cpu0_presentHandler(bool state)
{
    cpu0_presentIface->set_property("cpu0_presentState", !state);
}
static void bmc_present_nHandler(bool state)
{
    bmc_present_nIface->set_property("bmc_present_nState", !state);
}
static void post_completeHandler(bool state)
{
    post_completeIface->set_property("post_completeState", !state);
}

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
    cpu0_m_b_event_lvt_nIface  = objectServer.add_interface("/xyz/openbmc_project/gpio/cpu0_m_b_event_lvt_n", "xyz.openbmc_project.Gpio.cpu0_m_b_event_lvt_n");
    cpu0_m_a_event_lvt_nIface   = objectServer.add_interface("/xyz/openbmc_project/gpio/cpu0_m_a_event_lvt_n", "xyz.openbmc_project.Gpio.cpu0_m_a_event_lvt_n");
    id_buttonIface   = objectServer.add_interface("/xyz/openbmc_project/gpio/id_button", "xyz.openbmc_project.Gpio.id_button");
    cpu0_prochot_nIface  = objectServer.add_interface("/xyz/openbmc_project/gpio/cpu0_prochot_n", "xyz.openbmc_project.Gpio.cpu0_prochot_n");
    cpu0_memhot_nIface = objectServer.add_interface("/xyz/openbmc_project/gpio/cpu0_memhot_n", "xyz.openbmc_project.Gpio.cpu0_memhot_n");
    cpu0_presentIface    = objectServer.add_interface("/xyz/openbmc_project/gpio/cpu0_present", "xyz.openbmc_project.Gpio.cpu0_present");
    vbat_gpio_ctlIface   = objectServer.add_interface("/xyz/openbmc_project/gpio/vbat_gpio_ctl", "xyz.openbmc_project.Gpio.vbat_gpio_ctl");
    bmc_present_nIface   = objectServer.add_interface("/xyz/openbmc_project/gpio/bmc_present_n", "xyz.openbmc_project.Gpio.bmc_present_n");
    post_completeIface     = objectServer.add_interface("/xyz/openbmc_project/gpio/post_complete", "xyz.openbmc_project.Gpio.post_complete");

    // Request GPIO events 初始化过程中先获取一次在位状态
    if (!requestGPIOEvents(cpu0_m_b_event_lvt_nConfig.lineName, cpu0_m_b_event_lvt_nHandler,
                            cpu0_m_b_event_lvt_nLine, cpu0_m_b_event_lvt_nEvent))
    {
        return -1;
    }
    cpu0_m_b_event_lvt_nIface->register_property("cpu0_m_b_event_lvt_nState",
                    (cpu0_m_b_event_lvt_nLine.get_value() == cpu0_m_b_event_lvt_nConfig.polarity));


    if (!requestGPIOEvents(cpu0_m_a_event_lvt_nConfig.lineName, cpu0_m_a_event_lvt_nHandler,
                            cpu0_m_a_event_lvt_nLine, cpu0_m_a_event_lvt_nEvent))
    {
        return -1;
    }
    cpu0_m_a_event_lvt_nIface->register_property("cpu0_m_a_event_lvt_nState",
                    (cpu0_m_a_event_lvt_nLine.get_value() == cpu0_m_a_event_lvt_nConfig.polarity));


    if (!requestGPIOEvents(id_buttonConfig.lineName, id_buttonHandler,
                            id_buttonLine, id_buttonEvent))
    {
        return -1;
    }
    id_buttonIface->register_property("id_buttonState",
                    (id_buttonLine.get_value() == id_buttonConfig.polarity));

    if (!requestGPIOEvents(cpu0_prochot_nConfig.lineName, cpu0_prochot_nHandler,
                            cpu0_prochot_nLine, cpu0_prochot_nEvent))
    {
        return -1;
    }
    cpu0_prochot_nIface->register_property("cpu0_prochot_nState",
                    (cpu0_prochot_nLine.get_value() == cpu0_prochot_nConfig.polarity));

    if (!requestGPIOEvents(cpu0_memhot_nConfig.lineName, cpu0_memhot_nHandler,
                            cpu0_memhot_nLine, cpu0_memhot_nEvent))
    {
        return -1;
    }
    cpu0_memhot_nIface->register_property("cpu0_memhot_nState",
                    (cpu0_memhot_nLine.get_value() == cpu0_memhot_nConfig.polarity));



    if (!requestGPIOEvents(cpu0_presentConfig.lineName, cpu0_presentHandler,
                            cpu0_presentLine, cpu0_presentEvent))
    {
        return -1;
    }
    cpu0_presentIface->register_property("cpu0_presentState",
                    (cpu0_presentLine.get_value() == cpu0_presentConfig.polarity));



    if (!requestGPIOEvents(bmc_present_nConfig.lineName, bmc_present_nHandler,
                            bmc_present_nLine, bmc_present_nEvent))
    {
        return -1;
    }
    bmc_present_nIface->register_property("bmc_present_nState",
                    (bmc_present_nLine.get_value() == bmc_present_nConfig.polarity));



    if (!requestGPIOEvents(post_completeConfig.lineName, post_completeHandler,
                            post_completeLine, post_completeEvent))
    {
        return -1;
    }
    post_completeIface->register_property("post_completeState",
                    (post_completeLine.get_value() == post_completeConfig.polarity));

    if(!setGPIOOutput(vbat_gpio_ctlConfig.lineName, 1/* (SolPort::H0U0 >> 1) & 1 */, vbat_gpio_ctlLine))
    {
        return -1;
    }

    vbat_gpio_ctlIface->register_property(
        "vbat_gpio_ctlState",
        bool(1),
        [](const bool requested, bool resp) {
            setGPIOOutput(vbat_gpio_ctlConfig.lineName,1,vbat_gpio_ctlLine);
            resp = requested;
            return 1;
        });

    cpu0_m_b_event_lvt_nIface->initialize();
    cpu0_m_a_event_lvt_nIface->initialize();
    id_buttonIface->initialize();
    cpu0_prochot_nIface->initialize();
    cpu0_memhot_nIface->initialize();
    cpu0_presentIface->initialize();
    vbat_gpio_ctlIface->initialize();
    bmc_present_nIface->initialize();
    post_completeIface->initialize();

    io.run();

    return 0;
}
