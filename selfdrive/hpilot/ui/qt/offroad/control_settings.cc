#include <filesystem>
#include <iostream>

#include "selfdrive/hpilot/ui/qt/offroad/control_settings.h"

namespace fs = std::filesystem;

bool checkCommaNNFFSupport(const std::string &carFingerprint) {
  const std::string filePath = "../car/torque_data/neural_ff_weights.json";

  if (!std::filesystem::exists(filePath)) {
    return false;
  }

  std::ifstream file(filePath);
  std::string line;
  while (std::getline(file, line)) {
    if (line.find(carFingerprint) != std::string::npos) {
      std::cout << "comma's NNFF supports fingerprint: " << carFingerprint << std::endl;
      return true;
    }
  }

  return false;
}

bool checkNNFFLogFileExists(const std::string &carFingerprint) {
  const fs::path dirPath("../car/torque_data/lat_models");

  if (!fs::exists(dirPath)) {
    std::cerr << "Directory does not exist: " << fs::absolute(dirPath) << std::endl;
    return false;
  }

  for (const auto &entry : fs::directory_iterator(dirPath)) {
    if (entry.path().filename().string().find(carFingerprint) == 0) {
      std::cout << "NNFF supports fingerprint: " << entry.path().filename() << std::endl;
      return true;
    }
  }

  return false;
}

