"use strict";
var activeSurfaceIndex = 0;
var bezelWidth;
var bezelHeight;
var displaysPerScreenX;
var displaysPerScreenY;
var focus;
var fullscreen;
var locked;
var screenCountX;
var screenCountY;
var displayHeight;
var displayWidth;
var sessionFiles = [];
var timer;
var wallWidth;
var wallHeight;
var windowList = [];
var zoomScale;
var output = [];
var filters = [];
var timer;
window.onresize = setScale;

$(init);

function alertPopup(title, text) {
  swal({
      title: title,
      text: text,
      confirmButtonColor: "#014f86",
      confirmButtonText: "Refresh page!",
      closeOnConfirm: false
    },
    function() {
      location.reload();
    });
}

function autoRefresh() {
  if (timer > 0) {
    clearInterval(timer);
    timer = 0;
    $("#autoRefreshButton").removeClass("buttonPressed");
  }
  else {
    timer = setInterval(updateWall, refreshInterval);
    $("#autoRefreshButton").addClass("buttonPressed");
  }
}

function bootstrapContentsDialog() {
  $("#contentsDialog").dialog({
    modal: true,
    autoOpen: false,
    width: 'auto',
    open: function() {
      $('.ui-widget-overlay').bind('click', function() {
        $('#contentsDialog').dialog('close');
      });
    }
  });
  $("#contentsButton").on("click", function() {
    $("#contentsDialog").dialog("open");
  });
}

