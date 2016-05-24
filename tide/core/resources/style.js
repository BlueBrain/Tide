.pragma library

// Color palette
var baseColor = "#F2F2F2"   // grey
var activeColor = "#FCEE21" // yellow

// General
var backgroundZOrder = -1;
var focusBackgroundZorder = 98;
var unfocusZorder = 99;
var focusZorder = 100;
var sideControlZorder = 200;
var overlayZorder = 1000;

var focusContextColor = "black"
var focusContextOpacity = 0.7;
var focusTransitionTime = 500;
var panelsAnimationTime = 300

var sideButtonColor = activeColor
var sideButtonRelHeight = 0.3

var fpsX = 10;
var fpsY = 10;
var fpsFontSize = 32;
var fpsFontColor = "blue";

var clockScale = 0.2  // times the height of the displaygroup

// Content windows
var buttonsSize = 130;
var buttonsImageSize = 100;
var buttonsEnabledOpacity = 100;
var buttonsDisabledOpacity = 20;

var controlsDefaultColor = baseColor;
var controlsFocusedColor = activeColor;
var controlsBorderWidth = 3;
var controlsRadius = 0;
var controlsLeftMargin = 28;

var resizeCircleRadius = 50
var resizeCircleActiveColor = activeColor;
var resizeCircleInactiveColor = baseColor;
var resizeCircleOpacity = 0.6;

var statisticsBorderMargin = 10;
var statisticsFontSize = 24;
var statisticsFontColor = "red";

var windowBorderWidth = 12;
var windowBorderDefaultColor = baseColor;
var windowBorderSelectedColor = activeColor;
var windowBorderMovingColor= baseColor;
var windowBorderResizingColor = baseColor;

var windowSideButtonWidth = 64;
var windowSideButtonHeight = 360;
var windowSideButtonNarrowHeight = 90;
var windowSideButtonArrowColor = "black";

var windowTitleFontSize = 45;
var windowTitleHeight = 100;

var zoomContextBorderColor = "white";
var zoomContextBackgroundColor = "black"
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