HpilotControlsPanel::HpilotControlsPanel(SettingsWindow *parent) : HpilotListWidget(parent) {
  std::string branch = params.get("GitBranch");
  isStaging = branch == "Hpilot-dev" || branch == "Hpilot";

  const std::vector<std::tuple<QString, QString, QString, QString>> controlToggles {
    {"AlwaysOnLateral", tr("Always on Lateral"), tr("Maintain openpilot lateral control when the brake or gas pedals are used.\n\nDeactivation occurs only through the 'Cruise Control' button."), "../hpilot/assets/toggle_icons/icon_always_on_lateral.png"},
    {"AlwaysOnLateralMain", tr("Enable On Cruise Main"), tr("Enable 'Always On Lateral' by clicking your 'Cruise Control' button without requring openpilot to be enabled first."), ""},
    {"PauseAOLOnBrake", tr("Pause On Brake"), tr("Pause 'Always On Lateral' when the brake pedal is being pressed."), ""},
    {"HideAOLStatusBar", tr("Hide the Status Bar"), tr("Don't use the status bar for 'Always On Lateral'."), ""},

    {"ConditionalExperimental", tr("Conditional Experimental Mode"), tr("Automatically switches to 'Experimental Mode' under predefined conditions."), "../hpilot/assets/toggle_icons/icon_conditional.png"},
    {"CECurves", tr("Curve Detected Ahead"), tr("Switch to 'Experimental Mode' when a curve is detected."), ""},
    {"CENavigation", tr("Navigation Based"), tr("Switch to 'Experimental Mode' based on navigation data. (i.e. Intersections, stop signs, upcoming turns, etc.)"), ""},
    {"CESlowerLead", tr("Slower Lead Detected Ahead"), tr("Switch to 'Experimental Mode' when a slower lead vehicle is detected ahead."), ""},
    {"CEStopLights", tr("Stop Lights and Stop Signs"), tr("Switch to 'Experimental Mode' when a stop light or stop sign is detected."), ""},
    {"CESignal", tr("Turn Signal When Below Highway Speeds"), tr("Switch to 'Experimental Mode' when using turn signals below highway speeds to help assist with turns."), ""},
    {"HideCEMStatusBar", tr("Hide the Status Bar"), tr("Don't use the status bar for 'Conditional Experimental Mode'."), ""},

    {"CustomPersonalities", tr("Custom Driving Personalities"), tr("Customize the driving personality profiles to your driving style."), "../hpilot/assets/toggle_icons/icon_custom.png"},

    {"DeviceManagement", tr("Device Management"), tr("Tweak your device's behaviors to your personal preferences."), "../hpilot/assets/toggle_icons/icon_device.png"},
    {"MuteOverheated", tr("Bypass Thermal Safety Limits"), tr("Allow the device to run at any temperature even above comma's recommended thermal limits."), ""},
    {"DeviceShutdown", tr("Device Shutdown Timer"), tr("Configure how quickly the device shuts down after going offroad."), ""},
    {"NoLogging", tr("Disable Logging"), tr("Turn off all data tracking to enhance privacy or reduce thermal load."), ""},
    {"NoUploads", tr("Disable Uploads"), tr("Turn off all data uploads to comma's servers."), ""},
    {"LowVoltageShutdown", tr("Low Voltage Shutdown Threshold"), tr("Automatically shut the device down when your battery reaches a specific voltage level to prevent killing your battery."), ""},
    {"OfflineMode", tr("Offline Mode"), tr("Allow the device to be offline indefinitely."), ""},

    {"ExperimentalModeActivation", tr("Experimental Mode Activation"), tr("Toggle Experimental Mode with either buttons on the steering wheel or the screen.\n\nOverrides 'Conditional Experimental Mode'."), "../assets/img_experimental_white.svg"},
    {"ExperimentalModeViaLKAS", tr("Double Clicking the LKAS Button"), tr("Enable/disable 'Experimental Mode' by double clicking the 'LKAS' button on your steering wheel."), ""},
    {"ExperimentalModeViaTap", tr("Double Tapping the Onroad UI"), tr("Enable/disable 'Experimental Mode' by double tapping the onroad UI within a 0.5 second time frame."), ""},
    {"ExperimentalModeViaDistance", tr("Long Pressing the Distance Button"), tr("Enable/disable 'Experimental Mode' by holding down the 'distance' button on your steering wheel for 0.5 seconds."), ""},

    {"LateralTune", tr("Lateral Tuning"), tr("Modify openpilot's steering behavior."), "../hpilot/assets/toggle_icons/icon_lateral_tune.png"},
    {"ForceAutoTune", tr("Force Auto Tune"), tr("Forces comma's auto lateral tuning for unsupported vehicles."), ""},
    {"NNFF", tr("NNFF"), tr("Use Twilsonco's Neural Network Feedforward for enhanced precision in lateral control."), ""},
    {"NNFFLite", tr("NNFF-Lite"), tr("Use Twilsonco's Neural Network Feedforward for enhanced precision in lateral control for cars without available NNFF logs."), ""},
    {"SteerRatio", steerRatioStock != 0 ? QString(tr("Steer Ratio (Default: %1)")).arg(QString::number(steerRatioStock, 'f', 2)) : tr("Steer Ratio"), tr("Use a custom steer ratio as opposed to comma's auto tune value."), ""},
    {"TurnDesires", tr("Use Turn Desires"), tr("Use turn desires for greater precision in turns below the minimum lane change speed."), ""},

    {"CustomOfflineParams", tr("Custom Offline Tune"), tr("Set custom offline lateral acceleration and friction values for the torque controller. Overrides the default values in params.toml."), ""},
    {"CustomLiveParams", tr("Custom Live Tune"), tr("Set live tuning values that will be used immediately."), ""},
    {"OfflineLatAccel", tr("Offline Lat. Accel. Factor"), tr("Set a custom lateral acceleration factor value."), ""},
    {"OfflineFriction", tr("Offline Friction Coefficient"), tr("Set a custom friction coefficient value."), ""},
    {"LiveLatAccel", tr("Live Lat. Accel."), tr("Set a custom live lateral acceleration factor value."), ""},
    {"LiveFriction", tr("Live Friction"), tr("Set a custom live friction coefficient value"), ""},

    {"CustomTorque", tr("Override Torque Values"), tr("Override the default steering torque values."), ""},
    {"SteerMax", tr("Steer Max (Default: 270)"), tr("Adjust the maximum steering torque openpilot can apply."), ""},
    {"DeltaUp", tr("Delta Up (Default: 2)"), tr("Adjust how quickly the steering torque is ramped up."), ""},
    {"DeltaDown", tr("Delta Down (Default: 3)"), tr("Adjust how quickly the steering torque is ramped down."), ""},
    {"DriverAllowance", tr("Driver Allowance (Default: 250)"), tr("Adjust the driver torque allowance."), ""},
    {"SteerThreshold", tr("Steer Threshold (Default: 250)"), tr("Adjust the steering torque threshold."), ""},

    {"LongitudinalTune", tr("Longitudinal Tuning"), tr("Modify openpilot's acceleration and braking behavior."), "../hpilot/assets/toggle_icons/icon_longitudinal_tune.png"},
    {"AccelerationProfile", tr("Acceleration Profile"), tr("Change the acceleration rate to be either sporty or eco-friendly."), ""},
    {"DecelerationProfile", tr("Deceleration Profile"), tr("Change the deceleration rate to be either sporty or eco-friendly."), ""},
    {"AggressiveAcceleration", tr("Increase Acceleration Behind Faster Lead"), tr("Increase aggressiveness when following a faster lead."), ""},
    {"StoppingDistance", tr("Increase Stop Distance Behind Lead"), tr("Increase the stopping distance for a more comfortable stop from lead vehicles."), ""},
    {"LeadDetectionThreshold", tr("Lead Detection Threshold"), tr("Increase or decrease the lead detection threshold to either detect leads sooner, or increase model confidence."), ""},
    {"SmoothBraking", tr("Smoother Braking"), tr("Smoothen out the braking behavior when approaching slower vehicles."), ""},
    {"TrafficMode", tr("Traffic Mode"), tr("Enable the ability to activate 'Traffic Mode' by holding down the 'distance' button for 2.5 seconds. When 'Traffic Mode' is active the onroad UI will turn red and openpilot will drive catered towards stop and go traffic."), ""},

    {"MTSCEnabled", tr("Map Turn Speed Control"), tr("Slow down for anticipated curves detected by the downloaded maps."), "../hpilot/assets/toggle_icons/icon_speed_map.png"},
    {"DisableMTSCSmoothing", tr("Disable MTSC UI Smoothing"), tr("Disables the smoothing for the requested speed in the onroad UI to show exactly what speed MTSC is currently requesting."), ""},
    {"MTSCCurvatureCheck",  tr("Model Curvature Detection Failsafe"), tr("Only trigger MTSC when the model detects a curve in the road. Purely used as a failsafe to prevent false positives. Leave this off if you never experience false positives."), ""},
    {"MTSCAggressiveness", tr("Turn Speed Aggressiveness"), tr("Set turn speed aggressiveness. Higher values result in faster turns, lower values yield gentler turns.\n\nA change of +- 1% results in the speed being raised or lowered by about 1 mph."), ""},

    {"ModelSelector", tr("Model Selector"), tr("Manage openpilot's driving models."), "../assets/offroad/icon_calibration.png"},

    {"NudgelessLaneChange", tr("Nudgeless Lane Change"), tr("Enable lane changes without requiring manual steering input."), "../hpilot/assets/toggle_icons/icon_lane.png"},
    {"LaneChangeTime", tr("Lane Change Timer"), tr("Set a delay before executing a nudgeless lane change."), ""},
    {"LaneDetectionWidth", tr("Lane Detection Threshold"), tr("Set the required lane width to be qualified as a lane."), ""},
    {"OneLaneChange", tr("One Lane Change Per Signal"), tr("Only allow one nudgeless lane change per turn signal activation."), ""},

    {"QOLControls", tr("Quality of Life"), tr("Miscellaneous quality of life changes to improve your overall openpilot experience."), "../hpilot/assets/toggle_icons/quality_of_life.png"},
    {"CustomCruise", tr("Cruise Increase Interval"), tr("Set a custom interval to increase the max set speed by."), ""},
    {"DisableOnroadUploads", tr("Disable Onroad Uploads"), tr("Prevent uploads to comma connect unless you're offroad and connected to Wi-Fi."), ""},
    {"HigherBitrate", tr("Higher Bitrate Recording"), tr("Increases the quality of the footage uploaded to comma connect."), ""},
    {"OnroadDistanceButton", tr("Onroad Distance Button"), tr("Simulate a distance button via the onroad UI to control personalities, 'Experimental Mode', and 'Traffic Mode'."), ""},
    {"PauseLateralSpeed", tr("Pause Lateral Below"), tr("Pause lateral control on all speeds below the set speed."), ""},
    {"PauseLateralOnSignal", tr("Pause Lateral On Turn Signal Below"), tr("Pause lateral control when using a turn signal below the set speed."), ""},
    {"ReverseCruise", tr("Reverse Cruise Increase"), tr("Reverses the 'long press' functionality logic to increase the max set speed by 5 instead of 1. Useful to increase the max speed quickly."), ""},
    {"SetSpeedOffset", tr("Set Speed Offset"), tr("Set an offset for your desired set speed."), ""},

    {"SpeedLimitController", tr("Speed Limit Controller"), tr("Automatically adjust the max speed to match the current speed limit using 'Open Street Maps', 'Navigate On openpilot', or your car's dashboard (Toyotas/Lexus/HKG only)."), "../assets/offroad/icon_speed_limit.png"},
    {"SLCControls", tr("Controls Settings"), tr("Manage toggles related to 'Speed Limit Controller's controls."), ""},
    {"Offset1", tr("Speed Limit Offset (0-34 mph)"), tr("Speed limit offset for speed limits between 0-34 mph."), ""},
    {"Offset2", tr("Speed Limit Offset (35-54 mph)"), tr("Speed limit offset for speed limits between 35-54 mph."), ""},
    {"Offset3", tr("Speed Limit Offset (55-64 mph)"), tr("Speed limit offset for speed limits between 55-64 mph."), ""},
    {"Offset4", tr("Speed Limit Offset (65-99 mph)"), tr("Speed limit offset for speed limits between 65-99 mph."), ""},
    {"SLCFallback", tr("Fallback Method"), tr("Choose your fallback method when there is no speed limit available."), ""},
    {"SLCOverride", tr("Override Method"), tr("Choose your preferred method to override the current speed limit."), ""},
    {"SLCPriority", tr("Priority Order"), tr("Configure the speed limit priority order."), ""},
    {"SLCQOL", tr("Quality of Life Settings"), tr("Manage toggles related to 'Speed Limit Controller's quality of life features."), ""},
    {"SLCConfirmation", tr("Confirm New Speed Limits"), tr("Don't automatically start using the new speed limit until it's been manually confirmed."), ""},
    {"ForceMPHDashboard", tr("Force MPH From Dashboard Readings"), tr("Force MPH readings from the dashboard. Only use this if you live in an area where the speed limits from your dashboard are in KPH, but you use MPH."), ""},
    {"SLCLookahead", tr("Prepare For Upcoming Speed Limits"), tr("Set a 'lookahead' value to prepare for upcoming speed limit changes using the data stored in 'Open Street Maps'."), ""},
    {"SetSpeedLimit", tr("Use Current Speed Limit As Set Speed"), tr("Sets your max speed to the current speed limit if one is populated when you initially enable openpilot."), ""},
    {"SLCVisuals", tr("Visuals Settings"), tr("Manage toggles related to 'Speed Limit Controller's visuals."), ""},
    {"ShowSLCOffset", tr("Show Speed Limit Offset"), tr("Show the speed limit offset separated from the speed limit in the onroad UI when using 'Speed Limit Controller'."), ""},
    {"UseVienna", tr("Use Vienna Speed Limit Signs"), tr("Use the Vienna (EU) speed limit style signs as opposed to MUTCD (US)."), ""},

    {"VisionTurnControl", tr("Vision Turn Speed Controller"), tr("Slow down for detected curves in the road."), "../hpilot/assets/toggle_icons/icon_vtc.png"},
    {"DisableVTSCSmoothing", tr("Disable VTSC UI Smoothing"), tr("Disables the smoothing for the requested speed in the onroad UI."), ""},
    {"CurveSensitivity", tr("Curve Detection Sensitivity"), tr("Set curve detection sensitivity. Higher values prompt earlier responses, lower values lead to smoother but later reactions."), ""},
    {"TurnAggressiveness", tr("Turn Speed Aggressiveness"), tr("Set turn speed aggressiveness. Higher values result in faster turns, lower values yield gentler turns."), ""},
  };

  for (const auto &[param, title, desc, icon] : controlToggles) {
    ParamControl *toggle;

    if (param == "AlwaysOnLateral") {
      HpilotParamManageControl *aolToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(aolToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(aolKeys.find(key.c_str()) != aolKeys.end());
        }
      });
      toggle = aolToggle;

    } else if (param == "ConditionalExperimental") {
      HpilotParamManageControl *conditionalExperimentalToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(conditionalExperimentalToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        conditionalSpeedsImperial->setVisible(!isMetric);
        conditionalSpeedsMetric->setVisible(isMetric);
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end());
        }
      });
      toggle = conditionalExperimentalToggle;
    } else if (param == "CECurves") {
      HpilotParamValueControl *CESpeedImperial = new HpilotParamValueControl("CESpeed", tr("Below"), tr("Switch to 'Experimental Mode' below this speed when not following a lead vehicle."), "", 0, 99,
                                                                                   std::map<int, QString>(), this, false, tr(" mph"));
      HpilotParamValueControl *CESpeedLeadImperial = new HpilotParamValueControl("CESpeedLead", tr("  w/Lead"), tr("Switch to 'Experimental Mode' below this speed when following a lead vehicle."), "", 0, 99,
                                                                                       std::map<int, QString>(), this, false, tr(" mph"));
      conditionalSpeedsImperial = new HpilotDualParamControl(CESpeedImperial, CESpeedLeadImperial, this);
      addItem(conditionalSpeedsImperial);

      HpilotParamValueControl *CESpeedMetric = new HpilotParamValueControl("CESpeed", tr("Below"), tr("Switch to 'Experimental Mode' below this speed in absence of a lead vehicle."), "", 0, 150,
                                                                                 std::map<int, QString>(), this, false, tr(" kph"));
      HpilotParamValueControl *CESpeedLeadMetric = new HpilotParamValueControl("CESpeedLead", tr("  w/Lead"), tr("Switch to 'Experimental Mode' below this speed when following a lead vehicle."), "", 0, 150,
                                                                                     std::map<int, QString>(), this, false, tr(" kph"));
      conditionalSpeedsMetric = new HpilotDualParamControl(CESpeedMetric, CESpeedLeadMetric, this);
      addItem(conditionalSpeedsMetric);

      std::vector<QString> curveToggles{"CECurvesLead"};
      std::vector<QString> curveToggleNames{tr("With Lead")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, curveToggles, curveToggleNames);
    } else if (param == "CENavigation") {
      std::vector<QString> navigationToggles{"CENavigationIntersections", "CENavigationTurns", "CENavigationLead"};
      std::vector<QString> navigationToggleNames{tr("Intersections"), tr("Turns"), tr("With Lead")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, navigationToggles, navigationToggleNames);
    } else if (param == "CEStopLights") {
      std::vector<QString> stopLightToggles{"CEStopLightsLead"};
      std::vector<QString> stopLightToggleNames{tr("With Lead")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, stopLightToggles, stopLightToggleNames);

    } else if (param == "CustomPersonalities") {
      HpilotParamManageControl *customPersonalitiesToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customPersonalitiesToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(false);
        }
        aggressiveProfile->setVisible(true);
        standardProfile->setVisible(true);
        relaxedProfile->setVisible(true);
        trafficProfile->setVisible(isStaging && params.getBool("TrafficMode"));
      });
      toggle = customPersonalitiesToggle;

      HpilotParamValueControl *trafficFollow = new HpilotParamValueControl("TrafficFollow", tr("Follow"),
                                                                              tr("Set the minimum following distance when using 'Traffic Mode'. Your following distance will dynamically "
                                                                                 "adjust between this distance and the following distance from the 'Aggressive' profile when driving between "
                                                                                 "0 and %1.\n\nFor example:\n\nTraffic Mode: 0.5s\nAggressive: 1.0s\n\n0%2 = 0.5s\n%3 = 0.75s\n%1 = 1.0s")
                                                                                 .arg(isMetric ? "90ph" : "55mph")
                                                                                 .arg(isMetric ? "kph" : "mph")
                                                                                 .arg(isMetric ? "45kph" : "27.5mph"),
                                                                                 "../hpilot/assets/other_images/traffic.png",
                                                                                 0.5, 5, std::map<int, QString>(), this, false, tr(" sec"), 1, 0.01);
      HpilotParamValueControl *trafficJerk = new HpilotParamValueControl("TrafficJerk", tr("Jerk"),
                                                                            tr("Set the minimum jerk value when using 'Traffic Mode'. Your jerk will dynamically "
                                                                               "adjust between this jerk and the jerk from the 'Aggressive' profile when driving between "
                                                                               "0 and %1.\n\nFor example:\n\nTraffic Mode: 0.1\nAggressive: 1.0\n\n0%2 = 0.1\n%3 = 0.5\n%1 = 1.0")
                                                                               .arg(isMetric ? "90ph" : "55mph")
                                                                               .arg(isMetric ? "kph" : "mph")
                                                                               .arg(isMetric ? "45kph" : "27.5mph"),
                                                                               "",
                                                                               0.01, 5, std::map<int, QString>(), this, false, "", 1, 0.01);
      trafficProfile = new HpilotDualParamControl(trafficFollow, trafficJerk, this, true);
      addItem(trafficProfile);

      HpilotParamValueControl *aggressiveFollow = new HpilotParamValueControl("AggressiveFollow", tr("Follow"),
                                                                                 tr("Set the 'Aggressive' personality' following distance. "
                                                                                    "Represents seconds to follow behind the lead vehicle.\n\nStock: 1.25 seconds."),
                                                                                    "../hpilot/assets/other_images/aggressive.png",
                                                                                    0.10, 5, std::map<int, QString>(), this, false, tr(" sec"), 1, 0.01);
      HpilotParamValueControl *aggressiveJerk = new HpilotParamValueControl("AggressiveJerk", tr("Jerk"),
                                                                               tr("Adjust brake/gas pedal responsiveness for the 'Aggressive' personality. "
                                                                                  "Higher jerk value = Less likely to use the gas/brake.\nLower jerk value = More likely.\n\nStock: 0.5."),
                                                                                  "",
                                                                                  0.01, 5, std::map<int, QString>(), this, false, "", 1, 0.01);
      aggressiveProfile = new HpilotDualParamControl(aggressiveFollow, aggressiveJerk, this, true);
      addItem(aggressiveProfile);

      HpilotParamValueControl *standardFollow = new HpilotParamValueControl("StandardFollow", tr("Follow"),
                                                                               tr("Set the 'Standard' personality following distance. "
                                                                                  "Represents seconds to follow behind the lead vehicle.\n\nStock: 1.45 seconds."),
                                                                                  "../hpilot/assets/other_images/standard.png",
                                                                                  1, 5, std::map<int, QString>(), this, false, tr(" sec"), 1, 0.01);
      HpilotParamValueControl *standardJerk = new HpilotParamValueControl("StandardJerk", tr("Jerk"),
                                                                             tr("Adjust brake/gas pedal responsiveness for the 'Standard' personality. "
                                                                                "Higher jerk value = Less likely to use the gas/brake.\nLower jerk value = More likely.\n\nStock: 1.0."),
                                                                                "",
                                                                                0.01, 5, std::map<int, QString>(), this, false, "", 1, 0.01);
      standardProfile = new HpilotDualParamControl(standardFollow, standardJerk, this, true);
      addItem(standardProfile);

      HpilotParamValueControl *relaxedFollow = new HpilotParamValueControl("RelaxedFollow", tr("Follow"),
                                                                              tr("Set the 'Relaxed' personality following distance. "
                                                                                 "Represents seconds to follow behind the lead vehicle.\n\nStock: 1.75 seconds."),
                                                                                 "../hpilot/assets/other_images/relaxed.png",
                                                                                 1, 5, std::map<int, QString>(), this, false, tr(" sec"), 1, 0.01);
      HpilotParamValueControl *relaxedJerk = new HpilotParamValueControl("RelaxedJerk", tr("Jerk"),
                                                                            tr("Adjust brake/gas pedal responsiveness for the 'Relaxed' personality. "
                                                                               "Higher jerk value = Less likely to use the gas/brake.\nLower jerk value = More likely.\n\nStock: 1.0."),
                                                                               "",
                                                                               0.01, 5, std::map<int, QString>(), this, false, "", 1, 0.01);
      relaxedProfile = new HpilotDualParamControl(relaxedFollow, relaxedJerk, this, true);
      addItem(relaxedProfile);

    } else if (param == "DeviceManagement") {
      HpilotParamManageControl *deviceManagementToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(deviceManagementToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(deviceManagementKeys.find(key.c_str()) != deviceManagementKeys.end());
        }
      });
      toggle = deviceManagementToggle;
    } else if (param == "DeviceShutdown") {
      std::map<int, QString> shutdownLabels;
      for (int i = 0; i <= 33; ++i) {
        shutdownLabels[i] = i == 0 ? tr("5 mins") : i <= 3 ? QString::number(i * 15) + tr(" mins") : QString::number(i - 3) + (i == 4 ? tr(" hour") : tr(" hours"));
      }
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 33, shutdownLabels, this, false);
    } else if (param == "LowVoltageShutdown") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 11.8, 12.5, std::map<int, QString>(), this, false, tr(" volts"), 1, 0.01);

    } else if (param == "ExperimentalModeActivation") {
      HpilotParamManageControl *experimentalModeActivationToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(experimentalModeActivationToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(experimentalModeActivationKeys.find(key.c_str()) != experimentalModeActivationKeys.end());
        }
      });
      toggle = experimentalModeActivationToggle;

    } else if (param == "LateralTune") {
      HpilotParamManageControl *lateralTuneToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(lateralTuneToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedLateralTuneKeys = lateralTuneKeys;

          if (hasAutoTune) {
            modifiedLateralTuneKeys.erase("ForceAutoTune");
          }

          if (hasCommaNNFFSupport) {
            modifiedLateralTuneKeys.erase("NNFF");
            modifiedLateralTuneKeys.erase("NNFFLite");
          } else if (hasNNFFLog) {
            modifiedLateralTuneKeys.erase("NNFFLite");
          } else {
            modifiedLateralTuneKeys.erase("NNFF");
          }

          toggle->setVisible(modifiedLateralTuneKeys.find(key.c_str()) != modifiedLateralTuneKeys.end());
        }
      });
      toggle = lateralTuneToggle;
    } else if (param == "SteerRatio") {
      std::vector<QString> steerRatioToggles{"ResetSteerRatio"};
      std::vector<QString> steerRatioToggleNames{"Reset"};
      toggle = new HpilotParamValueToggleControl(param, title, desc, icon, steerRatioStock * 0.75, steerRatioStock * 1.25, std::map<int, QString>(), this, false, "", 1, 0.01, steerRatioToggles, steerRatioToggleNames);
    } else if (param == "CustomOfflineParams") {
      HpilotParamManageControl *customTuneToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customTuneToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customTuneKeys.find(key.c_str()) != customTuneKeys.end());
        }
      });
      toggle = customTuneToggle;
    } else if (param == "OfflineLatAccel") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0.0, 5.0, std::map<int, QString>(), this, false, "", 1, 0.01);
    } else if (param == "OfflineFriction") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0.0, 1.0, std::map<int, QString>(), this, false, "", 1, 0.01);
    } else if (param == "CustomLiveParams") {
      HpilotParamManageControl *liveTuneToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(liveTuneToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(liveTuneKeys.find(key.c_str()) != liveTuneKeys.end());
        }
      });
      toggle = liveTuneToggle;
    } else if (param == "LiveLatAccel") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0.0, 5.0, std::map<int, QString>(), this, false, "", 1, 0.01);
    } else if (param == "LiveFriction") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0.0, 1.0, std::map<int, QString>(), this, false, "", 1, 0.01);
    } else if (param == "CustomTorque") {
      HpilotParamManageControl *customTorqueToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customTorqueToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customTorqueKeys.find(key.c_str()) != customTorqueKeys.end());
        }
      });
      toggle = customTorqueToggle;
    } else if (param == "SteerMax") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 200, 409, std::map<int, QString>(), this, false, "");
    } else if (param == "DeltaUp") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 1, 7, std::map<int, QString>(), this, false, "");
    } else if (param == "DeltaDown") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 1, 15, std::map<int, QString>(), this, false, "");
    } else if (param == "DriverAllowance") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 50, 450, std::map<int, QString>(), this, false, "");
    } else if (param == "SteerThreshold") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 50, 450, std::map<int, QString>(), this, false, "");
    } else if (param == "LongitudinalTune") {
      HpilotParamManageControl *longitudinalTuneToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(longitudinalTuneToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(longitudinalTuneKeys.find(key.c_str()) != longitudinalTuneKeys.end());
        }
      });
      toggle = longitudinalTuneToggle;
    } else if (param == "AccelerationProfile") {
      std::vector<QString> profileOptions{tr("Standard"), tr("Eco"), tr("Sport"), tr("Sport+")};
      HpilotButtonParamControl *profileSelection = new HpilotButtonParamControl(param, title, desc, icon, profileOptions);
      toggle = profileSelection;

      QObject::connect(static_cast<HpilotButtonParamControl*>(toggle), &HpilotButtonParamControl::buttonClicked, [this](int id) {
        if (id == 3) {
          HpilotConfirmationDialog::toggleAlert(tr("WARNING: This maxes out openpilot's acceleration from 2.0 m/s to 4.0 m/s and may cause oscillations when accelerating!"),
          tr("I understand the risks."), this);
        }
      });
    } else if (param == "DecelerationProfile") {
      std::vector<QString> profileOptions{tr("Standard"), tr("Eco"), tr("Sport")};
      HpilotButtonParamControl *profileSelection = new HpilotButtonParamControl(param, title, desc, icon, profileOptions);
      toggle = profileSelection;
    } else if (param == "StoppingDistance") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 10, std::map<int, QString>(), this, false, tr(" feet"));
    } else if (param == "LeadDetectionThreshold") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 1, 100, std::map<int, QString>(), this, false, "%");
    } else if (param == "SmoothBraking" && isStaging) {
      std::vector<QString> brakingToggles{"SmoothBrakingJerk", "SmoothBrakingFarLead"};
      std::vector<QString> brakingToggleNames{tr("Apply to Jerk"), tr("Far Lead Offset")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, brakingToggles, brakingToggleNames);
      QObject::connect(static_cast<HpilotParamToggleControl*>(toggle), &HpilotParamToggleControl::buttonClicked, [this](bool checked) {
        if (checked) {
          HpilotConfirmationDialog::toggleAlert(
          tr("WARNING: This is very experimental and may cause the car to not brake or stop safely! Please report any issues in the FrogPilot Discord!"),
          tr("I understand the risks."), this);
        }
      });

    } else if (param == "MTSCEnabled") {
      HpilotParamManageControl *mtscToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(mtscToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(mtscKeys.find(key.c_str()) != mtscKeys.end());
        }
      });
      toggle = mtscToggle;
    } else if (param == "MTSCAggressiveness") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 1, 200, std::map<int, QString>(), this, false, "%");

    } else if (param == "ModelSelector") {
      HpilotParamManageControl *modelsToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(modelsToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(false);
        }

        deleteModelBtn->setVisible(true);
        downloadModelBtn->setVisible(true);
        selectModelBtn->setVisible(true);
      });
      toggle = modelsToggle;

      deleteModelBtn = new ButtonControl(tr("Delete Model"), tr("DELETE"), "");
      QObject::connect(deleteModelBtn, &ButtonControl::clicked, [this]() {
        QDir modelDir("/data/models/");

        QStringList availableModels = QString::fromStdString(params.get("AvailableModels")).split(",");
        QStringList modelLabels = QString::fromStdString(params.get("AvailableModelsNames")).split(",");

        QString defaultModelFileName;
        QMap<QString, QString> labelToFileMap;
        QStringList existingModelFiles = modelDir.entryList({"*.thneed"}, QDir::Files);

        for (int i = 0; i < availableModels.size(); ++i) {
          QString modelFileName = availableModels.at(i) + ".thneed";
          if (existingModelFiles.contains(modelFileName)) {
            QString readableName = modelLabels.at(i);
            if (readableName.endsWith("(Default)")) {
              defaultModelFileName = modelFileName;
            }
            labelToFileMap[readableName] = modelFileName;
          }
        }

        QStringList deletableModelLabels;
        for (const QString &modelLabel : modelLabels) {
          if (labelToFileMap.contains(modelLabel)) {
            deletableModelLabels.append(modelLabel);
          }
        }

        QString selectedModel = MultiOptionDialog::getSelection(tr("Select a model to delete"), deletableModelLabels, "", this);
        if (!selectedModel.isEmpty()) {
          if (ConfirmationDialog::confirm(tr("Are you sure you want to delete this model?"), tr("Delete"), this)) {
            std::thread([=]() {
              deleteModelBtn->setValue(tr("Deleting..."));

              deleteModelBtn->setEnabled(false);
              downloadModelBtn->setEnabled(false);
              selectModelBtn->setEnabled(false);

              QString modelToDelete = labelToFileMap[selectedModel];
              QString currentModel = QString::fromStdString(params.get("Model")) + ".thneed";

              QFile::remove(modelDir.absoluteFilePath(modelToDelete));

              if (modelToDelete == currentModel) {
                params.put("Model", "wd-40");
                selectModelBtn->setValue("WD-40 (Default)");
              }

              deleteModelBtn->setEnabled(true);
              downloadModelBtn->setEnabled(true);
              selectModelBtn->setEnabled(true);

              deleteModelBtn->setValue(tr("Deleted!"));
              std::this_thread::sleep_for(std::chrono::seconds(3));
              deleteModelBtn->setValue("");
            }).detach();
          }
        }
      });
      addItem(deleteModelBtn);

      downloadModelBtn = new ButtonControl(tr("Download Model"), tr("DOWNLOAD"), "");
      QObject::connect(downloadModelBtn, &ButtonControl::clicked, [this]() {
        QDir modelDir("/data/models/");

        QStringList availableModels = QString::fromStdString(params.get("AvailableModels")).split(",");
        QStringList modelLabels = QString::fromStdString(params.get("AvailableModelsNames")).split(",");

        QMap<QString, QString> labelToModelMap;
        QStringList downloadableModelLabels;
        QStringList existingModelFiles = modelDir.entryList({"*.thneed"}, QDir::Files);

        for (int i = 0; i < availableModels.size(); ++i) {
          QString modelFileName = availableModels.at(i) + ".thneed";
          if (!existingModelFiles.contains(modelFileName)) {
            QString readableName = modelLabels.at(i);
            if (!readableName.endsWith("(Default)")) {
              downloadableModelLabels.append(readableName);
              labelToModelMap.insert(readableName, availableModels.at(i));
            }
          }
        }

        QString modelToDownload = MultiOptionDialog::getSelection(tr("Select a driving model to download"), downloadableModelLabels, "", this);
        if (!modelToDownload.isEmpty()) {
          QString selectedModelValue = labelToModelMap.value(modelToDownload);
          paramsMemory.put("ModelToDownload", selectedModelValue.toStdString());

          deleteModelBtn->setEnabled(false);
          downloadModelBtn->setEnabled(false);
          selectModelBtn->setEnabled(false);

          QTimer *progressTimer = new QTimer(this);
          progressTimer->setInterval(250);
          connect(progressTimer, &QTimer::timeout, this, [this, progressTimer]() {
            int progress = paramsMemory.getInt("ModelDownloadProgress");

            if (progress < 100) {
              downloadModelBtn->setValue(QString::number(progress) + "%");

            } else {
              deleteModelBtn->setEnabled(true);
              downloadModelBtn->setEnabled(true);
              selectModelBtn->setEnabled(true);

              downloadModelBtn->setValue(tr("Downloaded!"));
              progressTimer->stop();
              progressTimer->deleteLater();
              QTimer::singleShot(3000, this, [this]() {
                downloadModelBtn->setValue("");
              });
            }
          });
          progressTimer->start();
        }
      });
      addItem(downloadModelBtn);

      selectModelBtn = new ButtonControl(tr("Select Model"), tr("SELECT"), "");
      QObject::connect(selectModelBtn, &ButtonControl::clicked, [this]() {
        QDir modelDir("/data/models/");

        QStringList availableModels = QString::fromStdString(params.get("AvailableModels")).split(",");
        QStringList modelLabels = QString::fromStdString(params.get("AvailableModelsNames")).split(",");

        QStringList modelFiles = modelDir.entryList({"*.thneed"}, QDir::Files);
        QStringList modelFilesBaseNames;
        for (const QString &modelFile : modelFiles) {
          modelFilesBaseNames.append(modelFile.section('.', 0, 0));
        }

        QStringList selectableModelLabels;

        for (int i = 0; i < availableModels.size(); ++i) {
          QString baseName = availableModels.at(i);
          QString label = modelLabels.at(i);
          if (modelFilesBaseNames.contains(baseName) || label.endsWith("(Default)")) {
            selectableModelLabels.append(label);
          }
        }

        QString modelToSelect = MultiOptionDialog::getSelection(tr("Select a model"), selectableModelLabels, "", this);
        if (!modelToSelect.isEmpty()) {
          params.put("ModelName", modelToSelect.toStdString());
          selectModelBtn->setValue(modelToSelect);

          int modelIndex = modelLabels.indexOf(modelToSelect);
          if (modelIndex != -1) {
            QString selectedModel = availableModels.at(modelIndex);
            params.put("Model", selectedModel.toStdString());
          }

          if (HpilotConfirmationDialog::yesorno(tr("Do you want to start with a fresh calibration for the newly selected model?"), this)) {
            params.remove("CalibrationParams");
            params.remove("LiveTorqueParameters");
          }

          if (started) {
            if (HpilotConfirmationDialog::toggle(tr("Reboot required to take effect."), tr("Reboot Now"), this)) {
              Hardware::reboot();
            }
          }
        }
      });
      addItem(selectModelBtn);
      selectModelBtn->setValue(QString::fromStdString(params.get("ModelName")));

    } else if (param == "QOLControls") {
      HpilotParamManageControl *qolToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(qolToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedQolKeys = qolKeys;

          if (!hasOpenpilotLongitudinal) {
            modifiedQolKeys.erase("OnroadDistanceButton");
          }

          if (!hasPCMCruise) {
            modifiedQolKeys.erase("ReverseCruise");
          } else {
            modifiedQolKeys.erase("CustomCruise");
            modifiedQolKeys.erase("SetSpeedOffset");
          }

          toggle->setVisible(modifiedQolKeys.find(key.c_str()) != modifiedQolKeys.end());
        }
      });
      toggle = qolToggle;
    } else if (param == "CustomCruise") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 1, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "OnroadDistanceButton") {
      std::vector<QString> onroadDistanceToggles{"KaofuiIcons"};
      std::vector<QString> onroadDistanceToggleNames{tr("Kaofui's Icons")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, onroadDistanceToggles, onroadDistanceToggleNames);
    } else if (param == "PauseLateralSpeed") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "PauseLateralOnSignal") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "ReverseCruise") {
      std::vector<QString> reverseCruiseToggles{"ReverseCruiseUI"};
      std::vector<QString> reverseCruiseNames{tr("Control Via UI")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, reverseCruiseToggles, reverseCruiseNames);
    } else if (param == "SetSpeedOffset") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"));

    } else if (param == "NudgelessLaneChange") {
      HpilotParamManageControl *laneChangeToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(laneChangeToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(laneChangeKeys.find(key.c_str()) != laneChangeKeys.end());
        }
      });
      toggle = laneChangeToggle;
    } else if (param == "LaneChangeTime") {
      std::map<int, QString> laneChangeTimeLabels;
      for (int i = 0; i <= 10; ++i) {
        laneChangeTimeLabels[i] = i == 0 ? "Instant" : QString::number(i / 2.0) + " seconds";
      }
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 10, laneChangeTimeLabels, this, false);
    } else if (param == "LaneDetectionWidth") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, " feet", 10);

    } else if (param == "SpeedLimitController") {
      HpilotParamManageControl *speedLimitControllerToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(speedLimitControllerToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end());
        }
      });
      toggle = speedLimitControllerToggle;
    } else if (param == "SLCControls") {
      HpilotParamManageControl *manageSLCControlsToggle = new HpilotParamManageControl(param, title, desc, icon, this, true);
      QObject::connect(manageSLCControlsToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerControlsKeys.find(key.c_str()) != speedLimitControllerControlsKeys.end());
          openSubParentToggle();
        }
        slcPriorityButton->setVisible(true);
      });
      toggle = manageSLCControlsToggle;
    } else if (param == "SLCQOL") {
      HpilotParamManageControl *manageSLCQOLToggle = new HpilotParamManageControl(param, title, desc, icon, this, true);
      QObject::connect(manageSLCQOLToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedSpeedLimitControllerQOLKeys = speedLimitControllerQOLKeys;

          if (hasPCMCruise) {
            modifiedSpeedLimitControllerQOLKeys.erase("SetSpeedLimit");
          }

          if (!isToyota) {
            modifiedSpeedLimitControllerQOLKeys.erase("ForceMPHDashboard");
          }

          toggle->setVisible(modifiedSpeedLimitControllerQOLKeys.find(key.c_str()) != modifiedSpeedLimitControllerQOLKeys.end());
          openSubParentToggle();
        }
      });
      toggle = manageSLCQOLToggle;
    } else if (param == "SLCConfirmation") {
      std::vector<QString> slcConfirmationToggles{"SLCConfirmationLower", "SLCConfirmationHigher"};
      std::vector<QString> slcConfirmationNames{tr("Lower Limits"), tr("Higher Limits")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, slcConfirmationToggles, slcConfirmationNames);
    } else if (param == "SLCLookahead") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 0, 60, std::map<int, QString>(), this, false, " seconds");
    } else if (param == "SLCVisuals") {
      HpilotParamManageControl *manageSLCVisualsToggle = new HpilotParamManageControl(param, title, desc, icon, this, true);
      QObject::connect(manageSLCVisualsToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerVisualsKeys.find(key.c_str()) != speedLimitControllerVisualsKeys.end());
          openSubParentToggle();
        }
      });
      toggle = manageSLCVisualsToggle;
    } else if (param == "Offset1" || param == "Offset2" || param == "Offset3" || param == "Offset4") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, -99, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "ShowSLCOffset") {
      std::vector<QString> slcOffsetToggles{"ShowSLCOffsetUI"};
      std::vector<QString> slcOffsetToggleNames{tr("Control Via UI")};
      toggle = new HpilotParamToggleControl(param, title, desc, icon, slcOffsetToggles, slcOffsetToggleNames);
    } else if (param == "SLCFallback") {
      std::vector<QString> fallbackOptions{tr("Set Speed"), tr("Experimental Mode"), tr("Previous Limit")};
      HpilotButtonParamControl *fallbackSelection = new HpilotButtonParamControl(param, title, desc, icon, fallbackOptions);
      toggle = fallbackSelection;
    } else if (param == "SLCOverride") {
      std::vector<QString> overrideOptions{tr("None"), tr("Manual Set Speed"), tr("Set Speed")};
      HpilotButtonParamControl *overrideSelection = new HpilotButtonParamControl(param, title, desc, icon, overrideOptions);
      toggle = overrideSelection;
    } else if (param == "SLCPriority") {
      slcPriorityButton = new ButtonControl(title, tr("SELECT"), desc);
      QStringList primaryPriorities = {tr("None"), tr("Dashboard"), tr("Navigation"), tr("Offline Maps"), tr("Highest"), tr("Lowest")};
      QStringList secondaryTertiaryPriorities = {tr("None"), tr("Dashboard"), tr("Navigation"), tr("Offline Maps")};
      QStringList priorityPrompts = {tr("Select your primary priority"), tr("Select your secondary priority"), tr("Select your tertiary priority")};

      QObject::connect(slcPriorityButton, &ButtonControl::clicked, [=]() {
        QStringList selectedPriorities;

        for (int i = 1; i <= 3; ++i) {
          QStringList currentPriorities = (i == 1) ? primaryPriorities : secondaryTertiaryPriorities;
          QStringList prioritiesToDisplay = currentPriorities;
          for (const auto &selectedPriority : qAsConst(selectedPriorities)) {
            prioritiesToDisplay.removeAll(selectedPriority);
          }

          if (!hasDashSpeedLimits) {
            prioritiesToDisplay.removeAll(tr("Dashboard"));
          }

          if (prioritiesToDisplay.size() == 1 && prioritiesToDisplay.contains(tr("None"))) {
            break;
          }

          QString priorityKey = QString("SLCPriority%1").arg(i);
          QString selection = MultiOptionDialog::getSelection(priorityPrompts[i - 1], prioritiesToDisplay, "", this);

          if (selection.isEmpty()) break;

          params.put(priorityKey.toStdString(), selection.toStdString());
          selectedPriorities.append(selection);

          if (selection == tr("Lowest") || selection == tr("Highest") || selection == tr("None")) break;

          updateToggles();
        }

        selectedPriorities.removeAll(tr("None"));
        slcPriorityButton->setValue(selectedPriorities.join(", "));
      });

      QStringList initialPriorities;
      for (int i = 1; i <= 3; ++i) {
        QString priorityKey = QString("SLCPriority%1").arg(i);
        QString priority = QString::fromStdString(params.get(priorityKey.toStdString()));

        if (!priority.isEmpty() && primaryPriorities.contains(priority) && priority != tr("None")) {
          initialPriorities.append(priority);
        }
      }
      slcPriorityButton->setValue(initialPriorities.join(", "));
      addItem(slcPriorityButton);

    } else if (param == "VisionTurnControl") {
      HpilotParamManageControl *visionTurnControlToggle = new HpilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(visionTurnControlToggle, &HpilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end());
        }
      });
      toggle = visionTurnControlToggle;
    } else if (param == "CurveSensitivity" || param == "TurnAggressiveness") {
      toggle = new HpilotParamValueControl(param, title, desc, icon, 1, 200, std::map<int, QString>(), this, false, "%");

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(toggle, &ToggleControl::toggleFlipped, [this]() {
      updateToggles();
    });

    QObject::connect(static_cast<HpilotButtonParamControl*>(toggle), &HpilotButtonParamControl::buttonClicked, [this]() {
      updateToggles();
    });

    QObject::connect(static_cast<HpilotParamValueControl*>(toggle), &HpilotParamValueControl::valueChanged, [this]() {
      updateToggles();
    });

    QObject::connect(toggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });

    QObject::connect(static_cast<HpilotParamManageControl*>(toggle), &HpilotParamManageControl::manageButtonClicked, this, [this]() {
      update();
    });
  }

  QObject::connect(toggles["AlwaysOnLateralMain"], &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("AlwaysOnLateralMain")) {
      HpilotConfirmationDialog::toggleAlert(
      tr("WARNING: This isn't guaranteed to work, so if you run into any issues, please report it in the Hpilot Discord!"),
      tr("I understand the risks."), this);
    }
  });

  QObject::connect(toggles["MuteOverheated"], &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("MuteOverheated")) {
      HpilotConfirmationDialog::toggleAlert(
      tr("WARNING: This can cause premature wear or damage by running the device over comma's recommended temperature limits!"),
      tr("I understand the risks."), this);
    }
  });

  QObject::connect(toggles["NoLogging"], &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("NoLogging")) {
      HpilotConfirmationDialog::toggleAlert(
      tr("WARNING: This will prevent your drives from being recorded and the data will be unobtainable!"),
      tr("I understand the risks."), this);
    }
  });

  QObject::connect(toggles["NoUploads"], &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("NoUploads")) {
      HpilotConfirmationDialog::toggleAlert(
      tr("WARNING: This will prevent your drives from appearing on comma connect which may impact debugging and support!"),
      tr("I understand the risks."), this);
    }
  });

  std::set<QString> rebootKeys = {"AlwaysOnLateral", "AlwaysOnLateralMain", "HigherBitrate", "NNFF", "NNFFLite", "PauseAOLOnBrake"};
  for (const QString &key : rebootKeys) {
    QObject::connect(toggles[key.toStdString().c_str()], &ToggleControl::toggleFlipped, [this]() {
      if (started) {
        if (HpilotConfirmationDialog::toggle(tr("Reboot required to take effect."), tr("Reboot Now"), this)) {
          Hardware::reboot();
        }
      }
    });
  }

  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &HpilotControlsPanel::hideToggles);
  QObject::connect(parent, &SettingsWindow::closeSubParentToggle, this, &HpilotControlsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::updateMetric, this, &HpilotControlsPanel::updateMetric);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &HpilotControlsPanel::updateCarToggles);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &HpilotControlsPanel::updateState);

  updateMetric();
}

