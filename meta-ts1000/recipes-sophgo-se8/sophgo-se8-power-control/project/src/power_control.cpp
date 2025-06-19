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
#include "power_control.hpp"

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

#include <filesystem>
#include <fstream>
#include <string_view>

namespace power_control
{
static boost::asio::io_service io;
std::shared_ptr<sdbusplus::asio::connection> conn;
PersistentState appState;
PowerRestoreController powerRestore(io);

static std::string node = "0";
static const std::string appName = "power-control";

static uint32_t hopInitValue=0;

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
    DBUS
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

static ConfigData powerOutConfig;
static ConfigData powerOkConfig;
static ConfigData resetOutConfig;
static ConfigData resetButtonConfig;

// map for storing list of gpio parameters whose config are to be read from x86
// power control json config
boost::container::flat_map<std::string, ConfigData*> powerSignalMap = {
    {"PowerOut", &powerOutConfig},
    {"PowerOk", &powerOkConfig},
    {"ResetOut", &resetOutConfig},
    {"ResetButton", &resetButtonConfig}};

static std::string hostDbusName = "xyz.openbmc_project.State.Host";
static std::string chassisDbusName = "xyz.openbmc_project.State.Chassis";
static std::string osDbusName = "xyz.openbmc_project.State.OperatingSystem";
static std::string buttonDbusName = "xyz.openbmc_project.Chassis.Buttons";
static std::string rstCauseDbusName =
    "xyz.openbmc_project.Control.Host.RestartCause";
static std::shared_ptr<sdbusplus::asio::dbus_interface> hostIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> hostHopIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> chassisIface;

static std::shared_ptr<sdbusplus::asio::dbus_interface> powerButtonIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> resetButtonIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> nmiButtonIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> osIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> idButtonIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> nmiOutIface;
static std::shared_ptr<sdbusplus::asio::dbus_interface> restartCauseIface;

static gpiod::line powerButtonMask;
static gpiod::line resetButtonMask;
static bool nmiButtonMasked = false;
#if IGNORE_SOFT_RESETS_DURING_POST
static bool ignoreNextSoftReset = false;
#endif

// This map contains all timer values that are to be read from json config
boost::container::flat_map<std::string, int> TimerMap = {
    {"PowerOn0Ms", 50},
    {"PowerOn1Ms", 1000},
    {"PowerOn2Ms", 1000},
    {"PowerOff0Ms", 50},
    {"PowerOff1Ms", 50},
    {"ResetPulseMs", 500},
    {"PowerCycleMs", 3000},
    {"PsPowerOKWatchdogMs", 15000},
    {"PowerOffSaveMs", 7000}};


// Timers
// Time holding GPIOs asserted
static boost::asio::steady_timer gpioAssertTimer(io);
// Time between off and on during a power cycle
static boost::asio::steady_timer powerCycleTimer(io);
// Time OS gracefully powering off
static boost::asio::steady_timer gracefulPowerOffTimer(io);
// Time the warm reset check
static boost::asio::steady_timer warmResetCheckTimer(io);
// Time power supply power OK assertion on power-on
static boost::asio::steady_timer psPowerOKWatchdogTimer(io);
// Time SIO power good assertion on power-on
static boost::asio::steady_timer sioPowerGoodWatchdogTimer(io);
// Time power-off state save for power loss tracking
static boost::asio::steady_timer powerStateSaveTimer(io);
// POH timer
static boost::asio::steady_timer pohCounterTimer(io);
// Time when to allow restart cause updates
static boost::asio::steady_timer restartCauseTimer(io);
static boost::asio::steady_timer slotPowerCycleTimer(io);

// GPIO Lines and Event Descriptors
static gpiod::line psPowerOKLine;
static boost::asio::posix::stream_descriptor psPowerOKEvent(io);
static gpiod::line powerButtonLine;
static boost::asio::posix::stream_descriptor powerButtonEvent(io);
static gpiod::line resetButtonLine;
static boost::asio::posix::stream_descriptor resetButtonEvent(io);
static boost::asio::posix::stream_descriptor idButtonEvent(io);

static constexpr uint8_t beepPowerFail = 8;

static void beep(const uint8_t& beepPriority)
{
    lg2::info("Beep with priority: {BEEP_PRIORITY}", "BEEP_PRIORITY",
              beepPriority);

    conn->async_method_call(
        [](boost::system::error_code ec) {
            if (ec)
            {
                lg2::error(
                    "beep returned error with async_method_call (ec = {ERROR_MSG})",
                    "ERROR_MSG", ec.message());
                return;
            }
        },
        "xyz.openbmc_project.BeepCode", "/xyz/openbmc_project/BeepCode",
        "xyz.openbmc_project.BeepCode", "Beep", uint8_t(beepPriority));
}

enum class OperatingSystemStateStage
{
    Inactive,
    Standby,
};
static OperatingSystemStateStage operatingSystemState;
static constexpr std::string_view
    getOperatingSystemStateStage(const OperatingSystemStateStage stage)
{
    switch (stage)
    {
        case OperatingSystemStateStage::Inactive:
            return "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Inactive";
            break;
        case OperatingSystemStateStage::Standby:
            return "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Standby";
            break;
        default:
            return "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Inactive";
            break;
    }
};
static void setOperatingSystemState(const OperatingSystemStateStage stage)
{
    operatingSystemState = stage;
#if IGNORE_SOFT_RESETS_DURING_POST
    // If POST complete has asserted set ignoreNextSoftReset to false to avoid
    // masking soft resets after POST
    if (operatingSystemState == OperatingSystemStateStage::Standby)
    {
        ignoreNextSoftReset = false;
    }
#endif
    osIface->set_property("OperatingSystemState",
                          std::string(getOperatingSystemStateStage(stage)));

    lg2::info("Moving os state to {STATE} stage", "STATE",
              getOperatingSystemStateStage(stage));
}

enum class PowerState
{
    on,
    waitForPSPowerOK,
    off,
    transitionToOff,
    gracefulTransitionToOff,
    cycleOff,//power cycle
    transitionToCycleOff,//power cycle
};
static PowerState powerState;
static std::string getPowerStateName(PowerState state)
{
    switch (state)
    {
        case PowerState::on:
            return "On";
            break;
        case PowerState::waitForPSPowerOK:
            return "Wait for Power Supply Power OK";
            break;
        case PowerState::off:
            return "Off";
            break;
        case PowerState::transitionToOff:
            return "Transition to Off";
            break;
        case PowerState::gracefulTransitionToOff:
            return "Graceful Transition to Off";
            break;
        case PowerState::cycleOff:
            return "Power Cycle Off";
            break;
        case PowerState::transitionToCycleOff:
            return "Transition to Power Cycle Off";
            break;
        default:
            return "unknown state: " + std::to_string(static_cast<int>(state));
            break;
    }
}
static void logStateTransition(const PowerState state)
{
    lg2::info("Host{HOST}: Moving to \"{STATE}\" state", "HOST", node, "STATE",
              getPowerStateName(state));
}

enum class Event
{
    psPowerOKAssert,
    psPowerOKDeAssert,
    powerButtonPressed,
    resetButtonPressed,
    powerCycleTimerExpired,
    psPowerOKWatchdogTimerExpired,
    powerOnRequest,
    powerOffRequest,
    powerCycleRequest,
    resetRequest,
    gracefulPowerOffRequest,//se8 = force power off
};
static std::string getEventName(Event event)
{
    switch (event)
    {
        case Event::psPowerOKAssert:
            return "power supply power OK assert";
            break;
        case Event::psPowerOKDeAssert:
            return "power supply power OK de-assert";
            break;
        case Event::powerButtonPressed:
            return "power button pressed";
            break;
        case Event::resetButtonPressed:
            return "reset button pressed";
            break;
        case Event::powerCycleTimerExpired:
            return "power cycle timer expired";
            break;
        case Event::psPowerOKWatchdogTimerExpired:
            return "power supply power OK watchdog timer expired";
            break;
        case Event::powerOnRequest:
            return "power-on request";
            break;
        case Event::powerOffRequest:
            return "power-off request";
            break;
        case Event::powerCycleRequest:
            return "power-cycle request";
            break;
        case Event::resetRequest:
            return "reset request";
            break;
        case Event::gracefulPowerOffRequest:
            return "graceful power-off request";
            break;
        default:
            return "unknown event: " + std::to_string(static_cast<int>(event));
            break;
    }
}
static void logEvent(const std::string_view stateHandler, const Event event)
{
    lg2::info("{STATE_HANDLER}: {EVENT} event received", "STATE_HANDLER",
              stateHandler, "EVENT", getEventName(event));
}

// Power state handlers
static void powerStateOn(const Event event);
static void powerStateWaitForPSPowerOK(const Event event);
static void powerStateOff(const Event event);
static void powerStateTransitionToOff(const Event event);
static void powerStateGracefulTransitionToOff(const Event event);
static void powerStateCycleOff(const Event event);
static void powerStateTransitionToCycleOff(const Event event);
static void powerStateGracefulTransitionToCycleOff(const Event event);


static std::function<void(const Event)> getPowerStateHandler(PowerState state)
{
    switch (state)
    {
        case PowerState::on:
            return powerStateOn;
            break;
        case PowerState::waitForPSPowerOK:
            return powerStateWaitForPSPowerOK;
            break;
        case PowerState::off:
            return powerStateOff;
            break;
        case PowerState::transitionToOff:
            return powerStateTransitionToOff;
            break;
        case PowerState::gracefulTransitionToOff:
            return powerStateGracefulTransitionToOff;
            break;
        case PowerState::cycleOff:
            return powerStateCycleOff;
            break;
        case PowerState::transitionToCycleOff:
            return powerStateTransitionToCycleOff;
            break;

        default:
            return nullptr;
            break;
    }
};

static void sendPowerControlEvent(const Event event)
{
    std::function<void(const Event)> handler = getPowerStateHandler(powerState);
    if (handler == nullptr)
    {
        lg2::error("Failed to find handler for power state: {STATE}", "STATE",
                   static_cast<int>(powerState));
        return;
    }
    handler(event);
}

static uint64_t getCurrentTimeMs()
{
    struct timespec time = {};

    if (clock_gettime(CLOCK_REALTIME, &time) < 0)
    {
        return 0;
    }
    uint64_t currentTimeMs = static_cast<uint64_t>(time.tv_sec) * 1000;
    currentTimeMs += static_cast<uint64_t>(time.tv_nsec) / 1000 / 1000;

    return currentTimeMs;
}

static constexpr std::string_view getHostState(const PowerState state)
{
    switch (state)
    {
        case PowerState::on:
        case PowerState::gracefulTransitionToOff:
            return "xyz.openbmc_project.State.Host.HostState.Running";
            break;
        case PowerState::waitForPSPowerOK:
        case PowerState::off:
        case PowerState::transitionToOff:
        case PowerState::transitionToCycleOff:
        case PowerState::cycleOff:
            return "xyz.openbmc_project.State.Host.HostState.Off";
            break;
        default:
            return "";
            break;
    }
}
static constexpr std::string_view getChassisState(const PowerState state)
{
    switch (state)
    {
        case PowerState::on:
        case PowerState::transitionToOff:
        case PowerState::gracefulTransitionToOff:
        case PowerState::transitionToCycleOff:
            return "xyz.openbmc_project.State.Chassis.PowerState.On";
            break;
        case PowerState::waitForPSPowerOK:
        case PowerState::off:
        case PowerState::cycleOff:
            return "xyz.openbmc_project.State.Chassis.PowerState.Off";
            break;
        default:
            return "";
            break;
    }
};

static void savePowerState(const PowerState state)
{
    powerStateSaveTimer.expires_after(
        std::chrono::milliseconds(TimerMap["PowerOffSaveMs"]));
    powerStateSaveTimer.async_wait([state](const boost::system::error_code ec) {
        if (ec)
        {
            // operation_aborted is expected if timer is canceled before
            // completion.
            if (ec != boost::asio::error::operation_aborted)
            {
                lg2::error("Power-state save async_wait failed: {ERROR_MSG}",
                           "ERROR_MSG", ec.message());
            }
            return;
        }
        appState.set(PersistentState::Params::PowerState,
                     std::string{getChassisState(state)});
    });
}
static void setPowerState(const PowerState state)
{
    powerState = state;
    logStateTransition(state);

    hostIface->set_property("CurrentHostState",
                            std::string(getHostState(powerState)));

    chassisIface->set_property("CurrentPowerState",
                               std::string(getChassisState(powerState)));
    chassisIface->set_property("LastStateChangeTime", getCurrentTimeMs());

    // Save the power state for the restore policy
    savePowerState(state);
}

enum class RestartCause
{
    command,
    resetButton,
    powerButton,
    watchdog,
    powerPolicyOn,
    powerPolicyRestore,
    softReset,
};
static boost::container::flat_set<RestartCause> causeSet;
static std::string getRestartCause(RestartCause cause)
{
    switch (cause)
    {
        case RestartCause::command:
            return "xyz.openbmc_project.State.Host.RestartCause.IpmiCommand";
            break;
        case RestartCause::resetButton:
            return "xyz.openbmc_project.State.Host.RestartCause.ResetButton";
            break;
        case RestartCause::powerButton:
            return "xyz.openbmc_project.State.Host.RestartCause.PowerButton";
            break;
        case RestartCause::watchdog:
            return "xyz.openbmc_project.State.Host.RestartCause.WatchdogTimer";
            break;
        case RestartCause::powerPolicyOn:
            return "xyz.openbmc_project.State.Host.RestartCause.PowerPolicyAlwaysOn";
            break;
        case RestartCause::powerPolicyRestore:
            return "xyz.openbmc_project.State.Host.RestartCause.PowerPolicyPreviousState";
            break;
        case RestartCause::softReset:
            return "xyz.openbmc_project.State.Host.RestartCause.SoftReset";
            break;
        default:
            return "xyz.openbmc_project.State.Host.RestartCause.Unknown";
            break;
    }
}
static void addRestartCause(const RestartCause cause)
{
    // Add this to the set of causes for this restart
    causeSet.insert(cause);
}
static void clearRestartCause()
{
    // Clear the set for the next restart
    causeSet.clear();
}
static void setRestartCauseProperty(const std::string& cause)
{
    lg2::info("RestartCause set to {RESTART_CAUSE}", "RESTART_CAUSE", cause);
    restartCauseIface->set_property("RestartCause", cause);
}

#ifdef USE_ACBOOT
static void resetACBootProperty()
{
    if ((causeSet.contains(RestartCause::command)) ||
        (causeSet.contains(RestartCause::softReset)))
    {
        conn->async_method_call(
            [](boost::system::error_code ec) {
                if (ec)
                {
                    lg2::error("failed to reset ACBoot property");
                }
            },
            "xyz.openbmc_project.Settings",
            "/xyz/openbmc_project/control/host0/ac_boot",
            "org.freedesktop.DBus.Properties", "Set",
            "xyz.openbmc_project.Common.ACBoot", "ACBoot",
            std::variant<std::string>{"False"});
    }
}
#endif // USE_ACBOOT

static void setRestartCause()
{
    // Determine the actual restart cause based on the set of causes
    std::string restartCause =
        "xyz.openbmc_project.State.Host.RestartCause.Unknown";
    if (causeSet.contains(RestartCause::watchdog))
    {
        restartCause = getRestartCause(RestartCause::watchdog);
    }
    else if (causeSet.contains(RestartCause::command))
    {
        restartCause = getRestartCause(RestartCause::command);
    }
    else if (causeSet.contains(RestartCause::resetButton))
    {
        restartCause = getRestartCause(RestartCause::resetButton);
    }
    else if (causeSet.contains(RestartCause::powerButton))
    {
        restartCause = getRestartCause(RestartCause::powerButton);
    }
    else if (causeSet.contains(RestartCause::powerPolicyOn))
    {
        restartCause = getRestartCause(RestartCause::powerPolicyOn);
    }
    else if (causeSet.contains(RestartCause::powerPolicyRestore))
    {
        restartCause = getRestartCause(RestartCause::powerPolicyRestore);
    }
    else if (causeSet.contains(RestartCause::softReset))
    {
#if IGNORE_SOFT_RESETS_DURING_POST
        if (ignoreNextSoftReset)
        {
            ignoreNextSoftReset = false;
            return;
        }
#endif
        restartCause = getRestartCause(RestartCause::softReset);
    }

    setRestartCauseProperty(restartCause);
}


static void psPowerOKFailedLog()
{
    sd_journal_send(
        "MESSAGE=PowerControl: power supply power good failed to assert",
        "PRIORITY=%i", LOG_INFO, "REDFISH_MESSAGE_ID=%s",
        "OpenBMC.0.1.PowerSupplyPowerGoodFailed", "REDFISH_MESSAGE_ARGS=%d",
        TimerMap["PsPowerOKWatchdogMs"], NULL);
}

static void powerRestorePolicyLog()
{
    sd_journal_send("MESSAGE=PowerControl: power restore policy applied",
                    "PRIORITY=%i", LOG_INFO, "REDFISH_MESSAGE_ID=%s",
                    "OpenBMC.0.1.PowerRestorePolicyApplied", NULL);
}

static void powerButtonPressLog()
{
    sd_journal_send("MESSAGE=PowerControl: power button pressed", "PRIORITY=%i",
                    LOG_INFO, "REDFISH_MESSAGE_ID=%s",
                    "OpenBMC.0.1.PowerButtonPressed", NULL);
}

static void resetButtonPressLog()
{
    sd_journal_send("MESSAGE=PowerControl: reset button pressed", "PRIORITY=%i",
                    LOG_INFO, "REDFISH_MESSAGE_ID=%s",
                    "OpenBMC.0.1.ResetButtonPressed", NULL);
}

static void nmiButtonPressLog()
{
    sd_journal_send("MESSAGE=PowerControl: NMI button pressed", "PRIORITY=%i",
                    LOG_INFO, "REDFISH_MESSAGE_ID=%s",
                    "OpenBMC.0.1.NMIButtonPressed", NULL);
}

static void nmiDiagIntLog()
{
    sd_journal_send("MESSAGE=PowerControl: NMI Diagnostic Interrupt",
                    "PRIORITY=%i", LOG_INFO, "REDFISH_MESSAGE_ID=%s",
                    "OpenBMC.0.1.NMIDiagnosticInterrupt", NULL);
}

PersistentState::PersistentState()
{
    // create the power control directory if it doesn't exist
    std::error_code ec;
    if (!(std::filesystem::create_directories(powerControlDir, ec)))
    {
        if (ec.value() != 0)
        {
            lg2::error("failed to create {DIR_NAME}: {ERROR_MSG}", "DIR_NAME",
                       powerControlDir.string(), "ERROR_MSG", ec.message());
            throw std::runtime_error("Failed to create state directory");
        }
    }

    // read saved state, it's ok, if the file doesn't exists
    std::ifstream appStateStream(powerControlDir / stateFile);
    if (!appStateStream.is_open())
    {
        lg2::info("Cannot open state file \'{PATH}\'", "PATH",
                  std::string(powerControlDir / stateFile));
        stateData = nlohmann::json({});
        return;
    }
    try
    {
        appStateStream >> stateData;
        if (stateData.is_discarded())
        {
            lg2::info("Cannot parse state file \'{PATH}\'", "PATH",
                      std::string(powerControlDir / stateFile));
            stateData = nlohmann::json({});
            return;
        }
    }
    catch (const std::exception& ex)
    {
        lg2::info("Cannot read state file \'{PATH}\'", "PATH",
                  std::string(powerControlDir / stateFile));
        stateData = nlohmann::json({});
        return;
    }
}
PersistentState::~PersistentState()
{
    saveState();
}
const std::string PersistentState::get(Params parameter)
{
    auto val = stateData.find(getName(parameter));
    if (val != stateData.end())
    {
        return val->get<std::string>();
    }
    return getDefault(parameter);
}
void PersistentState::set(Params parameter, const std::string& value)
{
    stateData[getName(parameter)] = value;
    saveState();
}

const std::string PersistentState::getName(const Params parameter)
{
    switch (parameter)
    {
        case Params::PowerState:
            return "PowerState";
    }
    return "";
}
const std::string PersistentState::getDefault(const Params parameter)
{
    switch (parameter)
    {
        case Params::PowerState:
            return "xyz.openbmc_project.State.Chassis.PowerState.Off";
    }
    return "";
}
void PersistentState::saveState()
{
    std::ofstream appStateStream(powerControlDir / stateFile, std::ios::trunc);
    if (!appStateStream.is_open())
    {
        lg2::error("Cannot write state file \'{PATH}\'", "PATH",
                   std::string(powerControlDir / stateFile));
        return;
    }
    appStateStream << stateData.dump(indentationSize);
}

static constexpr char const* setingsService = "xyz.openbmc_project.Settings";
static constexpr char const* powerRestorePolicyIface =
    "xyz.openbmc_project.Control.Power.RestorePolicy";
#ifdef USE_ACBOOT
static constexpr char const* powerACBootObject =
    "/xyz/openbmc_project/control/host0/ac_boot";
static constexpr char const* powerACBootIface =
    "xyz.openbmc_project.Common.ACBoot";
#endif // USE_ACBOOT

namespace match_rules = sdbusplus::bus::match::rules;

static int powerRestoreConfigHandler(sd_bus_message* m, void* context,
                                     sd_bus_error*)
{
    if (context == nullptr || m == nullptr)
    {
        throw std::runtime_error("Invalid match");
    }
    sdbusplus::message_t message(m);
    PowerRestoreController* powerRestore =
        static_cast<PowerRestoreController*>(context);

    if (std::string(message.get_member()) == "InterfacesAdded")
    {
        sdbusplus::message::object_path path;
        boost::container::flat_map<std::string, dbusPropertiesList> data;

        message.read(path, data);

        for (auto& [iface, properties] : data)
        {
            if ((iface == powerRestorePolicyIface)
#ifdef USE_ACBOOT
                || (iface == powerACBootIface)
#endif // USE_ACBOOT
            )
            {
                powerRestore->setProperties(properties);
            }
        }
    }
    else if (std::string(message.get_member()) == "PropertiesChanged")
    {
        std::string interfaceName;
        dbusPropertiesList propertiesChanged;

        message.read(interfaceName, propertiesChanged);

        powerRestore->setProperties(propertiesChanged);
    }
    return 1;
}

void PowerRestoreController::run()
{
    std::string powerRestorePolicyObject =
        "/xyz/openbmc_project/control/host" + node + "/power_restore_policy";
    powerRestorePolicyLog();
    // this list only needs to be created once
    if (matches.empty())
    {
        matches.emplace_back(
            *conn,
            match_rules::interfacesAdded() +
                match_rules::argNpath(0, powerRestorePolicyObject) +
                match_rules::sender(setingsService),
            powerRestoreConfigHandler, this);
#ifdef USE_ACBOOT
        matches.emplace_back(*conn,
                             match_rules::interfacesAdded() +
                                 match_rules::argNpath(0, powerACBootObject) +
                                 match_rules::sender(setingsService),
                             powerRestoreConfigHandler, this);
        matches.emplace_back(*conn,
                             match_rules::propertiesChanged(powerACBootObject,
                                                            powerACBootIface) +
                                 match_rules::sender(setingsService),
                             powerRestoreConfigHandler, this);
#endif // USE_ACBOOT
    }

    // Check if it's already on DBus
    conn->async_method_call(
        [this](boost::system::error_code ec,
               const dbusPropertiesList properties) {
            if (ec)
            {
                return;
            }
            setProperties(properties);
        },
        setingsService, powerRestorePolicyObject,
        "org.freedesktop.DBus.Properties", "GetAll", powerRestorePolicyIface);

#ifdef USE_ACBOOT
    // Check if it's already on DBus
    conn->async_method_call(
        [this](boost::system::error_code ec,
               const dbusPropertiesList properties) {
            if (ec)
            {
                return;
            }
            setProperties(properties);
        },
        setingsService, powerACBootObject, "org.freedesktop.DBus.Properties",
        "GetAll", powerACBootIface);
#endif
}

void PowerRestoreController::setProperties(const dbusPropertiesList& props)
{
    for (auto& [property, propValue] : props)
    {
        if (property == "PowerRestorePolicy")
        {
            const std::string* value = std::get_if<std::string>(&propValue);
            if (value == nullptr)
            {
                lg2::error("Unable to read Power Restore Policy");
                continue;
            }
            powerRestorePolicy = *value;
        }
        else if (property == "PowerRestoreDelay")
        {
            const uint64_t* value = std::get_if<uint64_t>(&propValue);
            if (value == nullptr)
            {
                lg2::error("Unable to read Power Restore Delay");
                continue;
            }
            powerRestoreDelay = *value / 1000000; // usec to sec
        }
#ifdef USE_ACBOOT
        else if (property == "ACBoot")
        {
            const std::string* value = std::get_if<std::string>(&propValue);
            if (value == nullptr)
            {
                lg2::error("Unable to read AC Boot status");
                continue;
            }
            acBoot = *value;
        }
#endif // USE_ACBOOT
    }
    invokeIfReady();
}

void PowerRestoreController::invokeIfReady()
{
    if ((powerRestorePolicy.empty()) || (powerRestoreDelay < 0))
    {
        return;
    }
#ifdef USE_ACBOOT
    if (acBoot.empty() || acBoot == "Unknown")
    {
        return;
    }
#endif

    matches.clear();
    if (!timerFired)
    {
        // Calculate the delay from now to meet the requested delay
        // Subtract the approximate uboot time
        static constexpr const int ubootSeconds = 20;
        int delay = powerRestoreDelay - ubootSeconds;
        // Subtract the time since boot
        struct sysinfo info = {};
        if (sysinfo(&info) == 0)
        {
            delay -= info.uptime;
        }

        if (delay > 0)
        {
            powerRestoreTimer.expires_after(std::chrono::seconds(delay));
            lg2::info("Power Restore delay of {DELAY} seconds started", "DELAY",
                      delay);
            powerRestoreTimer.async_wait([this](const boost::system::error_code
                                                    ec) {
                if (ec)
                {
                    // operation_aborted is expected if timer is canceled before
                    // completion.
                    if (ec == boost::asio::error::operation_aborted)
                    {
                        return;
                    }
                    lg2::error(
                        "power restore policy async_wait failed: {ERROR_MSG}",
                        "ERROR_MSG", ec.message());
                }
                else
                {
                    lg2::info("Power Restore delay timer expired");
                }
                invoke();
            });
            timerFired = true;
        }
        else
        {
            invoke();
        }
    }
}

void PowerRestoreController::invoke()
{
    // we want to run Power Restore only once
    if (policyInvoked)
    {
        return;
    }
    policyInvoked = true;

    lg2::info("Invoking Power Restore Policy {POLICY}", "POLICY",
              powerRestorePolicy);
    if (powerRestorePolicy ==
        "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.AlwaysOn")
    {
        sendPowerControlEvent(Event::powerOnRequest);
        setRestartCauseProperty(getRestartCause(RestartCause::powerPolicyOn));
    }
    else if (powerRestorePolicy ==
             "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.Restore")
    {
        if (wasPowerDropped())
        {
            lg2::info("Power was dropped, restoring Host On state");
            sendPowerControlEvent(Event::powerOnRequest);
            setRestartCauseProperty(
                getRestartCause(RestartCause::powerPolicyRestore));
        }
        else
        {
            lg2::info("No power drop, restoring Host Off state");
        }
    }
    // We're done with the previous power state for the restore policy, so store
    // the current state
    savePowerState(powerState);
}

bool PowerRestoreController::wasPowerDropped()
{
    std::string state = appState.get(PersistentState::Params::PowerState);
    return state == "xyz.openbmc_project.State.Chassis.PowerState.On";
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
    return true;
}

static int setMaskedGPIOOutputForMs(gpiod::line& maskedGPIOLine,
                                    const std::string& name, const int value,
                                    const int durationMs)
{
    // Set the masked GPIO line to the specified value
    maskedGPIOLine.set_value(value);
    lg2::info("{GPIO_NAME} set to {GPIO_VALUE}", "GPIO_NAME", name,
              "GPIO_VALUE", value);
    gpioAssertTimer.expires_after(std::chrono::milliseconds(durationMs));
    gpioAssertTimer.async_wait(
        [maskedGPIOLine, value, name](const boost::system::error_code ec) {
            // Set the masked GPIO line back to the opposite value
            maskedGPIOLine.set_value(!value);
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

static int setGPIOOutputForMs(const ConfigData& config, const int value,
                              const int durationMs)
{
    // If the requested GPIO is masked, use the mask line to set the output
    if (powerButtonMask && config.lineName == powerOutConfig.lineName)
    {
        return setMaskedGPIOOutputForMs(powerButtonMask, config.lineName, value,
                                        durationMs);
    }
    if (resetButtonMask && config.lineName == resetOutConfig.lineName)
    {
        return setMaskedGPIOOutputForMs(resetButtonMask, config.lineName, value,
                                        durationMs);
    }

    // No mask set, so request and set the GPIO normally
    gpiod::line gpioLine;
    if (!setGPIOOutput(config.lineName, value, gpioLine))
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

static int assertGPIOForMs(const ConfigData& config, const int durationMs)
{
    return setGPIOOutputForMs(config, config.polarity, durationMs);
}

static void powerOn()
{
    //1 -> 0(500ms) -> 1(1000ms) -> 0(3000ms) -> 1
    gpiod::line gpioLine;
    setGPIOOutput(powerOutConfig.lineName, 0, gpioLine);
    std::this_thread::sleep_for(std::chrono::milliseconds(TimerMap["PowerOn0Ms"]));

    setGPIOOutput(powerOutConfig.lineName, 1, gpioLine);
    std::this_thread::sleep_for(std::chrono::milliseconds(TimerMap["PowerOn1Ms"]));

    setGPIOOutput(powerOutConfig.lineName, 0, gpioLine);
    std::this_thread::sleep_for(std::chrono::milliseconds(TimerMap["PowerOn2Ms"]));

    setGPIOOutput(powerOutConfig.lineName, 1, gpioLine);
}

static void forcePowerOff()
{
    gpiod::line gpioLine;

    setGPIOOutput(powerOutConfig.lineName, 0, gpioLine);
    std::this_thread::sleep_for(std::chrono::milliseconds(TimerMap["PowerOff0Ms"]));

    setGPIOOutput(powerOutConfig.lineName, 1, gpioLine);
    std::this_thread::sleep_for(std::chrono::milliseconds(TimerMap["PowerOff1Ms"]));

    setGPIOOutput(powerOutConfig.lineName, 0, gpioLine);
}

static void reset()
{
    assertGPIOForMs(resetOutConfig, TimerMap["ResetPulseMs"]);
}



static void powerCycleTimerStart()
{
    lg2::info("Power-cycle timer started");
    powerCycleTimer.expires_after(
        std::chrono::milliseconds(TimerMap["PowerCycleMs"]));
    powerCycleTimer.async_wait([](const boost::system::error_code ec) {
        if (ec)
        {
            // operation_aborted is expected if timer is canceled before
            // completion.
            if (ec != boost::asio::error::operation_aborted)
            {
                lg2::error("Power-cycle async_wait failed: {ERROR_MSG}",
                           "ERROR_MSG", ec.message());
            }
            lg2::info("Power-cycle timer canceled");
            return;
        }
        lg2::info("Power-cycle timer completed");
        sendPowerControlEvent(Event::powerCycleTimerExpired);
    });
}

static void psPowerOKWatchdogTimerStart()
{
    lg2::info("power supply power OK watchdog timer started");
    psPowerOKWatchdogTimer.expires_after(
        std::chrono::milliseconds(TimerMap["PsPowerOKWatchdogMs"]));
    psPowerOKWatchdogTimer.async_wait([](const boost::system::error_code ec) {
        if (ec)
        {
            // operation_aborted is expected if timer is canceled before
            // completion.
            if (ec != boost::asio::error::operation_aborted)
            {
                lg2::error(
                    "power supply power OK watchdog async_wait failed: {ERROR_MSG}",
                    "ERROR_MSG", ec.message());
            }
            lg2::info("power supply power OK watchdog timer canceled");
            return;
        }
        lg2::info("power supply power OK watchdog timer expired");
        sendPowerControlEvent(Event::psPowerOKWatchdogTimerExpired);
    });
}

static void pohCounterClean()
{
    conn->async_method_call(
        [](boost::system::error_code ec) {
            if (ec)
            {
                lg2::error("failed to set poh counter");
            }
        },
        "xyz.openbmc_project.State.Host",
        "/xyz/openbmc_project/state/POH",
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.State.PowerOnHours", "POHCounter",
        std::variant<uint32_t>(hopInitValue));
}

static void pohCounterTimerStart()
{
    // lg2::info("POH timer started");
    // Set the time-out as 1 hour, to align with POH command in ipmid
    // pohCounterTimer.expires_after(std::chrono::hours(1));
    pohCounterTimer.expires_after(std::chrono::minutes(5));
    // pohCounterTimer.expires_after(std::chrono::seconds(5));
    pohCounterTimer.async_wait([](const boost::system::error_code& ec) {
        if (ec)
        {
            // operation_aborted is expected if timer is canceled before
            // completion.
            if (ec != boost::asio::error::operation_aborted)
            {
                lg2::error("POH timer async_wait failed: {ERROR_MSG}",
                           "ERROR_MSG", ec.message());
            }
            lg2::info("POH timer canceled");
            return;
        }

        if (getHostState(powerState) !=
            "xyz.openbmc_project.State.Host.HostState.Running")
        {
            // pohCounterClean();
            return;
        }

        conn->async_method_call(
            [](boost::system::error_code ec,
               const std::variant<uint32_t>& pohCounterProperty) {
                if (ec)
                {
                    lg2::error("error getting poh counter");
                    return;
                }
                const uint32_t* pohCounter =
                    std::get_if<uint32_t>(&pohCounterProperty);
                if (pohCounter == nullptr)
                {
                    lg2::error("unable to read poh counter");
                    return;
                }

                conn->async_method_call(
                    [](boost::system::error_code ec) {
                        if (ec)
                        {
                            lg2::error("failed to set poh counter");
                        }
                    },
                    "xyz.openbmc_project.State.Host",
                    "/xyz/openbmc_project/state/POH",
                    "org.freedesktop.DBus.Properties", "Set",
                    "xyz.openbmc_project.State.PowerOnHours", "POHCounter",
                    std::variant<uint32_t>(*pohCounter + 1));
            },
            "xyz.openbmc_project.State.Host",
            "/xyz/openbmc_project/state/POH",
            "org.freedesktop.DBus.Properties", "Get",
            "xyz.openbmc_project.State.PowerOnHours", "POHCounter");

        pohCounterTimerStart();
    });
}

static void currentHostStateMonitor()
{
    if (getHostState(powerState) ==
        "xyz.openbmc_project.State.Host.HostState.Running")
    {
        pohCounterTimerStart();
        // Clear the restart cause set for the next restart
        clearRestartCause();
    }
    else
    {
        pohCounterTimer.cancel();
        // Set the restart cause set for this restart
        setRestartCause();
        pohCounterClean();
    }

    static auto match = sdbusplus::bus::match_t(
        *conn,
        "type='signal',member='PropertiesChanged', "
        "interface='org.freedesktop.DBus.Properties', "
        "arg0='xyz.openbmc_project.State.Host'",
        [](sdbusplus::message_t& message) {
            std::string intfName;
            std::map<std::string, std::variant<std::string>> properties;

            try
            {
                message.read(intfName, properties);
            }
            catch (const std::exception& e)
            {
                lg2::error("Unable to read host state: {ERROR}", "ERROR", e);
                return;
            }
            if (properties.empty())
            {
                lg2::error("ERROR: Empty PropertiesChanged signal received");
                return;
            }

            // We only want to check for CurrentHostState
            if (properties.begin()->first != "CurrentHostState")
            {
                return;
            }
            std::string* currentHostState =
                std::get_if<std::string>(&(properties.begin()->second));
            if (currentHostState == nullptr)
            {
                lg2::error("{PROPERTY} property invalid", "PROPERTY",
                           properties.begin()->first);
                return;
            }

            if (*currentHostState ==
                "xyz.openbmc_project.State.Host.HostState.Running")
            {
                pohCounterTimerStart();
                // Clear the restart cause set for the next restart
                clearRestartCause();
                sd_journal_send("MESSAGE=Host system DC power is on",
                                "PRIORITY=%i", LOG_INFO,
                                "REDFISH_MESSAGE_ID=%s",
                                "OpenBMC.0.1.DCPowerOn", NULL);
            }
            else
            {
                pohCounterTimer.cancel();
                pohCounterClean();
                // POST_COMPLETE GPIO event is not working in some platforms
                // when power state is changed to OFF. This resulted in
                // 'OperatingSystemState' to stay at 'Standby', even though
                // system is OFF. Set 'OperatingSystemState' to 'Inactive'
                // if HostState is trurned to OFF.
                setOperatingSystemState(OperatingSystemStateStage::Inactive);

                // Set the restart cause set for this restart
                setRestartCause();
#ifdef USE_ACBOOT
                resetACBootProperty();
#endif // USE_ACBOOT
                sd_journal_send("MESSAGE=Host system DC power is off",
                                "PRIORITY=%i", LOG_INFO,
                                "REDFISH_MESSAGE_ID=%s",
                                "OpenBMC.0.1.DCPowerOff", NULL);
            }
        });
}



static void powerStateOn(const Event event)
{
    logEvent(__FUNCTION__, event);
    switch (event)
    {
        case Event::psPowerOKDeAssert:
            setPowerState(PowerState::off);
            pohCounterClean();
            // DC power is unexpectedly lost, beep
            beep(beepPowerFail);
            break;
        case Event::powerButtonPressed:
            //se8 = force poweroff;
            break;
        case Event::powerOffRequest:
            setPowerState(PowerState::transitionToOff);
            forcePowerOff();
            break;
        case Event::powerCycleRequest:
            setPowerState(PowerState::transitionToCycleOff);
            forcePowerOff();
            break;
        case Event::resetRequest:
            reset();
            break;
        default:
            lg2::info("No action taken.");
            break;
    }
}

static void powerStateWaitForPSPowerOK(const Event event)
{
    logEvent(__FUNCTION__, event);
    switch (event)
    {
        case Event::psPowerOKAssert:
        {
            // Cancel any GPIO assertions held during the transition
            gpioAssertTimer.cancel();
            psPowerOKWatchdogTimer.cancel();
            setPowerState(PowerState::on);
            break;
        }
        case Event::psPowerOKWatchdogTimerExpired:
            setPowerState(PowerState::off);
            psPowerOKFailedLog();
            break;
        default:
            lg2::info("No action taken.");
            break;
    }
}



static void powerStateOff(const Event event)
{
    logEvent(__FUNCTION__, event);
    switch (event)
    {
        case Event::psPowerOKAssert:
        {
            setPowerState(PowerState::on);
            break;
        }
        case Event::powerButtonPressed:
            psPowerOKWatchdogTimerStart();
            setPowerState(PowerState::waitForPSPowerOK);
            break;
        case Event::powerOnRequest:
            psPowerOKWatchdogTimerStart();
            setPowerState(PowerState::waitForPSPowerOK);
            powerOn();
            break;
        default:
            lg2::info("No action taken.");
            break;
    }
}

static void powerStateTransitionToOff(const Event event)
{
    logEvent(__FUNCTION__, event);
    gpiod::line gpioLine;
    switch (event)
    {
        case Event::psPowerOKDeAssert:
            // Cancel any GPIO assertions held during the transition
            gpioAssertTimer.cancel();
            setGPIOOutput(powerOutConfig.lineName, 1, gpioLine);
            setPowerState(PowerState::off);
            break;
        default:
            lg2::info("No action taken.");
            break;
    }
}

static void powerStateGracefulTransitionToOff(const Event event)
{
    logEvent(__FUNCTION__, event);
    switch (event)
    {
        case Event::psPowerOKDeAssert:
            gracefulPowerOffTimer.cancel();
            setPowerState(PowerState::off);
            break;
        case Event::powerOffRequest:
            gracefulPowerOffTimer.cancel();
            setPowerState(PowerState::transitionToOff);
            forcePowerOff();
            break;
        case Event::powerCycleRequest:
            gracefulPowerOffTimer.cancel();
            setPowerState(PowerState::transitionToCycleOff);
            forcePowerOff();
            break;
        case Event::resetRequest:
            gracefulPowerOffTimer.cancel();
            setPowerState(PowerState::on);
            reset();
            break;
        default:
            lg2::info("No action taken.");
            break;
    }
}

static void powerStateCycleOff(const Event event)
{
    logEvent(__FUNCTION__, event);
    switch (event)
    {
        case Event::psPowerOKAssert:
        {
            powerCycleTimer.cancel();
            setPowerState(PowerState::on);
            break;
        }
        case Event::powerButtonPressed:
            powerCycleTimer.cancel();
            psPowerOKWatchdogTimerStart();
            setPowerState(PowerState::waitForPSPowerOK);
            break;
        case Event::powerCycleTimerExpired:
            psPowerOKWatchdogTimerStart();
            setPowerState(PowerState::waitForPSPowerOK);
            powerOn();
            break;
        default:
            lg2::info("No action taken.");
            break;
    }
}

static void powerStateTransitionToCycleOff(const Event event)
{
    logEvent(__FUNCTION__, event);
    switch (event)
    {
        case Event::psPowerOKDeAssert:
            // Cancel any GPIO assertions held during the transition
            gpioAssertTimer.cancel();
            setPowerState(PowerState::cycleOff);
            powerCycleTimerStart();
            break;
        default:
            lg2::info("No action taken.");
            break;
    }
}





static void psPowerOKHandler(bool state)
{
    Event powerControlEvent =
        state ? Event::psPowerOKAssert : Event::psPowerOKDeAssert;
    sendPowerControlEvent(powerControlEvent);
}



static void sioOnControlHandler(bool state)
{
    lg2::info("SIO_ONCONTROL value changed: {VALUE}", "VALUE",
              static_cast<int>(state));
}



static void powerButtonHandler(bool state)
{
    powerButtonIface->set_property("ButtonPressed", !state);
    if (!state)
    {
        powerButtonPressLog();
        if (!powerButtonMask)
        {
            sendPowerControlEvent(Event::powerButtonPressed);
            addRestartCause(RestartCause::powerButton);
        }
        else
        {
            lg2::info("power button press masked");
        }
    }
}

static void resetButtonHandler(bool state)
{
    resetButtonIface->set_property("ButtonPressed", !state);
    if (!state)
    {
        resetButtonPressLog();
        //se8 just note
        // if (!resetButtonMask)
        // {
        //     sendPowerControlEvent(Event::resetButtonPressed);
        //     addRestartCause(RestartCause::resetButton);
        // }
        // else
        // {
        //     lg2::info("reset button press masked");
        // }
    }
}





static void idButtonHandler(bool state)
{
    idButtonIface->set_property("ButtonPressed", !state);
}


static int loadConfigValues()
{
    const std::string configFilePath =
        "/usr/share/sophgo-se8-power-control/power-config-host" + power_control::node +
        ".json";
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
    auto gpios = jsonData["gpio_configs"];
    auto timers = jsonData["timing_configs"];

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

static bool getDbusMsgGPIOState(sdbusplus::message_t& msg,
                                const std::string& lineName, bool& value)
{
    std::string thresholdInterface;
    std::string event;
    boost::container::flat_map<std::string, std::variant<bool>>
        propertiesChanged;
    try
    {
        msg.read(thresholdInterface, propertiesChanged);
        if (propertiesChanged.empty())
        {
            return false;
        }

        event = propertiesChanged.begin()->first;
        if (event.empty() || event != lineName)
        {
            return false;
        }

        value = std::get<bool>(propertiesChanged.begin()->second);
        return true;
    }
    catch (const std::exception& e)
    {
        lg2::error(
            "exception while reading dbus property \'{DBUS_NAME}\': {ERROR}",
            "DBUS_NAME", lineName, "ERROR", e);
        return false;
    }
}

static sdbusplus::bus::match_t
    dbusGPIOMatcher(const ConfigData& cfg, std::function<void(bool)> onMatch)
{
    auto pulseEventMatcherCallback = [&cfg,
                                      onMatch](sdbusplus::message_t& msg) {
        bool value = false;
        if (!getDbusMsgGPIOState(msg, cfg.lineName, value))
        {
            return;
        }
        onMatch(value);
    };

    return sdbusplus::bus::match_t(
        static_cast<sdbusplus::bus_t&>(*conn),
        "type='signal',interface='org.freedesktop.DBus.Properties',member='"
        "PropertiesChanged',arg0='" +
            cfg.interface + "'",
        std::move(pulseEventMatcherCallback));
}

int getProperty(ConfigData& configData)
{
    auto method = conn->new_method_call(
        configData.dbusName.c_str(), configData.path.c_str(),
        "org.freedesktop.DBus.Properties", "Get");
    method.append(configData.interface.c_str(), configData.lineName.c_str());

    auto reply = conn->call(method);
    if (reply.is_method_error())
    {
        lg2::error(
            "Error reading {PROPERTY} D-Bus property on interface {INTERFACE} and path {PATH}",
            "PROPERTY", configData.lineName, "INTERFACE", configData.interface,
            "PATH", configData.path);
        return -1;
    }
    std::variant<bool> resp;
    reply.read(resp);
    auto respValue = std::get_if<bool>(&resp);
    if (!respValue)
    {
        lg2::error("Error: {PROPERTY} D-Bus property is not the expected type",
                   "PROPERTY", configData.lineName);
        return -1;
    }
    return (*respValue);
}
} // namespace power_control

int main(int argc, char* argv[])
{
    using namespace power_control;

    if (argc > 1)
    {
        node = argv[1];
    }
    lg2::info("Start Chassis power control service for host : {NODE}", "NODE",
              node);

    conn = std::make_shared<sdbusplus::asio::connection>(io);

    // Load GPIO's through json config file
    if (loadConfigValues() == -1)
    {
        lg2::error("Host{NODE}: Error in Parsing...", "NODE", node);
    }
    /* Currently for single host based systems additional busname is added
    with "0" at the end of the name ex : xyz.openbmc_project.State.Host0.
    Going forward for single hosts the old bus name without zero numbering
    will be removed when all other applications adapted to the
    bus name with zero numbering (xyz.openbmc_project.State.Host0). */

    if (node == "0")
    {
        // Request all the dbus names
        conn->request_name(hostDbusName.c_str());
        conn->request_name(chassisDbusName.c_str());
        conn->request_name(osDbusName.c_str());
        conn->request_name(buttonDbusName.c_str());
        conn->request_name(rstCauseDbusName.c_str());
    }

    hostDbusName += node;
    chassisDbusName += node;
    osDbusName += node;
    buttonDbusName += node;
    rstCauseDbusName += node;

    // Request all the dbus names
    conn->request_name(hostDbusName.c_str());
    conn->request_name(chassisDbusName.c_str());
    conn->request_name(osDbusName.c_str());
    conn->request_name(buttonDbusName.c_str());
    conn->request_name(rstCauseDbusName.c_str());



    // Request PS_PWROK GPIO events
    if (powerOkConfig.type == ConfigType::GPIO)
    {
        if (!requestGPIOEvents(powerOkConfig.lineName, psPowerOKHandler,
                               psPowerOKLine, psPowerOKEvent))
        {
            return -1;
        }
    }
    else if (powerOkConfig.type == ConfigType::DBUS)
    {

        static sdbusplus::bus::match_t powerOkEventMonitor =
            power_control::dbusGPIOMatcher(powerOkConfig, psPowerOKHandler);
    }
    else
    {
        lg2::error("PowerOk name should be configured from json config file");
        return -1;
    }





    // Request RESET_BUTTON GPIO events
    if (resetButtonConfig.type == ConfigType::GPIO)
    {
        if (!requestGPIOEvents(resetButtonConfig.lineName, resetButtonHandler,
                               resetButtonLine, resetButtonEvent))
        {
            return -1;
        }
    }
    else if (resetButtonConfig.type == ConfigType::DBUS)
    {
        static sdbusplus::bus::match_t resetButtonEventMonitor =
            power_control::dbusGPIOMatcher(resetButtonConfig,
                                           resetButtonHandler);
    }




    // Initialize POWER_OUT and RESET_OUT GPIO.
    // high_level
    gpiod::line line;
    if (!powerOutConfig.lineName.empty())
    {
        if (!setGPIOOutput(powerOutConfig.lineName, !powerOutConfig.polarity,
                           line))
        {
            return -1;
        }
    }
    else
    {
        lg2::error("powerOut name should be configured from json config file");
        return -1;
    }

    if (!resetOutConfig.lineName.empty())
    {
        if (!setGPIOOutput(resetOutConfig.lineName, !resetOutConfig.polarity,
                           line))
        {
            return -1;
        }
    }
    else
    {
        lg2::error("ResetOut name should be configured from json config file");
        return -1;
    }
    // Release line
    line.reset();

    // Initialize the power state and operating system state
    powerState = PowerState::off;
    operatingSystemState = OperatingSystemStateStage::Inactive;
    // Check power good

    if (powerOkConfig.type == ConfigType::GPIO)
    {
        if (psPowerOKLine.get_value() > 0)
        {
            powerState = PowerState::on;
        }
    }
    else
    {
        if (getProperty(powerOkConfig))
        {
            powerState = PowerState::on;
        }
    }
    // Check if we need to start the Power Restore policy
    if (powerState != PowerState::on)
    {
        powerRestore.run();
    }

    lg2::info("Initializing power state.");
    logStateTransition(powerState);

    // Power Control Service
    sdbusplus::asio::object_server hostServer =
        sdbusplus::asio::object_server(conn);

    hostHopIface =
        hostServer.add_interface("/xyz/openbmc_project/state/POH",
                                 "xyz.openbmc_project.State.PowerOnHours");
    hostHopIface->register_property("POHCounter", hopInitValue, sdbusplus::asio::PropertyPermission::readWrite );

    hostHopIface->initialize();

    // Power Control Interface
    hostIface =
        hostServer.add_interface("/xyz/openbmc_project/state/host" + node,
                                 "xyz.openbmc_project.State.Host");
    // Interface for IPMI/Redfish initiated host state transitions
    hostIface->register_property(
        "RequestedHostTransition",
        std::string("xyz.openbmc_project.State.Host.Transition.Off"),
        [](const std::string& requested, std::string& resp) {
            if (requested == "xyz.openbmc_project.State.Host.Transition.Off")
            {
                // if power button is masked, ignore this
                //web shutdown orderly se8：Event::gracefulPowerOffRequest -> Event::powerOffRequest
                if (!powerButtonMask)
                {
                    sendPowerControlEvent(Event::powerOffRequest);
                    addRestartCause(RestartCause::command);
                }
                else
                {
                    lg2::info("Power Button Masked.");
                    throw std::invalid_argument("Transition Request Masked");
                    return 0;
                }
            }
            else if (requested == "xyz.openbmc_project.State.Host.Transition.On")
            {
                // if power button is masked, ignore this
                //web powerOn
                if (!powerButtonMask)
                {
                    sendPowerControlEvent(Event::powerOnRequest);
                    addRestartCause(RestartCause::command);
                }
                else
                {
                    lg2::info("Power Button Masked.");
                    throw std::invalid_argument("Transition Request Masked");
                    return 0;
                }
            }
            //web Reboot immediate
            else if ((requested == "xyz.openbmc_project.State.Host.Transition.Reboot") ||
                     (requested == "xyz.openbmc_project.State.Host.Transition.ForceWarmReboot"))
            {
                // if power button is masked, ignore this
                 //se8 = power cycle Event::powerCycleRequest
                if (!powerButtonMask)
                {

                    sendPowerControlEvent(Event::powerCycleRequest);
                    addRestartCause(RestartCause::command);
                }
                else
                {
                    lg2::info("Power Button Masked.");
                    throw std::invalid_argument("Transition Request Masked");
                    return 0;
                }
            }
            else if (requested == "xyz.openbmc_project.State.Host.Transition.GracefulWarmReboot")
            {
                // se8 todo Event::resetRequest
                if (!resetButtonMask)
                {
                    sendPowerControlEvent(Event::resetRequest);
                    addRestartCause(RestartCause::command);
                }
                else
                {
                    lg2::info("Reset Button Masked.");
                    throw std::invalid_argument("Transition Request Masked");
                    return 0;
                }
            }
            else
            {
                lg2::error("Unrecognized host state transition request.");
                throw std::invalid_argument("Unrecognized Transition Request");
                return 0;
            }
            resp = requested;
            return 1;
        });
    hostIface->register_property("CurrentHostState",
                                 std::string(getHostState(powerState)));

    hostIface->initialize();


    // Chassis Control Service
    sdbusplus::asio::object_server chassisServer =
        sdbusplus::asio::object_server(conn);

    // Chassis Control Interface
    chassisIface =
        chassisServer.add_interface("/xyz/openbmc_project/state/chassis" + node,
                                    "xyz.openbmc_project.State.Chassis");

    chassisIface->register_property(
        "RequestedPowerTransition",
        std::string("xyz.openbmc_project.State.Chassis.Transition.Off"),
        [](const std::string& requested, std::string& resp) {
            if (requested == "xyz.openbmc_project.State.Chassis.Transition.Off")
            {
                // if power button is masked, ignore this
                if (!powerButtonMask)
                {
                    sendPowerControlEvent(Event::powerOffRequest);
                    addRestartCause(RestartCause::command);
                }
                else
                {
                    lg2::info("Power Button Masked.");
                    throw std::invalid_argument("Transition Request Masked");
                    return 0;
                }
            }
            else if (requested ==
                     "xyz.openbmc_project.State.Chassis.Transition.On")
            {
                // if power button is masked, ignore this
                if (!powerButtonMask)
                {
                    sendPowerControlEvent(Event::powerOnRequest);
                    addRestartCause(RestartCause::command);
                }
                else
                {
                    lg2::info("Power Button Masked.");
                    throw std::invalid_argument("Transition Request Masked");
                    return 0;
                }
            }
            else if (requested ==
                     "xyz.openbmc_project.State.Chassis.Transition.PowerCycle")
            {
                // if power button is masked, ignore this
                if (!powerButtonMask)
                {
                    sendPowerControlEvent(Event::powerCycleRequest);
                    addRestartCause(RestartCause::command);
                }
                else
                {
                    lg2::info("Power Button Masked.");
                    throw std::invalid_argument("Transition Request Masked");
                    return 0;
                }
            }
            else
            {
                lg2::error("Unrecognized chassis state transition request.");
                throw std::invalid_argument("Unrecognized Transition Request");
                return 0;
            }
            resp = requested;
            return 1;
        });
    chassisIface->register_property("CurrentPowerState",
                                    std::string(getChassisState(powerState)));
    chassisIface->register_property("LastStateChangeTime", getCurrentTimeMs());

    chassisIface->initialize();


    // Buttons Service
    sdbusplus::asio::object_server buttonsServer =
        sdbusplus::asio::object_server(conn);



    if (!resetButtonConfig.lineName.empty())
    {
        // Reset Button Interface

        resetButtonIface = buttonsServer.add_interface(
            "/xyz/openbmc_project/chassis/buttons/reset",
            "xyz.openbmc_project.Chassis.Buttons");

        resetButtonIface->register_property(
            "ButtonMasked", false, [](const bool requested, bool& current) {
                if (requested)
                {
                    if (resetButtonMask)
                    {
                        return 1;
                    }
                    if (!setGPIOOutput(resetOutConfig.lineName,
                                       !resetOutConfig.polarity,
                                       resetButtonMask))
                    {
                        throw std::runtime_error("Failed to request GPIO");
                        return 0;
                    }
                    lg2::info("Reset Button Masked.");
                }
                else
                {
                    if (!resetButtonMask)
                    {
                        return 1;
                    }
                    lg2::info("Reset Button Un-masked");
                    resetButtonMask.reset();
                }
                // Update the mask setting
                current = requested;
                return 1;
            });

        // Check reset button state
        bool resetButtonPressed;
        if (resetButtonConfig.type == ConfigType::GPIO)
        {
            resetButtonPressed = resetButtonLine.get_value() == 0;
        }
        else
        {
            resetButtonPressed = getProperty(resetButtonConfig) == 0;
        }

        resetButtonIface->register_property("ButtonPressed",
                                            resetButtonPressed);

        resetButtonIface->initialize();
    }







    // OS State Service
    sdbusplus::asio::object_server osServer =
        sdbusplus::asio::object_server(conn);

    // OS State Interface
    osIface = osServer.add_interface(
        "/xyz/openbmc_project/state/os",
        "xyz.openbmc_project.State.OperatingSystem.Status");

    // Get the initial OS state based on POST complete
    //      0: Asserted, OS state is "Standby" (ready to boot)
    //      1: De-Asserted, OS state is "Inactive"
    OperatingSystemStateStage osState;

    osIface->register_property(
        "OperatingSystemState",
        std::string(getOperatingSystemStateStage(osState)));

    osIface->initialize();

    // Restart Cause Service
    sdbusplus::asio::object_server restartCauseServer =
        sdbusplus::asio::object_server(conn);

    // Restart Cause Interface
    restartCauseIface = restartCauseServer.add_interface(
        "/xyz/openbmc_project/control/host" + node + "/restart_cause",
        "xyz.openbmc_project.Control.Host.RestartCause");

    restartCauseIface->register_property(
        "RestartCause",
        std::string("xyz.openbmc_project.State.Host.RestartCause.Unknown"));

    restartCauseIface->register_property(
        "RequestedRestartCause",
        std::string("xyz.openbmc_project.State.Host.RestartCause.Unknown"),
        [](const std::string& requested, std::string& resp) {
            if (requested ==
                "xyz.openbmc_project.State.Host.RestartCause.WatchdogTimer")
            {
                addRestartCause(RestartCause::watchdog);
            }
            else
            {
                throw std::invalid_argument(
                    "Unrecognized RestartCause Request");
                return 0;
            }

            lg2::info("RestartCause requested: {RESTART_CAUSE}",
                      "RESTART_CAUSE", requested);
            resp = requested;
            return 1;
        });

    restartCauseIface->initialize();

    currentHostStateMonitor();

    io.run();

    return 0;
}
