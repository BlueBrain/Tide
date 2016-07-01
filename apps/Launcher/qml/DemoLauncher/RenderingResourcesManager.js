// Copyright (c) 2014-2016, EPFL/Blue Brain Project
//                          Pawel Podhajski <pawel.podhajski@epfl.ch>
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

// Constants
var MODULE_SESSION = 'session';

var SESSION_STATUS_STOPPED = 0;
var SESSION_STATUS_SCHEDULING = 1;
var SESSION_STATUS_SCHEDULED = 2;
var SESSION_STATUS_GETTING_HOSTNAME = 3;
var SESSION_STATUS_STARTING = 4;
var SESSION_STATUS_RUNNING = 5;
var SESSION_STATUS_STOPPING = 6;

var SESSION_COOKIE_NAME = 'HBP';

var SESSION_COMMAND_NONE = '';
var SESSION_COMMAND_LOG = 'log';
var SESSION_COMMAND_ERROR = 'err';
var SESSION_COMMAND_JOB = 'job';
var SESSION_COMMAND_SCHEDULE = 'schedule';
var SESSION_COMMAND_STATUS = 'status';

var NO_BODY;

/**
 * Class to manage communication with the BlueBrain RenderingResourceManager
 */
function Communicator(baseUrl, deflectStreamHost) {
    // Constants
    this.serviceUrl = baseUrl;
    this.deflectStreamHost = deflectStreamHost;
    this.commandLineArguments = '';
    this.environmentVariables = 'DEFLECT_HOST='+deflectStreamHost;

    // The id of the demo (or application name), set by the launch() function
    this.renderer_id = '';

    // Status of the session, returned by the RRM
    this.currentStatus = SESSION_STATUS_STOPPED;
    this.status = '';
}

/**
 * Return true if the current session is in running state.
 */
Communicator.prototype.isRunning = function() {
    return this.currentStatus === SESSION_STATUS_RUNNING;
}

Communicator.prototype.sendRequest = function(method, module, command, body, callback) {
    var fullUrl = this.serviceUrl + '/' + module + '/' + command;

    var request = new XMLHttpRequest();
    request.withCredentials = true;
    request.open(method, fullUrl, true);

    var bodyStr;
    if (body) {
        request.setRequestHeader('Content-Type', 'application/json');
        bodyStr = JSON.stringify(body);
    }

    request.onreadystatechange = function () {
        if (request.readyState === XMLHttpRequest.DONE)
            callback(request);
    }
    request.send(bodyStr);
};

/**
 * Query the status of the session as a string.
 */
Communicator.prototype.toStatusString = function(statusCode) {
    switch(statusCode) {
    case SESSION_STATUS_STOPPED:
        return "STOPPED";
    case SESSION_STATUS_SCHEDULING:
        return "SCHEDULING";
    case SESSION_STATUS_SCHEDULED:
        return "SCHEDULED";
    case SESSION_STATUS_GETTING_HOSTNAME:
        return "GETTING_HOSTNAME";
    case SESSION_STATUS_STARTING:
        return "STARTING";
    case SESSION_STATUS_RUNNING:
        return "RUNNING";
    case SESSION_STATUS_STOPPING:
        return "STOPPING";
    default:
        return "UNKNOWN_STATUS";
    }
}

/**
 * Query the status of the session and pass it back to the callback.
 */
Communicator.prototype.queryStatus = function(callback) {
    var self = this;
    this.sendRequest('GET', MODULE_SESSION, SESSION_COMMAND_STATUS, NO_BODY,
                     function(request) {
                         if (request.status === 200) {
                             var obj = JSON.parse(request.responseText);
                             self.currentStatus = obj.code;
                             self.status = obj.description;
                             callback(self.currentStatus);
                         }
                     });
};

/**
 * Create a session on the Rendering Resource Manager and initiates the process
 * of starting the remote rendering resource.
 */
Communicator.prototype.launch = function(demo_id) {
    this.renderer_id = demo_id;

    var openSessionParams = {
        owner: demo_id+'@'+this.deflectStreamHost,
        renderer_id: demo_id
    };

    var self = this;
    this.sendRequest('POST', MODULE_SESSION, SESSION_COMMAND_NONE, openSessionParams,
                     function(request) {
                         if (request.status === 201)
                             self.startRenderer();
                         else
                             console.warn("Session could not be opened",
                                          request.status, request.responseText);
                     });
};

Communicator.prototype.closeCurrentSession = function() {
    var self = this;
    this.sendRequest('DELETE', MODULE_SESSION, SESSION_COMMAND_NONE, NO_BODY,
                     function(request) {
                         console.log("DELETE SESSION:", request.status, request.responseText);
                     });
};

/**
 * Starts the remote rendering resource according to parameters
 */
Communicator.prototype.startRenderer = function() {
    var params = {
        params: this.commandLineArguments,
        environment: this.environmentVariables,
    };
    var self = this;
    this.sendRequest('PUT', MODULE_SESSION, SESSION_COMMAND_SCHEDULE, params,
                     function (request) {
                         if(request.status === 200)
                             self.querySessionInfo(SESSION_COMMAND_JOB);
                         else {
                             self.querySessionInfo(SESSION_COMMAND_ERROR);
                             self.querySessionInfo(SESSION_COMMAND_LOG);
                         }
                     });
}

/**
 * Query the log of the session and pass it back to the callback.
 */
Communicator.prototype.querySessionInfo = function(infoType) {
    this.sendRequest('GET', MODULE_SESSION, infoType, NO_BODY,
                     function(request) {
                         if (request.status === 200) {
                             var obj = JSON.parse(request.responseText);
                             if (infoType === SESSION_COMMAND_JOB)
                                 console.log("job info:", request.status,
                                             obj.contents);
                             else
                                 console.warn("job "+infoType+":",
                                              request.status, obj.contents);
                         }
                         else
                             console.warn("get "+infoType+":", request.status);
                     });
};

/**
 * Query all configurations, filter them and pass them to the callback.
 */
Communicator.prototype.queryConfigurations = function(callback, filter) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState === XMLHttpRequest.DONE && request.status == 200) {
            var configurations = JSON.parse(request.responseText);
            callback(filter(configurations));
        }
    }
    request.open("GET", this.serviceUrl + '/config/', true);
    request.send();
};

/**
 * Query all the demos and pass them to the callback.
 */
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