function bootstrapMenus() {
  $("#addButton").click(function (e) {
    $("#uploadMenu,#sessionMenu,#optionsMenu,#appsMenu,#infoMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
      e.stopPropagation()
    });

    $("#fsMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#addButton)").removeClass("buttonPressed");
    $("#addButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  $("#sessionButton").click(function (e) {
    $("#uploadMenu,#fsMenu,#optionsMenu,#appsMenu,#infoMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
      e.stopPropagation()
    });

    $("#sessionMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#sessionButton)").removeClass("buttonPressed");
    $("#sessionButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  $("#uploadButton").click(function (e) {
    $("#sessionMenu,#fsMenu,#optionsMenu,#appsMenu,#infoMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
      e.stopPropagation()
    });

    $("#uploadMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#uploadButton)").removeClass("buttonPressed");
    $("#uploadButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  $("#optionsButton").click(function (e) {
    $("#sessionMenu,#fsMenu,#uploadMenu,#appsMenu,#infoMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
    });

    $("#optionsMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#optionsButton)").removeClass("buttonPressed");
    $("#optionsButton").toggleClass("buttonPressed");
    updateOptions();
    e.stopPropagation()
  });

  $("#appsButton").click(function (e) {
    $("#sessionMenu,#fsMenu,#uploadMenu,#optionsMenu,#infoMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
    });

    $("#appsMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#appsButton)").removeClass("buttonPressed");
    $("#appsButton").toggleClass("buttonPressed");
    e.stopPropagation()
  });

  $("#browseUrlInput").on("click", function (e) {
    e.stopPropagation()
  });

  $("#infoButton").click(function (e) {
    $("#uploadMenu,#fsMenu,#optionsMenu,#appsMenu,#sessionMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
      e.stopPropagation()
    });

    $("#infoMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#infoButton)").removeClass("buttonPressed");
    $("#infoButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  getOptions();
}

function bootstrapUpload() {
  var wall = document.getElementById('wall');
  wall.addEventListener('dragover', handleDragOver, false);
  wall.addEventListener('dragleave', handleDragLeave, false);
  wall.addEventListener('drop', handleUpload, false);
  document.getElementById('file-select').addEventListener('change', handleUpload, false);
  $(".file-input").mousedown(function () {
    $(this).addClass("buttonPressed");
  }).mouseup(function () {
    $(this).removeClass("buttonPressed")});
}

function browse() {
  var url = $('#browseUrlInput').val();
  var params = {"uri": (url), "surfaceIndex": activeSurfaceIndex};
  sendAppJsonRpc("browse", params, updateWall);
  $('#browseUrlInput').val("");
  $("#appsButton").click();
}

function checkIfEqual(tile1, tile2) {
  return (
    tile1.x === tile2.x &&
    tile1.y === tile2.y &&
    tile1.width === tile2.width &&
    tile1.height === tile2.height &&
    tile1.mode === tile2.mode &&
    tile1.selected === tile2.selected &&
    tile1.z === tile2.z &&
    tile1.visible === tile2.visible
  );
}

function clearUploadList() {
  for (var i = output.length - 1; i >= 0; i--) {
    // if (output[i].finished === true || output[i].started !== true) {
    if (output[i].started === false || output[i].finished === true) {
      $('#' + output[i].id).remove();
      output.splice(i, 1);
    }
  }
  $('#file-form').find("input[type=file]").val("");
  $('#upload-button').hide();
}

function copy(jsonWindow, tile) {
  tile.z = jsonWindow.z;
  tile.width = jsonWindow.width;
  tile.height = jsonWindow.height;
  tile.y = jsonWindow.y;
  tile.x = jsonWindow.x;
  tile.selected = jsonWindow.selected;
  tile.fullscreen = jsonWindow.fullscreen;
  tile.mode = jsonWindow.mode;
  tile.visible = jsonWindow.visible;
  tile.focus = jsonWindow.focus;
  tile.minHeight = jsonWindow.minHeight;
  tile.minWidth = jsonWindow.minWidth;
}

function createButton(type, tile) {
  var button = document.createElement("div");
  button.id = type + tile["uuid"];
  button.className = "windowControl";
  return button;
}

function createCloseButton(tile) {
  var closeButton = createButton("closeButton", tile);
  closeButton.onclick = function (event) {
    event.stopImmediatePropagation();
    sendSceneJsonRpc("close-window", getIdAsObject(tile), updateWall);
  };
  var icon = document.createElement("img");
  icon.src = closeImageUrl;
  closeButton.appendChild(icon);
  return closeButton;
}

function createFocusButton(tile) {
  var focusButton = createButton("focusButton", tile);
  focusButton.onclick = function (event) {
    event.stopImmediatePropagation();
    if (focus)
      sendSceneJsonRpc("unfocus-window", getIdAsObject(tile), updateWall);
    else
      sendSceneJsonRpc("focus-windows", getSurfaceParams(), updateWall);
  };
  var icon = document.createElement("img");
  icon.src = focusImageUrl;
  focusButton.appendChild(icon);
  return focusButton;
}

function createFullscreenButton(tile) {
  var fullscreenButton = createButton("fullscreenButton", tile);
  fullscreenButton.style.visibility = tile.fullscreen ? "hidden" : "";
  fullscreenButton.onclick = function (event) {
    event.stopImmediatePropagation();
    sendSceneJsonRpc("move-window-to-fullscreen", getIdAsObject(tile), updateWall);
  };
  var icon = document.createElement("img");
  icon.src = fullscreenImageUrl;
  fullscreenButton.appendChild(icon);
  return fullscreenButton;
}

function createWindow(tile) {
  windowList.push(tile);

  var windowDiv = document.createElement("div");
  $("#wall").append(windowDiv);
  windowDiv.id = tile.uuid;
  windowDiv.className = "windowDiv";
  windowDiv.title = tile.title;
  $(this).animate({
    transform: 'scale(' + zoomScale + ')'
  });

  var controlDiv = document.createElement("div");
  controlDiv.className = "windowControls";
  controlDiv.ondragstart = function() { return false; };
  controlDiv.appendChild(createCloseButton(tile));
  controlDiv.appendChild(createFullscreenButton(tile));
  controlDiv.appendChild(createFocusButton(tile));
  windowDiv.appendChild(controlDiv);

  var thumbnail = new Image();
  thumbnail.id = "img" + tile["uuid"];
  thumbnail.className = "thumbnail";
  queryThumbnail(tile);
  windowDiv.appendChild(thumbnail);

  setHandles(tile);

  $('#' + tile.uuid).bind('mousewheel DOMMouseScroll', _.throttle(function (event) {
      return resizeOnWheelEvent(event, tile)
    }, zoomInterval)
  );
  windowDiv.onclick = function (event) {
    if (!fullscreen && !focus) {
      sendSceneJsonRpc("toggle-select-window", getIdAsObject(tile),
        function () {
          return sendSceneJsonRpc("move-window-to-front", getIdAsObject(tile), updateWall)
        }
      );
    }
    event.stopImmediatePropagation();
  };

  updateWindow(tile);
}

function displayWallLock() {
  var wallLock = $("#wallLock");
  wallLock.clearQueue();
  wallLock.fadeTo(100, 1).css("background-color", "grey").fadeTo(1000, 0.0);
}

function showControls(tile) {
  $('#closeButton' + tile.uuid).css("visibility", "visible");
  $('#fullscreenButton' + tile.uuid).css("visibility", "visible");
  var focusButtonVisible = tile.selected ? "visible" : "hidden";
  $('#focusButton' + tile.uuid).css("visibility", focusButtonVisible);
}

function hideControls(tile) {
  $('#closeButton' + tile.uuid).css("visibility", "hidden");
  $('#fullscreenButton' + tile.uuid).css("visibility", "hidden");
  $('#focusButton' + tile.uuid).css("visibility", "hidden");
}

function getFileSystemContent(path) {
  var xhr = new XMLHttpRequest();
  var url = restUrl + "files/" + encodeURI(path);
  var files = [];
  xhr.open("GET", url, true);
  xhr.onload = function () {
    var data = JSON.parse(xhr.responseText);
    var filesCount = 0;
    for (var i = 0; i < data.length; i++)
      if (!data[i].dir)
        ++filesCount;

    // No "Move up" button at the top-level
    if (path !== "") {
      var upFile = {
        text: "Move up",
        path: path.split('/').slice(0, -1).join('/'),
        dir: true,
        icon: "glyphicon glyphicon-level-up",
        backColor: "#d1e2ee"
      };
      files.push(upFile);
    }
    var openAllFile = {
      text: filesCount > 0 ? " Open all regular files: " + filesCount : " No regular files to open here",
      path: path,
      dir: true,
      icon: "glyphicon glyphicon-folder-open",
      backColor: filesCount > 0 ? "#1a6092" : "ligthgrey",
      currentDir: true
    };
    files.push(openAllFile);

    for (var i = 0; i < data.length; i++) {
      var file = {
        text: data[i].name,
        path: (path === "") ? data[i].name : (path + "/" + data[i].name),
        dir: data[i].dir,
        icon: data[i].dir ? "glyphicon glyphicon-folder-close" : "glyphicon glyphicon-file"
      };
      files.push(file);
    }

    $('#fsMenu').treeview({
      data: files,
      searchResultBackColor: "#014f86",
      highlightSelected: true
    });
    $('#fsMenu').on('nodeSelected', function (event, data) {
      // CURRENT FOLDER - enable option to open all content only if it contains at least a file
      if (data.currentDir && filesCount > 0) {
        if (filesCount > 10) {
          swal({
              type: "warning",
              title: "Are you sure?",
              text: "You intend to open a folder with " + filesCount + " files.",
              confirmButtonColor: "#DD6B55",
              confirmButtonText: "Yes",
              cancelButtonText: "No",
              closeOnConfirm: true,
              closeOnCancel: true,
              showCancelButton: true
            },
            function (isConfirm) {
              if (isConfirm) {
                var params = {"uri": data.path, "surfaceIndex": activeSurfaceIndex};
                sendAppJsonRpc("open", params, updateWall);
                $('#fsMenu').hide()
              }
            });
        }
        else {
          var params = {"uri": data.path, "surfaceIndex": activeSurfaceIndex};
          sendAppJsonRpc("open", params, updateWall);
          $('#fsMenu').treeview('toggleNodeSelected', [data.nodeId, {silent: true}]);
        }
      }
      // FOLDER - query its content and update the view
      else if (data.dir) {
        getFileSystemContent(data.path);
      }
      // REGULAR FILE
      else {
        var params = {"uri": data.path, "surfaceIndex": activeSurfaceIndex};
        sendAppJsonRpc("open", params, updateWall);
        $('#fsMenu').treeview('toggleNodeSelected', [data.nodeId, {silent: true}]);
      }
    })
  };
  xhr.send(null);
}

function getOptions() {
  var options = [];
  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "options", true);
  xhr.onload = function () {
    options = JSON.parse(xhr.responseText);

    for (var property in options) {
      if (options.hasOwnProperty(property)) {
        if (typeof(options[property]) !== "boolean")
          continue;
        var label = document.createElement("label");
        label.className = "optionLabel";
        var checkbox = document.createElement("input");
        checkbox.type = "CHECKBOX";
        checkbox.class = "optionInput";
        checkbox.id = "checkbox_" + property;
        var labelText = document.createTextNode(property);
        label.appendChild(checkbox);
        label.appendChild(labelText);
        checkbox.checked = options[property];
        checkbox.addEventListener('change', setOption.bind(null, property));
        document.getElementById('optionsMenu').appendChild(label);
      }
    }
  };
  xhr.send(null)
}