void HpilotControlsPanel::showEvent(QShowEvent *event, const UIState &s) {
  online = s.scene.online;
  bool parked = s.scene.parked;
  started = s.scene.started;

  downloadModelBtn->setEnabled(online && (!started || parked));

  if (params.getBool("ResetSteerRatio")) {
    HpilotParamValueToggleControl *steerRatioToggle = static_cast<HpilotParamValueToggleControl*>(toggles["SteerRatio"]);
    params.putFloat("SteerRatio", steerRatioStock);
    params.putBool("ResetSteerRatio", false);
    steerRatioToggle->refresh();
  }
}

void HpilotControlsPanel::updateState() {
  if (!isVisible()) return;
}

void HpilotControlsPanel::updateToggles() {
  std::thread([this]() {
    paramsMemory.putBool("HpilotTogglesUpdated", true);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    paramsMemory.putBool("HpilotTogglesUpdated", false);
  }).detach();
}

void HpilotControlsPanel::updateCarToggles() {
  auto carParams = params.get("CarParamsPersistent");
  if (!carParams.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(carParams.data(), carParams.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();
    auto carFingerprint = CP.getCarFingerprint();
    auto carName = CP.getCarName();

    hasAutoTune = (carName == "hyundai" || carName == "toyota") && CP.getLateralTuning().which() == cereal::CarParams::LateralTuning::TORQUE;
    hasCommaNNFFSupport = checkCommaNNFFSupport(carFingerprint);
    hasDashSpeedLimits = carName == "hyundai" || carName == "toyota";
    hasNNFFLog = checkNNFFLogFileExists(carFingerprint);
    hasOpenpilotLongitudinal = CP.getOpenpilotLongitudinalControl() && !params.getBool("DisableOpenpilotLongitudinal");
    hasPCMCruise = CP.getPcmCruise();
    isToyota = carName == "toyota";
    steerRatioStock = CP.getSteerRatio();

    HpilotParamValueToggleControl *steerRatioToggle = static_cast<HpilotParamValueToggleControl*>(toggles["SteerRatio"]);
    steerRatioToggle->setTitle(QString(tr("Steer Ratio (Default: %1)")).arg(QString::number(steerRatioStock, 'f', 2)));
    steerRatioToggle->updateControl(steerRatioStock * 0.75, steerRatioStock * 1.25, "", 0.01);
    steerRatioToggle->refresh();
  } else {
    hasAutoTune = false;
    hasCommaNNFFSupport = false;
    hasDashSpeedLimits = true;
    hasNNFFLog = true;
    hasOpenpilotLongitudinal = true;
    hasPCMCruise = true;
    isToyota = true;
  }

  hideToggles();
}

void HpilotControlsPanel::updateMetric() {
  bool previousIsMetric = isMetric;
  isMetric = params.getBool("IsMetric");

  if (isMetric != previousIsMetric) {
    double distanceConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
    double speedConversion = isMetric ? MILE_TO_KM : KM_TO_MILE;

    params.putIntNonBlocking("CESpeed", std::nearbyint(params.getInt("CESpeed") * speedConversion));
    params.putIntNonBlocking("CESpeedLead", std::nearbyint(params.getInt("CESpeedLead") * speedConversion));
    params.putIntNonBlocking("CustomCruise", std::nearbyint(params.getInt("CustomCruise") * speedConversion));
    params.putIntNonBlocking("LaneDetectionWidth", std::nearbyint(params.getInt("LaneDetectionWidth") * distanceConversion));
    params.putIntNonBlocking("Offset1", std::nearbyint(params.getInt("Offset1") * speedConversion));
    params.putIntNonBlocking("Offset2", std::nearbyint(params.getInt("Offset2") * speedConversion));
    params.putIntNonBlocking("Offset3", std::nearbyint(params.getInt("Offset3") * speedConversion));
    params.putIntNonBlocking("Offset4", std::nearbyint(params.getInt("Offset4") * speedConversion));
    params.putIntNonBlocking("PauseLateralSpeed", std::nearbyint(params.getInt("PauseLateralSpeed") * speedConversion));
    params.putIntNonBlocking("PauseLateralOnSignal", std::nearbyint(params.getInt("PauseLateralOnSignal") * speedConversion));
    params.putIntNonBlocking("SetSpeedOffset", std::nearbyint(params.getInt("SetSpeedOffset") * speedConversion));
    params.putIntNonBlocking("StoppingDistance", std::nearbyint(params.getInt("StoppingDistance") * distanceConversion));
  }

  HpilotParamValueControl *customCruiseToggle = static_cast<HpilotParamValueControl*>(toggles["CustomCruise"]);
  HpilotParamValueControl *laneWidthToggle = static_cast<HpilotParamValueControl*>(toggles["LaneDetectionWidth"]);
  HpilotParamValueControl *offset1Toggle = static_cast<HpilotParamValueControl*>(toggles["Offset1"]);
  HpilotParamValueControl *offset2Toggle = static_cast<HpilotParamValueControl*>(toggles["Offset2"]);
  HpilotParamValueControl *offset3Toggle = static_cast<HpilotParamValueControl*>(toggles["Offset3"]);
  HpilotParamValueControl *offset4Toggle = static_cast<HpilotParamValueControl*>(toggles["Offset4"]);
  HpilotParamValueControl *pauseLateralToggle = static_cast<HpilotParamValueControl*>(toggles["PauseLateralOnSignal"]);
  HpilotParamValueControl *pauseLateralSpeedToggle = static_cast<HpilotParamValueControl*>(toggles["PauseLateralSpeed"]);
  HpilotParamValueControl *setSpeedOffsetToggle = static_cast<HpilotParamValueControl*>(toggles["SetSpeedOffset"]);
  HpilotParamValueControl *stoppingDistanceToggle = static_cast<HpilotParamValueControl*>(toggles["StoppingDistance"]);

  if (isMetric) {
    customCruiseToggle->updateControl(1, 150, tr(" kph"));

    offset1Toggle->setTitle(tr("Speed Limit Offset (0-34 kph)"));
    offset2Toggle->setTitle(tr("Speed Limit Offset (35-54 kph)"));
    offset3Toggle->setTitle(tr("Speed Limit Offset (55-64 kph)"));
    offset4Toggle->setTitle(tr("Speed Limit Offset (65-99 kph)"));

    offset1Toggle->setDescription(tr("Set speed limit offset for limits between 0-34 kph."));
    offset2Toggle->setDescription(tr("Set speed limit offset for limits between 35-54 kph."));
    offset3Toggle->setDescription(tr("Set speed limit offset for limits between 55-64 kph."));
    offset4Toggle->setDescription(tr("Set speed limit offset for limits between 65-99 kph."));

    laneWidthToggle->updateControl(0, 30, tr(" meters"), 10);

    offset1Toggle->updateControl(-99, 99, tr(" kph"));
    offset2Toggle->updateControl(-99, 99, tr(" kph"));
    offset3Toggle->updateControl(-99, 99, tr(" kph"));
    offset4Toggle->updateControl(-99, 99, tr(" kph"));

    pauseLateralToggle->updateControl(0, 99, tr(" kph"));
    pauseLateralSpeedToggle->updateControl(0, 99, tr(" kph"));
    setSpeedOffsetToggle->updateControl(0, 150, tr(" kph"));

    stoppingDistanceToggle->updateControl(0, 5, tr(" meters"));
  } else {
    customCruiseToggle->updateControl(1, 99, tr(" mph"));

    offset1Toggle->setTitle(tr("Speed Limit Offset (0-34 mph)"));
    offset2Toggle->setTitle(tr("Speed Limit Offset (35-54 mph)"));
    offset3Toggle->setTitle(tr("Speed Limit Offset (55-64 mph)"));
    offset4Toggle->setTitle(tr("Speed Limit Offset (65-99 mph)"));

    offset1Toggle->setDescription(tr("Set speed limit offset for limits between 0-34 mph."));
    offset2Toggle->setDescription(tr("Set speed limit offset for limits between 35-54 mph."));
    offset3Toggle->setDescription(tr("Set speed limit offset for limits between 55-64 mph."));
    offset4Toggle->setDescription(tr("Set speed limit offset for limits between 65-99 mph."));

    laneWidthToggle->updateControl(0, 100, tr(" feet"), 10);

    offset1Toggle->updateControl(-99, 99, tr(" mph"));
    offset2Toggle->updateControl(-99, 99, tr(" mph"));
    offset3Toggle->updateControl(-99, 99, tr(" mph"));
    offset4Toggle->updateControl(-99, 99, tr(" mph"));

    pauseLateralToggle->updateControl(0, 99, tr(" mph"));
    pauseLateralSpeedToggle->updateControl(0, 99, tr(" mph"));
    setSpeedOffsetToggle->updateControl(0, 99, tr(" mph"));

    stoppingDistanceToggle->updateControl(0, 10, tr(" feet"));
  }

  customCruiseToggle->refresh();
  laneWidthToggle->refresh();
  offset1Toggle->refresh();
  offset2Toggle->refresh();
  offset3Toggle->refresh();
  offset4Toggle->refresh();
  pauseLateralToggle->refresh();
  pauseLateralSpeedToggle->refresh();
  setSpeedOffsetToggle->refresh();
  stoppingDistanceToggle->refresh();
}

void HpilotControlsPanel::hideToggles() {
  aggressiveProfile->setVisible(false);
  conditionalSpeedsImperial->setVisible(false);
  conditionalSpeedsMetric->setVisible(false);
  deleteModelBtn->setVisible(false);
  downloadModelBtn->setVisible(false);
  selectModelBtn->setVisible(false);
  slcPriorityButton->setVisible(false);
  standardProfile->setVisible(false);
  relaxedProfile->setVisible(false);
  trafficProfile->setVisible(false);

  std::set<QString> longitudinalKeys = {"ConditionalExperimental", "CustomPersonalities", "ExperimentalModeActivation",
                                        "LongitudinalTune", "MTSCEnabled", "SpeedLimitController", "VisionTurnControl"};

  for (auto &[key, toggle] : toggles) {
    toggle->setVisible(false);

    if (!hasOpenpilotLongitudinal && longitudinalKeys.find(key.c_str()) != longitudinalKeys.end()) {
      continue;
    }

    bool subToggles = aolKeys.find(key.c_str()) != aolKeys.end() ||
                      conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end() ||
                      experimentalModeActivationKeys.find(key.c_str()) != experimentalModeActivationKeys.end() ||
                      deviceManagementKeys.find(key.c_str()) != deviceManagementKeys.end() ||
                      laneChangeKeys.find(key.c_str()) != laneChangeKeys.end() ||
                      lateralTuneKeys.find(key.c_str()) != lateralTuneKeys.end() ||
                      longitudinalTuneKeys.find(key.c_str()) != longitudinalTuneKeys.end() ||
                      mtscKeys.find(key.c_str()) != mtscKeys.end() ||
                      qolKeys.find(key.c_str()) != qolKeys.end() ||
                      speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end() ||
                      speedLimitControllerControlsKeys.find(key.c_str()) != speedLimitControllerControlsKeys.end() ||
                      speedLimitControllerQOLKeys.find(key.c_str()) != speedLimitControllerQOLKeys.end() ||
                      speedLimitControllerVisualsKeys.find(key.c_str()) != speedLimitControllerVisualsKeys.end() ||
                      visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end();
    toggle->setVisible(!subToggles);
  }

  update();
}

void HpilotControlsPanel::hideSubToggles() {
  slcPriorityButton->setVisible(false);

  for (auto &[key, toggle] : toggles) {
    bool isVisible = speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end();
    toggle->setVisible(isVisible);
  }

  update();
}
