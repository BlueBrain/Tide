// Copyright (c) 2014-2016, EPFL/Blue Brain Project
//                          Pawel Podhajski <pawel.podhajski@epfl.ch>
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

/**
 * Class to manage communication with the BlueBrain RenderingResourceManager
 */
function Communicator(baseUrl) {
    this.baseUrl = baseUrl;
    this.connected = false;
    this.renderer_id = ''

    var STATUS_NONE = 0
    var STATUS_SESSION_CREATED = 1
    var STATUS_SESSION_RUNNING = 2
    this.currentStatus = STATUS_NONE
}

Communicator.prototype.sendRequest = function(method, url, body) {
    var request = new XMLHttpRequest()
    request.withCredentials = true
    request.onreadystatechange = function () {
        if (request.readyState == XMLHttpRequest.DONE) {
            switch (currentStatus) {
            case STATUS_NONE:
                currentStatus = STATUS_SESSION_CREATED
                startRenderer()
                break
            case STATUS_SESSION_CREATED:
                currentStatus = STATUS_NONE
            }
        }
    }
    request.open(method, url, true)
    request.setRequestHeader('HBP', openSessionParams.renderer_id)
    var bodyStr
    if (body) {
        bodyStr = JSON.stringify(body)
        request.setRequestHeader('Content-Type', 'application/json')
    }
    request.send(bodyStr)
};

Communicator.prototype.launch = function(demo_id) {
    var openSessionParams = { owner: 'bbpdemolauncher', renderer_id: demo_id }
    this.sendRequest('POST', this.baseUrl + '/session/', openSessionParams)
    this.renderer_id = demo_id
};

Communicator.prototype.startRenderer = function() {
    var rendererParams = {}
    this.sendRequest('PUT', this.baseUrl + '/session/schedule', rendererParams)
};

Communicator.prototype.querySessionStatus = function(callback) {
    var request = new XMLHttpRequest()
    request.withCredentials = true
    request.onreadystatechange = function() {
        if (request.readyState === XMLHttpRequest.DONE && request.status == 200) {
            try {
                var status = JSON.parse(request.responseText).description
                callback(status)
            }
            catch (err) {
                console.log("JSON parsing error:", request.responseText)
            }
        }
    }
    request.open('GET', this.baseUrl + '/session/status', true)
    request.setRequestHeader('HBP', openSessionParams.renderer_id)
    request.send()
};

Communicator.prototype.queryConfigurations = function(callback, filter) {
    var request = new XMLHttpRequest()
    request.onreadystatechange = function() {
        if (request.readyState == XMLHttpRequest.DONE && request.status == 200) {
            var configurations = JSON.parse(request.responseText)
            callback(filter(configurations))
        }
    }
    request.open("GET", this.baseUrl + '/config/', true)
    request.send()
};

Communicator.prototype.queryDemos = function(callback) {
    function filterDemos(configurations) {
        var demos = [];
        var regex = new RegExp("demo")
        for(var i = 0; i < configurations.length; ++i) {
            if(regex.test(configurations[i].id))
                demos.push(configurations[i])
        }
        return demos;
    }

    this.queryConfigurations(callback, filterDemos);
};