function getSessionFolderContent() {
  var url = restUrl + "sessions/";
  var xhr = new XMLHttpRequest();
  var data;
  sessionFiles = [];
  xhr.open("GET", encodeURI(url), true);
  xhr.onload = function () {
    data = JSON.parse(xhr.responseText);
    for (var i = 0; i < data.length; i++) {
      var file = {
        text: data[i].name,
        path: data[i].name,
        dir: data[i].dir,
      };
      file.icon = data[i].dir ? "glyphicon glyphicon-chevron-right" : "glyphicon glyphicon-file";
      file.color = data[i].dir ? "grey" : "black";
      if (!data[i].dir)
        sessionFiles.push(file);
    }

    $('#sessionTree').treeview({
      data: sessionFiles,
      searchResultBackColor: "#014f86",
      highlightSelected: true
    });
    $('#sessionTree').on('nodeSelected', function (event, data) {
      if (!data.dir) {
        $("#wall").css("opacity", 0.2);
        sendAppJsonRpc("load", {"uri": data.text}, function () {
          $("#sessionNameInput").val(data.text);
          $("#sessionMenu").toggle("puff", showEffectSpeed);
          $("#sessionButton").toggleClass("buttonPressed");
          $('#sessionTree').treeview('toggleNodeSelected', [data.nodeId, {silent: true}]);
          $("#wall").css("opacity", 1);
          updateWall();
        });
      }
    })
  };
  xhr.send(null);
}

function getSurfaceParams()
{
    return {"surfaceIndex": activeSurfaceIndex};
}

function getUrlParam(name) {
    var results = new RegExp('[\?&]' + name + '=([^&#]*)')
                      .exec(window.location.href);
    if (results) {
        return results[1];
    }
    return 0;
}

function handleDragLeave(evt) {
  evt.stopPropagation();
  evt.preventDefault();
  $("#wall").css("opacity", 1);
}

function handleDragOver(evt) {
  evt.stopPropagation();
  evt.preventDefault();
  $("#wall").css("opacity", 0.2);
}

function handleUpload(evt) {
  evt.stopPropagation();
  evt.preventDefault();
  var coords = {};
  $("#wall").css("opacity", 1);
  var data;
  if (evt.type == "drop") {
    data = evt.dataTransfer.files;
    var offset = $("#wall").offset();
    coords["x"] = (evt.clientX-offset["left"]) / zoomScale;
    coords["y"] = (evt.clientY-offset["top"]) / zoomScale;
  }
  else if (evt.type === "change") {
    data = evt.target.files;
    $('#submitButton').show();
    //remove finished uploads and added but not transfered ones
    for (var i = output.length - 1; i >= 0; i--) {
      if (output[i].started === false || output[i].finished === true) {
        $('#' + output[i].id).remove();
        output.splice(i, 1);
      }
    }
  }

  data = Array.from(data);
  var files = data.filter(function (file) {
    return filters.some(function (element) {
      return file.name.toLowerCase().endsWith(element)
    });
  });

  var notUploaded = data.length - files.length;
  if (notUploaded > 0)
    swal({
      type: "warning",
      title: (notUploaded > 1) ? notUploaded + " unsupported files" : notUploaded + " unsupported file",
      text: "Supported files are: " + filters.join(" "),
      confirmButtonColor: "#014f86",
      confirmButtonText: "OK",
      closeOnConfirm: true
    });
  if (files.length > 0)
    $('.formButton').removeAttr('disabled');
  for (var i = 0; i < files.length; i++) {
    var f = files[i];
    f.id = "file_" + parseInt(Math.random() * 100000);
    var transfer = {};
    transfer.finished = false;
    transfer.text = '<strong>' + f.name + '</strong>: ' + (f.size / MBtoB).toFixed(2) + ' MB';
    transfer.id = f.id;
    transfer.started = false;
    output.push(transfer);
    var li = document.createElement("li");
    li.id = f.id;
    var span = document.createElement("span");
    span.innerHTML = transfer.text;
    li.appendChild(span);
    document.getElementById('outputList').appendChild(li);
  }

  if (evt.type == "change") {
    var form = document.getElementById('file-form');
    form.onsubmit = (function (files) {
      return function () {
        $('#submitButton').hide();
        preuploadCheck(files, coords);
      }
    })(files);
  }
  else if (evt.type == "drop") {
    preuploadCheck(files, coords);
    if (!$('#uploadMenu').is(':visible') && files.length > 0)
    {
      $('#uploadButton').notify("Upload started", {
        className: "info",
        autoHideDelay: 3000
      }).effect("pulsate", {times: 2}, 3000);
    }
  }
}

