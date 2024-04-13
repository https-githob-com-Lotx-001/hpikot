#pragma once

#include <set>

#include "selfdrive/hpilot/ui/qt/widgets/hpilot_controls.h"
#include "selfdrive/ui/qt/offroad/settings.h"
#include "selfdrive/ui/ui.h"

class HpilotControlsPanel : public HpilotListWidget {
  Q_OBJECT

public:
  explicit HpilotControlsPanel(SettingsWindow *parent);

signals:
  void openParentToggle();
  void openSubParentToggle();

private:
  void hideSubToggles();
  void hideToggles();
  void showEvent(QShowEvent *event, const UIState &s);
  void updateCarToggles();
  void updateMetric();
  void updateState(const UIState &s);
  void updateToggles();

  ButtonControl *deleteModelBtn;
  ButtonControl *downloadModelBtn;
  ButtonControl *selectModelBtn;
  ButtonControl *slcPriorityButton;

  HpilotDualParamControl *aggressiveProfile;
  HpilotDualParamControl *conditionalSpeedsImperial;
  HpilotDualParamControl *conditionalSpeedsMetric;
  HpilotDualParamControl *relaxedProfile;
  HpilotDualParamControl *standardProfile;
  HpilotDualParamControl *trafficProfile;

  std::set<QString> aolKeys = {"AlwaysOnLateralMain", "HideAOLStatusBar", "PauseAOLOnBrake"};
  std::set<QString> conditionalExperimentalKeys = {"CECurves", "CECurvesLead", "CENavigation", "CESignal", "CESlowerLead", "CEStopLights", "HideCEMStatusBar"};
  std::set<QString> deviceManagementKeys = {"DeviceShutdown", "HigherBitrate", "IncreaseThermalLimits", "LowVoltageShutdown", "NoLogging", "NoUploads", "OfflineMode"};
  std::set<QString> experimentalModeActivationKeys = {"ExperimentalModeViaDistance", "ExperimentalModeViaLKAS", "ExperimentalModeViaTap"};
  std::set<QString> laneChangeKeys = {"LaneChangeTime", "LaneDetectionWidth", "OneLaneChange"};
  std::set<QString> lateralTuneKeys = {"ForceAutoTune", "NNFF", "NNFFLite", "SteerRatio", "TurnDesires"};
  std::set<QString> longitudinalTuneKeys = {"AccelerationProfile", "AggressiveAcceleration", "DecelerationProfile", "LeadDetectionThreshold", "SmoothBraking", "StoppingDistance", "TrafficMode"};
  std::set<QString> mtscKeys = {"DisableMTSCSmoothing", "MTSCAggressiveness", "MTSCCurvatureCheck"};
  std::set<QString> qolKeys = {"CustomCruise", "CustomCruiseLong", "DisableOnroadUploads", "OnroadDistanceButton", "PauseLateralSpeed", "ReverseCruise", "SetSpeedOffset"};
  std::set<QString> speedLimitControllerKeys = {"SLCControls", "SLCQOL", "SLCVisuals"};
  std::set<QString> speedLimitControllerControlsKeys = {"Offset1", "Offset2", "Offset3", "Offset4", "SLCFallback", "SLCOverride", "SLCPriority"};
  std::set<QString> speedLimitControllerQOLKeys = {"ForceMPHDashboard", "SetSpeedLimit", "SLCConfirmation", "SLCLookahead"};
  std::set<QString> speedLimitControllerVisualsKeys = {"ShowSLCOffset", "UseVienna"};
  std::set<QString> visionTurnControlKeys = {"CurveSensitivity", "DisableVTSCSmoothing", "TurnAggressiveness"};

  std::map<std::string, ParamControl*> toggles;

  Params params;
  Params paramsMemory{"/dev/shm/params"};

  bool hasAutoTune;
  bool hasCommaNNFFSupport;
  bool hasNNFFLog;
  bool hasOpenpilotLongitudinal;
  bool hasPCMCruise;
  bool hasDashSpeedLimits;
  bool isMetric = params.getBool("IsMetric");
  bool isStaging;
  bool isToyota;
  bool started;

  float steerRatioStock;
};