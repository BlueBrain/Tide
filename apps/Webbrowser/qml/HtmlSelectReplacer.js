// Copyright (c) 2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

/**
 * Class to replace the system html select elements of a WebEngineView.
 */
function HtmlSelectReplacer(webengine) {
    this.webengine = webengine;
}

// Load the necessary scripts and replace the select elements.
// Call this when the page has finished loading.
HtmlSelectReplacer.prototype.process = function() {
    this.findSelectElements();
}

HtmlSelectReplacer.prototype.findSelectElements = function() {
    var hasSelectElements = "document.getElementsByTagName('select').length > 0";
    this.webengine.runJavaScript(hasSelectElements, (function(result) {
        if (result)
            this.checkJQuery();
    }).bind(this));
}

HtmlSelectReplacer.prototype.checkJQuery = function() {
    var hasJQuery = "window.jQuery ? true : false";
    this.webengine.runJavaScript(hasJQuery, (function(result) {
        if (result)
            this.checkJQueryUI();
        else
            this.loadJQuery();
    }).bind(this));
}

HtmlSelectReplacer.prototype.loadJQuery = function() {
    this.webengine.runJavaScript(htmlselect.jQuery,
                                 this.checkJQueryUI.bind(this));
}

HtmlSelectReplacer.prototype.checkJQueryUI = function() {
    var hasJQueryUI = "window.jQuery.ui ? true : false"
    this.webengine.runJavaScript(hasJQueryUI, (function(result) {
        if (result)
            this.loadSelectboxit()
        else
            this.loadJQueryUI()
    }).bind(this));
}

HtmlSelectReplacer.prototype.loadJQueryUI = function() {
    this.webengine.runJavaScript(htmlselect.jQueryUI,
                                 this.loadSelectboxit.bind(this));
}

HtmlSelectReplacer.prototype.loadSelectboxit = function() {
    this.webengine.runJavaScript(htmlselect.selectboxit,
                                 this.loadCssUsingJQuery.bind(this));
}

HtmlSelectReplacer.prototype.loadCssUsingJQuery = function() {
    this.webengine.runJavaScript(htmlselect.selectboxitCss,
                                 this.replaceSelectElements.bind(this));
}

HtmlSelectReplacer.prototype.replaceSelectElements = function() {
    this.webengine.runJavaScript(htmlselect.selectboxitReplace);
}