function init() {
  bootstrapUpload();
  bootstrapMenus();

  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "config", true);
  xhr.onload = function () {
    var config = JSON.parse(xhr.responseText)["config"];

    activeSurfaceIndex = parseInt(getUrlParam('surface')) || 0;
    if (activeSurfaceIndex >= config["surfaceSizes"].length) {
        swal("No such surface: " + activeSurfaceIndex,
             "Surface 0 will be shown instead");
        activeSurfaceIndex = 0;
    }

    var surfaceSize = config["surfaceSizes"][activeSurfaceIndex];
    wallWidth = surfaceSize[0];
    wallHeight = surfaceSize[1];
    var surface = config["surfaces"][activeSurfaceIndex];
    screenCountX = surface["screenCountX"];
    screenCountY = surface["screenCountY"];
    bezelWidth = surface["bezelWidth"];
    bezelHeight = surface["bezelHeight"];
    displaysPerScreenX = surface["displaysPerScreenX"];
    displaysPerScreenY = surface["displaysPerScreenY"];
    displayWidth = surface["displayWidth"];
    displayHeight = surface["displayHeight"];
    var backgroundColor = surface["background"]["color"];

    if (config["name"] !== "")
      document.title = config["name"];

    setScale();
    setBezels();

    filters = config["filters"];
    for (var i = 0; i < filters.length; i++)
      filters[i] = filters[i].replace(/\*/g, "");
    $("#file-select").attr("accept", filters);
    var wall = $("#wall");
    wall.css("background-color", backgroundColor);
    wall.css("width", wallWidth).css("height", wallHeight);

    $('#wallWrapper').click(function(){
      $(".topMenu").hide();
      $(".menuButton").removeClass("buttonPressed");
    });

    wall.click(function () {
      if (windowList.length > 0) {
        sendSceneJsonRpc("deselect-windows", getSurfaceParams(), updateWall);
      }
    })

    $('.screenbezel').hover(
      function(event){
        if (!isAnyWindowDragged())
          return;
        let bezel = this;
        timer = setTimeout(function(){
         return (stickToBezel)(event, bezel);
         }, 250)
      },function (event) {
        if (!isAnyWindowDragged())
          return;
        clearTimeout(timer);
        $("#stickToOverlay").remove();
      }
    );

    $("#infoMenu").append("<b>Tide " + config["version"] + "</b> rev ",
      "<a style='text-decoration: underline' href=\"https://github.com/BlueBrain/Tide/commit/" + config["revision"] + "\">" + config["revision"],
      " </a><br>", "running on <b>" + config["hostname"], "</b><br>since <b>" + config["startTime"]+"</b>");
    getFileSystemContent("");
    getSessionFolderContent();
    updateWall();
  };

  xhr.onerror = function () {
    alertPopup("Something went wrong.", "Tide REST interface not accessible at: " + restUrl);
  };
  xhr.send(null);
  $("#exitFullscreenButton").on("click", function () {
    sendSceneJsonRpc("exit-fullscreen", getSurfaceParams(), updateWall);
    removeCurtain(fullscreenCurtain);
  });
  $("#closeAllButton").on("click", function () {
    sendSceneJsonRpc("clear", getSurfaceParams(), updateWall);
  });

  bootstrapContentsDialog();
}

function isBezelVisible(){
  return ($('.screen').is(':visible'));
}

function isAnyWindowDragged()
{
  return $(".ui-draggable-dragging").length === 1;
}

function getIdAsObject(tile) {
  return {"id": tile.uuid}
}

function markAsStandard(tile) {
  var tileDiv = $('#' + tile.uuid);
  tileDiv.draggable('enable');
  tileDiv.draggable({axis: false});
  tileDiv.resizable('enable');
  tileDiv.removeClass("windowFullscreen");

  showControls(tile);
}

function markAsFocused(tile) {
  var tileDiv = $('#' + tile.uuid);
  tileDiv.draggable('disable');
  tileDiv.resizable('disable');
  tileDiv.removeClass('active').addClass('inactive');
  tileDiv.removeClass("windowFullscreen");
  tileDiv.css("z-index", zIndexFocus).css("border", "0px");
  $('#focusButton' + tile.uuid).css("visibility", "visible");

  showControls(tile);
}

function markAsFullscreen(tile) {
  var tileDiv = $('#' + tile.uuid);

  tileDiv.resizable('disable');

  if (tile.height > wallHeight && tile.width > wallWidth)
    tileDiv.draggable('enable');
  else if (tile.height > wallHeight)
    tileDiv.draggable({axis: "y"}).draggable('enable');
  else if (tile.width > wallWidth)
    tileDiv.draggable({axis: "x"}).draggable('enable');
  else
    tileDiv.draggable('disable');

  tileDiv.removeClass('active').addClass('inactive');
  tileDiv.css("z-index", zIndexFullscreen);
  tileDiv.addClass("windowFullscreen");

  hideControls(tile);
}

function markAsSelected(tile) {
  $('#' + tile.uuid).addClass("windowSelected");
  $('#focusButton' + tile.uuid).css("visibility", "visible")
}

function markAsUnselected(tile) {
  $('#' + tile.uuid).removeClass('windowSelected');
  $('#focusButton' + tile.uuid).css("visibility", "hidden")
}

function markAsLocked(tile) {
  var tileDiv = $('#' + tile.uuid);
  tileDiv.draggable('disable');
  tileDiv.resizable('disable');
}

function openWhiteboard() {
  sendAppJsonRpc("whiteboard", getSurfaceParams(), updateWall);
  $("#appsButton").click();
}

function preuploadCheck(files, coords) {
  var totalSize = 0;
  for (var i = 0, f; f = files[i]; i++) {
    totalSize += (f.size / MBtoB);
  }
  totalSize = Number(totalSize).toFixed(2);
  if (totalSize < maxUploadSizeMBWithoutWarning)
    uploadFiles(files, coords);
  else if (totalSize > maxUploadSizeMB) {
    swal({
      type: "warning",
      title: "Maximum upload size limited to " + maxUploadSizeMB + " MB",
      text: "You intend to transfer " + files.length + " files with total size of: " + totalSize + " MB.",
      confirmButtonColor: "#DD6B55",
      confirmButtonText: "OK",
      closeOnConfirm: true
    });
    clearUploadList();
  }
  else {
    swal({
        type: "warning",
        title: "Are you sure?",
        text: "You intend to transfer " + files.length + " files with total size of: " + totalSize + " MB.",
        confirmButtonColor: "#DD6B55",
        confirmButtonText: "Yes",
        cancelButtonText: "No",
        closeOnConfirm: true,
        closeOnCancel: true,
        showCancelButton: true
      },
      function (isConfirm) {
        if (isConfirm)
          uploadFiles(files, coords);
        else
          clearUploadList();
      });
  }
}

