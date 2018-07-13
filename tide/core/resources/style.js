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
var contextMenuZorder = 202;
var overlayZorder = 1000;

var focusContextColor = "black"
var focusContextOpacity = 0.7;
var focusContextPanelsOpacity = 0.5;
var focusContextFullscreenOpacity = 0.99;
var focusTransitionTime = 500;
var panelsAnimationTime = 300
var countdownTransitionTime = 1500;

var fpsX = 10;
var fpsY = 10;
var fpsFontSize = 32;
var fpsFontColor = "blue";

var clockScale = 0.2;  // times the height of the wall

var countdownTextScale = 0.045;

// Regular Buttons
var buttonsSize = 106;
var buttonsPadding = buttonsSize / 4;
var buttonsImageRelSize = 0.85;
var buttonsEnabledOpacity = 1.0;
var buttonsDisabledOpacity = 0.5;

// Large Buttons (surface controls, context menu)
var buttonsSizeLarge = 1.15 * buttonsSize
var buttonsPaddingLarge = 1.15 * buttonsPadding;

// Side buttons (triangular) for surface and windows
var sideButtonColor = activeColor
var sideButtonAspectRatio = 0.18
var sideButtonRelNarrowHeight = 0.22

// Surface controls (left-side triangle)
var surfaceControlsColor = activeColor

// Content windows
var windowBorderWidth = 20;
var windowBorderDefaultColor = baseColor;
var windowBorderSelectedColor = activeColor;
var windowBorderFocusedColor = activeColor;
var windowBorderMovingColor = baseColor;
var windowBorderResizingColor = baseColor;

var windowTitleHeight = 100;
var windowTitleFontSize = windowTitleHeight / 2;
var windowTitleControlsOverlap = 10;

var resizeCircleRadius = 50;
var resizeCircleActiveColor = activeColor;
var resizeCircleFreeResizeColor = highlightColor;
var resizeCircleInactiveColor = baseColor;
var resizeCircleOpacity = 0.6;

var controlsDefaultColor = baseColor;
var controlsFocusedColor = activeColor;
var controlsLeftMargin = buttonsPadding;

var keyboardRelSize = 0.8
var keyboardMaxSizePx = 1200;

var windowSideButtonWidth = 64;

var movieControlsHandleDiameter = buttonsSize / 3;
var movieControlsLineThickness = movieControlsHandleDiameter / 9;

var statisticsBorderMargin = 10;
var statisticsFontSize = 24;
var statisticsFontColor = "red";

var transparentContentsBackgroundColor = "black";

var windowFocusGlowColor = highlightColor;
var windowFocusGlowRadius = 15;
var windowFocusGlowSpread = 0.2;

var zoomContextBorderColor = "white";
var zoomContextBackgroundColor = transparentContentsBackgroundColor;
var zoomContextBorderWidth = 5;
var zoomContextSelectionColor = "#6B9794";
var zoomContextSelectionWidth = 3;
var zoomContextSizeRatio = 0.25
var zoomContextMaxSizeRatio = 0.75
var zoomContextRelMargin = 0.25

// Touch point markers
var touchPointMarkerSize = 18
var touchPointMarkerBorderSize = 2
var touchPointMarkerBorderColor = "red"
var touchPointMarkerCenterColor = "white"

// Master window only
var masterWindowFirstCheckerColor = "#B2C7CF"
var masterWindowSecondCheckerColor = "#97BFCC"
var masterWindowBezelsColor = "black"
var masterWindowMarginFactor = 0.05
