.pragma library

// Color palette
var baseColor = "#F2F2F2";   // grey
var activeColor = "#FCEE21"; // yellow
var highlightColor = "cyan"; // cyan
var contrastColor = "black"; // black

// General
var backgroundZOrder = -1;
var panelsBackgroundZorder = 96;
var panelsZorder = 97;
var focusBackgroundZorder = 98;
var unfocusZorder = 99;
var focusZorder = 100;
var fullscreenBackgroundZorder = 198;
var fullscreenZorder = 199;
var streamNotificationZorder = 200;
var sideControlZorder = 201;
var countdownZorder = 202;

var overlayZorder = 1000;

var focusContextColor = "black"
var focusContextOpacity = 0.7;
var focusContextPanelsOpacity = 0.5;
var focusContextFullscreenOpacity = 0.99;
var focusTransitionTime = 500;
var panelsAnimationTime = 300
var countdownTransitionTime = 1500;

var sideButtonColor = activeColor
var sideButtonRelHeight = 0.3
var streamNotificationAreaBannerRelHeight = 0.04

var fpsX = 10;
var fpsY = 10;
var fpsFontSize = 32;
var fpsFontColor = "blue";

var clockScale = 0.2;  // times the height of the wall

var countdownTextScale = 0.045;

// Content windows
var buttonsSize = 130;
var buttonsImageRelSize = 0.75;
var buttonsEnabledOpacity = 100;
var buttonsDisabledOpacity = 20;

var controlsDefaultColor = baseColor;
var controlsFocusedColor = activeColor;
var controlsBorderWidth = 3;
var controlsRadius = 0;
var controlsLeftMargin = 28;

var keyboardRelSize = 0.8
var keyboardMaxSizePx = 1200;

var movieControlsLineThickness = 4;
var movieControlsHandleDiameter = 34;

var resizeCircleRadius = 50;
var resizeCircleActiveColor = activeColor;
var resizeCircleFreeResizeColor = highlightColor;
var resizeCircleInactiveColor = baseColor;
var resizeCircleOpacity = 0.6;

var statisticsBorderMargin = 10;
var statisticsFontSize = 24;
var statisticsFontColor = "red";

var transparentContentsBackgroundColor = "black";

var windowBorderWidth = 24;
var windowBorderDefaultColor = baseColor;
var windowBorderSelectedColor = activeColor;
var windowBorderFocusedColor = activeColor;
var windowBorderMovingColor = baseColor;
var windowBorderResizingColor = baseColor;

var windowFocusGlowColor = highlightColor;
var windowFocusGlowRadius = 15;
var windowFocusGlowSpread = 0.2;

var windowSideButtonWidth = 64;
var windowSideButtonHeight = 360;
var windowSideButtonNarrowHeight = 90;

var windowTitleControlsOverlap = 10;
var windowTitleFontSize = 45;
var windowTitleHeight = 100;

var zoomContextBorderColor = "white";
var zoomContextBackgroundColor = transparentContentsBackgroundColor;
var zoomContextBorderWidth = 5;
var zoomContextSelectionColor = "#6B9794";
var zoomContextSelectionWidth = 3;
var zoomContextSizeRatio = 0.25
var zoomContextMaxSizeRatio = 0.75
var zoomContextRelMargin = 0.25

// Touch point markers
var touchPointMarkerBorderColor = "red"
var touchPointMarkerCenterColor = "white"
var touchPointMarkerSize = 18
var touchPointMarkerBorderSize = 2

// Master window only
var masterWindowFirstCheckerColor = "#B2C7CF"
var masterWindowSecondCheckerColor = "#97BFCC"
var masterWindowBezelsColor = "black"
var masterWindowMarginFactor = 0.05