function queryThumbnail(tile) {
  var url = restUrl + "windows/" + tile["uuid"] + "/thumbnail";
  $.ajax({
    url: url,
    type: 'GET',
    tile: tile,
    tryCount: 0,
    retryLimit: 10,
    success: function (resp, textStatus, xhr) {
      var that = this;
      if (xhr.status == 200) {
        $('#img' + tile.uuid).attr("src", resp)
      }
      else if (xhr.status == 204) {
        setTimeout(function () {
          $.ajax(that);
        }, 1000)
      }
    }
  });
}

function removeCurtain(type) {
  $('#' + type).remove()
}

function requestPUT(command, parameters, callback) {
  request("PUT", command, JSON.stringify(parameters), callback)
}

function sendAppJsonRpc(method, parameters, callback) {
  sendJsonRpc("application", method, parameters, callback);
}

function sendSceneJsonRpc(method, parameters, callback) {
  sendJsonRpc("controller", method, parameters, callback);
}

function sendJsonRpc(endpoint, method, parameters, callback) {
  if (typeof sendJsonRpc.counter == 'undefined') {
    sendJsonRpc.counter = 0;
  }
  var jsonrpc = {"jsonrpc": "2.0", "method": method, "params": parameters, "id": sendJsonRpc.counter++};
  request("POST", endpoint, JSON.stringify(jsonrpc), function (response) {
    if (response && response.hasOwnProperty('error')) {
      var err = response.error;
      var msg = "Reason: " + err.message + " (" + err.code + ")";
      alertPopup("Error executing '" + method + "'", msg);
    }
    else {
      callback();
    }
  });
}

function request(method, command, parameters, callback)
{
  if (locked) {
    displayWallLock();
    return;
  }

  var xhr = new XMLHttpRequest();
  xhr.open(method, restUrl + command, true);
  xhr.responseType = "json";
  if(parameters == null)
    xhr.send(null)
  else
    xhr.send(parameters);

  xhr.onload = function () {
    if (xhr.status === 400)
      alertPopup("Something went wrong", "Issue at: " + restUrl + command);
    else if (xhr.status === 403) {
      if (locked)
        displayWallLock();
      else
        location.reload();
    }
    else if (xhr.status === 502) {
      alertPopup("Something went wrong", "A proxy error (502) occured for: " + restUrl + command);
    }
    if (xhr.readyState === XMLHttpRequest.DONE && (xhr.status === 200 || xhr.status === 403)) {
      if (callback !== null)
        callback(xhr.response);
    }
  };
  xhr.onerror = function () {
    alertPopup("Something went wrong", "Issue at: " + restUrl + command)
  }
}

function resizeOnWheelEvent(event, tile) {
  // do nothing if a window is in focus mode
  if (tile.focus && !tile.fullscreen)
    return;

  var oldWidth = $('#' + tile.uuid).width() / zoomScale;
  var oldHeight = $('#' + tile.uuid).height() / zoomScale;
  var incrementSize = wallHeight / 10;

  var newWidth = 0;
  var newHeight = 0;

  // Zoom out
  if (event.detail > 0 || event.originalEvent.wheelDelta < 0) {
    // do nothing if window already at minimum size
    if (tile.height === tile.minHeight || tile.width === tile.minWidth)
      return;

    newWidth = Math.round(oldWidth - incrementSize);
    newHeight = Math.round(oldHeight - incrementSize);
    // make sure new size is not smaller than minimum dimensions served from rest api a windows
    if (newHeight < tile.minHeight || newWidth < tile.minWidth) {
      newHeight = tile.minHeight;
      newWidth = tile.minWidth;
    }
  }
  // Zoom in
  else {
    newWidth = oldWidth + incrementSize;
    newHeight = oldHeight + incrementSize;
  }

  var params = {"id": tile.uuid, "w": newWidth, "h": newHeight, "centered": true};
  sendSceneJsonRpc("resize-window", params, updateWall);
}

function saveSession() {
  var uri = $('#sessionNameInput').val();

  if (uri.length == 0)
    return;
  if (!uri.endsWith(".dcx"))
    uri = uri + ".dcx";
  var params = {"uri": uri};
  var exist = false;
  for (var i = 0; i < sessionFiles.length; i++) {
    if (sessionFiles[i].text == uri)
      exist = true;
  }
  swal({
      type: "warning",
      title: "Are you sure?",
      text: exist ? "You intend to overwrite an existing session: " + uri : "Save as: " + uri + "?",
      confirmButtonColor: exist ? "#DD6B55" : "#014f86",
      confirmButtonText: "Yes",
      cancelButtonText: "No",
      closeOnConfirm: false,
      closeOnCancel: true,
      showCancelButton: true
    },
    function () {
      swal({
        title: "Saving session, please wait...",
        text: "Your session is being saved as: " + uri,
        confirmButtonColor: "#014f86"
      });
      sendAppJsonRpc("save", params, function(){
        swal({
          type: "success",
          title: "Saved!",
          text: "Your session has been saved as: " + uri,
          confirmButtonText: "OK",
          confirmButtonColor: "#014f86"
        }, function () {
          $('#sessionNameInput').val("");
          $("#sessionMenu").toggle("puff", showEffectSpeed);
          $("#sessionButton").toggleClass("buttonPressed");
        });
        getSessionFolderContent();
        updateWall(); // contents may have been relocated changing some window UUIDs
      });
    });
}

function setBezels() {
  if (bezelHeight <= 0 && bezelWidth <= 0) {
    $("#showBezelButton").remove();
    return;
  }
  stickyBezelSize = (stickyBezelSize / zoomScale);
  $('#wall').css("grid-template-columns", "repeat("+screenCountX +", 1fr)").
  css("grid-template-rows", "repeat("+screenCountY+", 1fr)").
  css("grid-column-gap", bezelWidth).css("grid-row-gap", bezelHeight);

  var totalScreens = screenCountX * screenCountY;
  for (var i = 0; i < totalScreens; i++) {
    let screen = $("<div class=screen id=b" + i + "></div>");
    $("#wall").append(screen);
    screen.css("grid-template-rows", "repeat(" + displaysPerScreenY +", 1fr)");
    screen.css("grid-template-columns", "repeat(" + displaysPerScreenX +", 1fr)");
    screen.css("grid-column-gap", displaysPerScreenX > 1 ? bezelWidth : 0);
    screen.css("grid-row-gap", displaysPerScreenY > 1 ? bezelHeight : 0)

    var totalDisplaysPerScreen = displaysPerScreenX * displaysPerScreenY;

    for (var j = 0; j < totalDisplaysPerScreen; j++) {
      let display = $("<div class='display'> </div>");
      display.css("outline-width", bezelHeight/2);
      screen.append(display)

     var bezels = [{name: 'N', type: 'horizontal'},{name: 'S', type: 'horizontal'},
        {name: 'E', type: 'vertical'},{name: 'W', type: 'vertical'}];
      for (var k = 0; k < bezels.length; k++) {
        let edge = $("<div class='screenbezel' id='" + bezels[k].name + "' > </div>");
        if (bezels[k].type === 'horizontal')
        {
          edge.css("width", "100%");
          edge.css("height", stickyBezelSize);
          if (bezels[k].name == 'S')
            edge.css("top", displayHeight - stickyBezelSize);
        }
        else {
          edge.css("width", stickyBezelSize);
          edge.css("height", "100%");
          if (bezels[k].name == 'E')
            edge.css("left", displayWidth - stickyBezelSize);
        }
        display.append(edge);
      }
     }
  }
  $("#showBezelButton").addClass("buttonPressed");
}

function setCurtain(type) {
  if ($('#' + type).length)
    return;
  var curtain = document.createElement("div");
  curtain.id = type;
  $("#wall").append(curtain);
  curtain = $('#' + type);
  curtain.css("width", wallWidth).css("height", wallHeight);
  curtain.addClass("curtain");
  if (type === fullscreenCurtain) {
    curtain.css("z-index", zIndexFullscreenCurtain);
    curtain.css("opacity", 1)
  }
  else {
    curtain.css("z-index", zIndexFocusCurtain);
    curtain.css("opacity", 0.8)
  }
  curtain.click(function (event) {
    event.stopImmediatePropagation();
    if (fullscreen && focus) {
      sendSceneJsonRpc("exit-fullscreen", getSurfaceParams(), updateWall);
    }
    if (focus && !fullscreen) {
      sendSceneJsonRpc("unfocus-windows", getSurfaceParams(), updateWall);
    }
    if (fullscreen && !focus) {
      sendSceneJsonRpc("exit-fullscreen", getSurfaceParams(), updateWall);
      removeCurtain(fullscreenCurtain);
    }
  });
}

function setHandles(tile) {
  var newLeft;
  var newTop;
  var windowDiv = $('#' + tile.uuid);
  windowDiv.draggable({
    cursor: 'all-scroll',
    snap: '.screen',
    snapTolerance: 10,
    containment: $("#wallWrapper"),
    start: function (event, ui) {
      $('#' + tile.uuid).css("zIndex", 100);
      ui.position.left = 0;
      ui.position.top = 0;
      if (isBezelVisible())
        $('.screenbezel').fadeTo('fast', 0.1).css("pointer-events", "all").css("zIndex", 150);
    },
    drag: function (event, ui) {
      var changeLeft = ui.position.left - ui.originalPosition.left;
      newLeft = ui.originalPosition.left + changeLeft / zoomScale;
      var changeTop = ui.position.top - ui.originalPosition.top;
      newTop = ui.originalPosition.top + changeTop / zoomScale;
      ui.position.left = newLeft;
      ui.position.top = newTop;
    },
    stop: function (event) {
      if ($('.screenbezel').css("opacity") != 0)
        $('.screenbezel').fadeTo('fast', 0).css("pointer-events", "none").css("zIndex", 50);
      var stickToOverlay = $("#stickToOverlay");
      if (stickToOverlay.length>0) {
        var paramsMove = {"id": tile.uuid, "x": parseFloat(stickToOverlay.css("left")) ,
          "y": parseFloat(stickToOverlay.css("top"))};
        var paramsResize = {"id": tile.uuid, "w": parseFloat(stickToOverlay.css("width"))  / zoomScale ,
          "h": parseFloat(stickToOverlay.css("height"))  / zoomScale, "centered": false};
        $("#stickToOverlay").remove()
        sendSceneJsonRpc("resize-window", paramsResize, function () {
          return sendSceneJsonRpc("move-window", paramsMove, updateWall);
          });
      }
      else {
        var params = {"id": tile.uuid, "x": newLeft, "y": newTop};
        sendSceneJsonRpc("move-window", params, updateWall);
      }
    },
    disabled: false,
    cancel: '.windowControls'
  });

  windowDiv.resizable({
    aspectRatio: true,
    start: function (event, ui) {
      ui.originalSize.height = tile.height;
      ui.originalSize.width = tile.width;
    },
    resize: function (event, ui) {
      var changeWidth = ui.size.width - ui.originalSize.width;
      var newWidth = ui.originalSize.width + changeWidth / zoomScale;
      var changeHeight = ui.size.height - ui.originalSize.height;
      var newHeight = ui.originalSize.height + changeHeight / zoomScale;
      ui.size.width = newWidth;
      ui.size.height = newHeight;
      $('#' + tile.uuid).css("width", ui.size.width).css("height", ui.size.height);
    },
    stop: function (event, ui) {
      if (ui.size.height < tile.minHeight || ui.size.width < tile.minWidth) {
        tile.height = tile.minHeight;
        tile.width = tile.minWidth;
      }
      else {
        tile.height = ui.size.height;
        tile.width = ui.size.width
      }
      var params = {"id": tile.uuid, "w": tile.width, "h": tile.height, "centered": false};
      sendSceneJsonRpc("resize-window", params, function () {
        return sendSceneJsonRpc("move-window-to-front", getIdAsObject(tile), updateWall);
      });
    }
  }).on('resize', function (e) {
    e.stopPropagation()
  });
}

function setOption(property) {
  var action = {};
  action[property] = $('#checkbox_' + property).prop('checked');
  requestPUT("options", action, updateOptions);
}

function setScale() {
  var viewportWidth = window.innerWidth;
  var viewportHeight = window.innerHeight;

  var minimalVerticalMargin = 100;
  var minimalHorizontalMargin = 100;

  var scaleV = (viewportWidth - minimalHorizontalMargin) / wallWidth;
  var scaleH = (viewportHeight - minimalVerticalMargin) / wallHeight;

  if (scaleV <= scaleH)
    zoomScale = Math.round(scaleV * 100) / 100;
  else
    zoomScale = Math.round(scaleH * 100) / 100;

  var wallMargin = (window.innerWidth - (wallWidth * zoomScale)) / 2;
  $("#infoBox").css("right", wallMargin - wallOutlineWidth )
  var wall = $("#wall");
  wall.css({transform: 'scale(' + zoomScale + ')'});
  wall.css("margin-left", wallMargin);
  wall.css("margin-right", wallMargin);
  wall.css("margin-top", 25);
  wall.css("margin-bottom", minimalVerticalMargin);
  $(".windowControl").css({ transform: 'scale(1)' });
  $("#wallOutline").css("outline-width", wallOutlineWidth / zoomScale  )
}

function showBezels() {
  $(".screen").toggle();
  if (isBezelVisible())
    $("#showBezelButton").addClass("buttonPressed");
  else
    $("#showBezelButton").removeClass("buttonPressed");
}

function stickToBezel(event, bezel) {
  var screen = $(".ui-draggable-dragging")[0]
  var tile = $("#" + screen.id)
  var aspectRatio = parseFloat(tile.width()) / parseFloat(tile.height())

  var $div = $("<div id='stickToOverlay'></div>");
  var parent = $(bezel).parent()
  parent.append($div)

  // Size
  if (aspectRatio === 1) {
    var newWidth = displayHeight;
    var newHeight = displayHeight;
  }
  else {
    var newWidth = displayHeight * aspectRatio;
    var newHeight = displayHeight;
  }
  // Size - special handling for "overflowing" windows
  if (newWidth > displayWidth) {
    newWidth = displayWidth;
    newHeight = newWidth / aspectRatio;
  }
  if (newHeight > displayHeight) {
    newHeight = displayHeight;
    newWidth = newHeight / aspectRatio;
  }
  // Size - special handling for minHeight, minWidth exceeding the screen size
  if (newHeight < parseFloat(tile.css("minHeight"))) {
    newHeight = parseFloat(tile.css("minHeight"));
    newWidth = newHeight * aspectRatio;
  }
  else if (newWidth < parseFloat(tile.css("minWidth"))) {
    newWidth = parseFloat(tile.css("minWidth"));
    newHeight = newWidth * aspectRatio;
  }

  // Anchors
  var parentLeft = parent.offset().left - $("#wall").offset().left
  var parentTop = parent.offset().top - $("#wall").offset().top
  var left = parentLeft / zoomScale

  var right = (parentLeft / zoomScale + displayWidth) - newWidth;
  var centerV = (parentTop / zoomScale + 0.5 * displayHeight) - 0.5 * newHeight;
  var centerH = (parentLeft / zoomScale + 0.5 * displayWidth) - newWidth * 0.5;
  var bottom = (parentTop / zoomScale + displayHeight) - newHeight;
  var top = parentTop / zoomScale;

  $div.css("width", newWidth);
  $div.css("height", newHeight);
  $div.fadeIn('10')

  // Placement based on the move direction and aspect ratio
  var vertical = aspectRatio < 1;
  var dir = bezel.id;

  if (vertical) {
    if (dir == 'E') {
      $div.css("left", right);
      $div.css("top", top);
    }
    else if (dir == 'W') {
      $div.css("left", left);
      $div.css("top", top);
    }
    else if (dir == 'N' || dir == 'S') {
      $div.css("left", centerH);
      $div.css("top", top);
    }
  }
  else {
    if (dir == 'N') {
      $div.css("left", centerH);
      $div.css("top", top);
    }
    else if (dir == 'S') {
      $div.css("top", bottom);
      $div.css("left", centerH);
    }
    else if (dir == 'E') {
      $div.css("top", centerV);
      $div.css("left", right);
    }
    else if (dir == 'W') {
      $div.css("top", centerV);
      $div.css("left", left);
    }
    // Align a window exceeding a screen to the left
    if ((dir == 'N' || dir == 'S') && newWidth > displayWidth)
      $div.css("left", left);
  }
}

function updateContentsDialog(windowList) {
  var contentsLinksHtml = windowList.map(function (window) {
    return "<a href='file://" + window.uri + "'>" + window.uri + "</a>";
  }).join("<br/>");
  var downloadData = "data:text/html," + contentsLinksHtml;
  var downloadLink = '<a download="contents.html" href="' + downloadData + '">Export</a>';
  var htmlContent = contentsLinksHtml + "<br/><br/>" + downloadLink;
  $("#contentsDialog").html(htmlContent);
}

function updateOptions() {
  var options = [];
  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "options", true);
  xhr.onload = function () {
    options = JSON.parse(xhr.responseText);
    for (var property in options) {
      if (options.hasOwnProperty(property)) {
        if (typeof (options[property]) !== "boolean")
          continue;
        $('#checkbox_' + property).prop('checked', options[property]);
      }
    }
  };
  xhr.send(null);
}

function updateWindow(tile) {
  var windowDiv = $('#' + tile.uuid);
  // don't use minHeight, minWitdh and zIndex from REST interface for focused window
  // or zIndex for fullscreen window
  windowDiv.css("min-height", tile.focus ? 0 : tile.minHeight);
  windowDiv.css("min-width", tile.focus ? 0 : tile.minWidth);
  windowDiv.css("zIndex", tile.fullscreen ? zIndexFullscreen : tile.focus ? zIndexFocus : tile.z);
  windowDiv.css("top", tile.y);
  windowDiv.css("left", tile.x);
  windowDiv.css("height", tile.height);
  windowDiv.css("width", tile.width);
  windowDiv.css("visibility", tile.visible ? "visible" : "hidden");

  if (tile.selected)
    markAsSelected(tile);
  else
    markAsUnselected(tile);

  if (tile.focus)
    markAsFocused(tile);
  else if (tile.fullscreen)
    markAsFullscreen(tile);
  else
    markAsStandard(tile);

  if (locked)
    markAsLocked(tile);
}

function updateWall() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "windows", true);
  xhr.onload = function () {
    if (xhr.status === 200) {
      var jsonList = JSON.parse(xhr.responseText)[activeSurfaceIndex]["windows"];

      updateContentsDialog(jsonList);

      var jsonUuids = jsonList.map(function (element) {
        return element.uuid;
      });
      var windowUuids = windowList.map(function (element) {
        return element.uuid;
      });
      closeRemovedWindows(jsonUuids, windowUuids);

      focus = false;
      fullscreen = false;

      // Add new windows and updated exisiting ones if they have changed
      for (var i = 0; i < jsonList.length; ++i) {
        var window = jsonList[i];
        if (window.focus)
          focus = true;
        if (window.fullscreen)
          fullscreen = true;

        var tileIndex = windowUuids.indexOf(window.uuid);
        if (tileIndex === -1) {
          createWindow(window);
        }
        else {
          var tile = windowList[tileIndex];
          if (!checkIfEqual(window, tile)) {
            copy(window, tile);
            updateWindow(tile);
          }
        }
      }

      if (fullscreen)
        setCurtain(fullscreenCurtain);
      else
        removeCurtain(fullscreenCurtain);

      if (focus)
        setCurtain(focusCurtain);
      else
        removeCurtain(focusCurtain);
    }

    function closeRemovedWindows(jsonUuids, windowUuids) {
      for (var i = 0; i < windowUuids.length; ++i) {
        if (jsonUuids.indexOf(windowUuids[i]) === -1) {
          windowList.splice(windowList.findIndex(function (element) {
            return element.uuid === windowUuids[i];
          }), 1);
          $('#' + windowUuids[i]).remove();
        }
      }
    }
  };
  xhr.send(null);
  xhr.onerror = function () {
    alertPopup("Something went wrong.", "Tide REST interface not accessible at: " + restUrl);
  };

  var lockCheck = new XMLHttpRequest();
  lockCheck.open("GET", restUrl + "lock", true);
  lockCheck.onload = function () {
    var lock = JSON.parse(lockCheck.responseText);
    var newLockStatus = lock["locked"];
    if (newLockStatus === locked)
      return;

    locked = newLockStatus;
    if (locked) {
      $('#wallLock').show();
      $("#lockIcon").attr("src", lockImageUrl);
      $(".menuButton").prop("disabled", true);
    }
    else {
      $('#wallLock').hide();
      $("#lockIcon").attr("src", unlockImageUrl);
      $(".menuButton").prop("disabled", false);
    }
    // update all windows to disable or re-enable the draggable / resizable behaviour
    for (var i = 0; i < windowList.length; ++i)
      updateWindow(windowList[i]);
  };
  lockCheck.send(null);

  var screenCheck = new XMLHttpRequest();
  screenCheck.open("GET", restUrl + "stats", true);
  screenCheck.onload = function () {
    var config = JSON.parse(screenCheck.responseText)["screens"];
    var monitorState = config["state"];
    if (monitorState === "ON") {
      $("#screenIcon").attr("src", screenOnImageUrl);
    }
    else if (monitorState === "OFF") {
      $("#screenIcon").attr("src", screenOffImageUrl);
    }
    else
      $("#screenIcon").removeAttr("src");
  };
  screenCheck.send(null);
}

function uploadFiles(files, coords) {
  var url = "upload";
  var requests = [];
  for (var i = 0; i < files.length; i++) {
    (function (i) {
      var file = files[i];
      requests[i] = new XMLHttpRequest();
      requests[i].open('POST', restUrl + url, true);
      requests[i].onload = function () {
        if (requests[i].readyState === XMLHttpRequest.DONE && requests[i].status === 200) {
          var xhr2 = new XMLHttpRequest();
          var index = output.findIndex(function (element) {
            return element.id == file.id
          });
          output[index].started = true;

          var cancelIcon = document.createElement("span");
          cancelIcon.innerHTML = "<font color='red' >&#x2718; </font>";
          cancelIcon.class = "cancelUploadSpan";
          var loadingGif = document.createElement("img");
          loadingGif.src = loadingGifUrl;
          $('#' + file.id).append(loadingGif).append(cancelIcon);

          cancelIcon.addEventListener("click", function () {
            index = output.findIndex(function (element) {
              return element.id == file.id
            });
            output[index].finished = true;
            xhr2.abort();
            var fileLi = $('#' + file.id);
            fileLi.find('img:first').remove();
            fileLi.find('span:last').remove();
            fileLi.append("<font color='red'> &#x2716; cancelled</font>");
          });

          var fileName = decodeURI(JSON.parse(this.responseText)["url"]);
          xhr2.open('PUT', restUrl + url + "/" + fileName, true);
          xhr2.onload = function () {
            var fileLi = $('#' + file.id);
            if (xhr2.readyState === XMLHttpRequest.DONE && xhr2.status === 201) {
              index = output.findIndex(function (element) {
                return element.id == file.id
              });
              output[index].finished = true;
              var success = JSON.parse(xhr2.responseText)["info"];
              fileLi.find('img:first').remove();
              fileLi.find('span:last').remove();
              fileLi.append("<font color='green'> &#10004;" + success + "</font>");
              updateWall();
            }
            else {
              output[index].finished = true;
              var error = JSON.parse(xhr2.responseText)["info"];
              fileLi.find('img:first').remove();
              fileLi.find('span:last').remove();
              fileLi.append("<font color='red'> &#x2716;" + error + "</font>");
            }
            $('#file-form').find("input[type=file]").val("");
          };
          xhr2.send(file);
        }
        else
          console.log('ENDPOINT REGISTRATION: An error occurred!');
      };
      var body = { "filename": (file.name), "x": coords["x"], "y": coords["y"] };
      requests[i].send(JSON.stringify(body));
    })(i)
  }
}
