"use strict";
var bezelWidth;
var bezelHeight;
var bezelsVertical;
var bezelsHorizontal;
var focus;
var fullscreen;
var screenCountX;
var screenCountY;
var sessionFiles = [];
var timer;
var wallWidth;
var wallHeight;
var windowList = [];
var zoomScale;
var output = [];
var filters = [];
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

function bootstrapMenus() {

  $("#addButton").click(function (e) {
    $("#uploadMenu,#sessionMenu,#optionsMenu,#appsMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
        e.stopPropagation()
    });

    $("#fsMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#addButton)").removeClass("buttonPressed");
    $("#addButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  $("#sessionButton").click(function (e) {
    $("#uploadMenu,#fsMenu,#optionsMenu,#appsMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
      e.stopPropagation()
    });

    $("#sessionMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#sessionButton)").removeClass("buttonPressed");
    $("#sessionButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  $("#uploadButton").click(function (e) {
    $("#sessionMenu,#fsMenu,#optionsMenu,#appsMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
      e.stopPropagation()
    });

    $("#uploadMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#uploadButton)").removeClass("buttonPressed");
    $("#uploadButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  $("#optionsButton").click(function (e) {
    $("#sessionMenu,#fsMenu,#uploadMenu,#appsMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
    });

    $("#optionsMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#optionsButton)").removeClass("buttonPressed");
    $("#optionsButton").toggleClass("buttonPressed");
    updateOptions();
    e.stopPropagation()
  });

  $("#appsButton").click(function (e) {
    $("#sessionMenu,#fsMenu,#uploadMenu,#optionsMenu").each(function () {
      $(this).hide("puff", showEffectSpeed);
    });

    $("#appsMenu").css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $(".menuButton:not(#appsButton)").removeClass("buttonPressed");
    $("#appsButton").toggleClass("buttonPressed")
    e.stopPropagation()
  });

  $("#browseUrlInput").on("click", function (e) {
    e.stopPropagation()
  });

  getOptions();
}

function boostrapUpload() {
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
  requestPUT("browse", JSON.stringify({"uri": (url)}), updateWall);
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
    tile1.z === tile2.z
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
    windowList.splice(windowList.findIndex(function (element) {
      return element.uuid === tile.uuid;
    }), 1);
    if (tile.focus)
      requestPUT("unfocus-window", jsonUuidHelper(tile), function () {
        return requestDELETE("windows/"+tile.uuid, updateWall);
      });
    else
      requestDELETE("windows/"+tile.uuid, updateWall);
    $('#' + tile.uuid).remove();
  };
  var icon = document.createElement("img");
  icon.src = closeImageUrl;
  closeButton.appendChild(icon);
  return closeButton;
}

function createFocusButton(tile) {
  var focusButton = createButton("focusButton", tile);
  focusButton.style.visibility = tile.selected ? "visible" : "hidden";
  focusButton.onclick = function (event) {
    event.stopImmediatePropagation();
    if (!focus)
      requestPUT("focus-windows", null, updateWall);
    else {
      tile.selected = false;
      requestPUT("unfocus-window", jsonUuidHelper(tile), updateWall);
    }
  };
  var icon = document.createElement("img");
  icon.src = focusImageUrl;
  focusButton.appendChild(icon);
  return focusButton;
}

function createFullscreenButton(tile) {
  var fullscreenButton = createButton("fsButton", tile);
  fullscreenButton.style.visibility = tile.mode === modeFullscreen ? "hidden" : "visible";
  fullscreenButton.onclick = function (event) {
    requestPUT("move-window-to-fullscreen", jsonUuidHelper(tile), updateWall);
    event.stopImmediatePropagation();
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

  if (tile.mode === modeFocus)
    disableHandles();
  if (tile.mode === modeFullscreen)
    disableHandlesForFullscreen(tile);

  if (tile.selected)
    markAsSelected(tile);
  if (tile.focus)
    markAsFocused(tile);
  if (tile.fullscreen)
    markAsFullscreen(tile);

  $('#' + tile.uuid).bind('mousewheel DOMMouseScroll', _.throttle(function (event) {
      return resizeOnWheelEvent(event, tile)
    }, zoomInterval)
  );
  windowDiv.onclick = function (event) {
    if (!fullscreen && !focus) {
      requestPUT("toggle-select-window", jsonUuidHelper(tile),
        function () {
          return requestPUT("move-window-to-front", jsonUuidHelper(tile), updateWall)
        }
      );
    }
    event.stopImmediatePropagation();
  };
  updateTile(tile);
}

function disableHandles() {
  var draggableObj = $('.ui-draggable');
  draggableObj.draggable('disable');
  draggableObj.resizable('disable');
  draggableObj.removeClass('active').addClass('inactive');
}

function disableHandlesForFullscreen(tile) {
  $('.ui-draggable').draggable('disable');
  $('.ui-resizable').resizable('disable');
  var draggableObj = $('#' + tile.uuid);

  if (tile.height > wallHeight && tile.width > wallWidth)
    draggableObj.draggable('enable');
  else if (tile.height > wallHeight)
    draggableObj.draggable({axis: "y"}).draggable('enable');
  else if (tile.width > wallWidth)
    draggableObj.draggable({axis: "x"}).draggable('enable');
  draggableObj.removeClass('active').addClass('inactive');
}

function enableControls(tile) {
  $('#' + tile.uuid).removeClass("windowFullscreen");
  $('#fsButton' + tile.uuid).css("visibility", "visible");
  $('#sb' + tile.uuid).css("visibility", "visible");
  $('#closeButton' + tile.uuid).css("visibility", "visible");
}

function enableHandles() {
  var draggableObj = $('.ui-draggable');
  draggableObj.draggable('enable');
  draggableObj.resizable('enable');
  draggableObj.draggable({axis: false});
  draggableObj.removeClass('inactive').addClass('active');
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
                requestPUT("open", JSON.stringify({"uri": data.path}), updateWall);
                $('#fsMenu').hide()
              }
            });
        }
        else {
          requestPUT("open", JSON.stringify({"uri": data.path}), updateWall);
          $('#fsMenu').treeview('toggleNodeSelected', [data.nodeId, {silent: true}]);
        }
      }
      // FOLDER - query its content and update the view
      else if (data.dir) {
        getFileSystemContent(data.path);
      }
      // REGULAR FILE
      else {
        requestPUT("open", JSON.stringify({"uri": data.path}), updateWall);
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
        requestPUT("load", JSON.stringify({"uri": data.text}), function () {
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
      title: (notUploaded > 1) ? notUploaded + " unsuported files" : notUploaded + " unsuported file",
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
  boostrapUpload();
  bootstrapMenus();

  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "config", true);
  xhr.onload = function () {
    var config = JSON.parse(xhr.responseText)["config"];
    wallWidth = config["wallSize"]["width"];
    wallHeight = config["wallSize"]["height"];
    screenCountX=config["dimensions"]["screenCountX"];
    screenCountY=config["dimensions"]["screenCountY"];
    bezelWidth=config["dimensions"]["bezelWidth"];
    bezelHeight=config["dimensions"]["bezelHeight"];
    bezelsVertical=config["dimensions"]["bezelsVertical"];
    bezelsHorizontal=config["dimensions"]["bezelsHorizontal"];

    setBezel();
    setScale();

    filters = config["filters"];
    for (var i = 0; i < filters.length; i++)
      filters[i] = filters[i].replace(/\*/g, "");
    $("#file-select").attr("accept", filters);
    var wall = $("#wall");
    wall.css("background-color", config["backgroundColor"]);
    wall.css("width", wallWidth).css("height", wallHeight);

    $('#wallWrapper').click(function(){
      $(".topMenu").hide();
      $(".menuButton").removeClass("buttonPressed");
    });

    wall.click(function () {
        if (windowList.length > 0) {
        requestPUT("deselect-windows", null, updateWall);
      }

    });

    $("#buttonContainer").append("Tide ", config["version"], " rev ",
      "<a href=\"https://github.com/BlueBrain/Tide/commit/" + config["revision"] + "\">" + config["revision"],
      " </a>", " running on ", config["hostname"], " since ", config["startTime"]);
    getFileSystemContent("");
    getSessionFolderContent();
    updateWall();
  };

  xhr.onerror = function () {
    alertPopup("Something went wrong.", "Tide REST interface not accessible at: " + restUrl);
  };
  xhr.send(null);
  $("#exitFullscreenButton").on("click", function () {
    requestPUT("exit-fullscreen", null, updateWall);
    removeCurtain(fullscreenCurtain);
  });
  $("#closeAllButton").on("click", function () {
    requestPUT("clear", null, updateWall);
  });
}

function jsonUuidHelper(tile) {
  return JSON.stringify({"uri": tile.uuid})
}

function markAsFocused(tile) {
  $('#' + tile.uuid).css("z-index", zIndexFocus).css("border", "0px").addClass("windowSelected");
}

function markAsFullscreen(tile) {
  $('#' + tile.uuid).css("z-index", zIndexFullscreen).addClass("windowFullscreen");
  $('#closeButton' + tile.uuid).css("visibility", "hidden");
  $('#fsButton' + tile.uuid).css("visibility", "hidden");
  $('#focusButton' + tile.uuid).css("visibility", "hidden");
}

function markAsSelected(tile) {
  $('#' + tile.uuid).addClass("windowSelected");
  $('#focusButton' + tile.uuid).css("visibility", "visible")
}

function markAsUnselected(tile) {
  $('#' + tile.uuid).removeClass('windowSelected');
  $('#focusButton' + tile.uuid).css("visibility", "hidden")
}

function openWhiteboard() {
  requestPUT("whiteboard", JSON.stringify({}), updateWall);
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

function requestDELETE(command, callback) {
  request("DELETE", command, null, callback)
}

function requestPUT(command, parameters, callback) {
  request("PUT", command, parameters, callback)
}

function request(method, command, parameters, callback)
{
  var xhr = new XMLHttpRequest();
  xhr.open(method, restUrl + command, true);
  xhr.responseType = "json";
  if(parameters == null)
    xhr.send(null)
  else
    xhr.send(parameters)

  xhr.onload = function () {
    if (xhr.status === 400)
      alertPopup("Something went wrong", "Issue at: " + restUrl + command);
    if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 200) {
      if (callback !== null)
        callback();
    }
  };
  xhr.onerror = function () {
    alertPopup("Something went wrong", "Issue at: " + restUrl + command)
  }
}

function resizeOnWheelEvent(event, tile) {
  // do nothing if a window is in focus mode
  if (focus && !fullscreen)
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

  var params = JSON.stringify({"uri": tile.uuid, "w": newWidth, "h": newHeight, "centered": true});
  requestPUT("resize-window", params, updateWall);
}

function saveSession() {
  var uri = $('#sessionNameInput').val();

  if (uri.length == 0)
    return;
  if (!uri.endsWith(".dcx"))
    uri = uri + ".dcx";
  var params = JSON.stringify({"uri": uri});
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
      requestPUT("save", params, getSessionFolderContent);
      swal({
        title: "Saved!",
        text: "Your file has been saved as: " + uri,
        type: "success",
        confirmButtonText: "OK",
        confirmButtonColor: "#014f86"
      }, function () {
        $('#sessionNameInput').val("");
        $("#sessionMenu").toggle("puff", showEffectSpeed);
        $("#sessionButton").toggleClass("buttonPressed");
      });
    });
}

function setBezel() {
  $('#wallOutline').css("grid-template-columns", "repeat("+screenCountX +", 1fr)").
  css("grid-template-rows", "repeat("+screenCountY+", 1fr)").
  css("grid-column-gap", bezelWidth).css("grid-row-gap", bezelHeight);

  var totalScreens = screenCountX  * screenCountY;
  var displayPerScreenVertical  = bezelsVertical + 1;
  var displayPerScreenHorizontal = bezelsHorizontal + 1;

  for (var i = 0; i< totalScreens; i++)
  {
    var div = $("<div class=bezel > </div>");
    $("#wallOutline").append(div);
    div.css("grid-template-columns", "repeat("+displayPerScreenVertical+", 1fr)");
    div.css("grid-template-rows", "repeat("+displayPerScreenHorizontal+ ", 1fr)");
    var totalDisplaysPerScreen = displayPerScreenHorizontal * displayPerScreenVertical;

    for (var j = 0; j < totalDisplaysPerScreen; j++)
      div.append("<div class='bezel'></div>");

    $(".bezel").css("grid-column-gap", bezelWidth).css("grid-row-gap", bezelHeight).hide();
  }
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
      requestPUT("exit-fullscreen", null, updateWall);
    }
    if (focus && !fullscreen) {
      requestPUT("unfocus-windows", null, updateWall);

    }
    if (fullscreen && !focus) {
      requestPUT("exit-fullscreen", null, updateWall);
      removeCurtain(fullscreenCurtain);
    }
  });
}

function setHandles(tile) {
  var newLeft;
  var newTop;
  var windowDiv = $('#' + tile.uuid);
  windowDiv.draggable({
    containment: $("#wallWrapper"),
    start: function (event, ui) {
      $('#' + tile.uuid).css("zIndex", 100);
      ui.position.left = 0;
      ui.position.top = 0;
    },
    drag: function (event, ui) {
      var changeLeft = ui.position.left - ui.originalPosition.left;
      newLeft = ui.originalPosition.left + changeLeft / zoomScale;
      var changeTop = ui.position.top - ui.originalPosition.top;
      newTop = ui.originalPosition.top + changeTop / zoomScale;
      ui.position.left = newLeft;
      ui.position.top = newTop;
    },
    stop: function () {
      var params = JSON.stringify({"uri": tile.uuid, "x": newLeft, "y": newTop});
      requestPUT("move-window", params, updateWall);
    },
    disabled: false,
    cancel : '.windowControls'
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
      var params = JSON.stringify({"uri": tile.uuid, "w": tile.width, "h": tile.height, "centered": false});
      requestPUT("resize-window", params, function () {
        return requestPUT("move-window-to-front", jsonUuidHelper(tile), updateWall);
      });
    }
  }).on('resize', function (e) {
    e.stopPropagation()
  });
}

function setOption(property) {
  var action = {};
  action[property] = $('#checkbox_' + property).prop('checked');
  requestPUT("options", JSON.stringify(action), updateOptions);
}

function setScale() {
  var viewportWidth = window.innerWidth;
  var viewportHeight = window.innerHeight;

  var minimalVerticalMargin = 100;
  var minimalHorizontalMargin = 100;

  var scaleV = (viewportWidth - minimalHorizontalMargin ) / wallWidth;
  var scaleH = (viewportHeight - minimalVerticalMargin ) / wallHeight;

  if (scaleV <= scaleH)
    zoomScale = Math.round(scaleV * 100) / 100;
  else
    zoomScale = Math.round(scaleH * 100) / 100;

  var wallMargin = (window.innerWidth - (wallWidth * zoomScale)) / 2;
  var wall = $("#wall");
  wall.css({transform: 'scale(' + zoomScale + ')'});
  wall.css("margin-left", wallMargin);
  wall.css("margin-right", wallMargin);
  wall.css("margin-top", 25);
  wall.css("margin-bottom", minimalVerticalMargin);
  $(".windowControl").css({transform: 'scale(1)'});
}

function updateOptions() {
  var options = [];
  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "options", true);
  xhr.onload = function () {
    options = JSON.parse(xhr.responseText);
    for (var property in options) {
      if (options.hasOwnProperty(property)) {
        if (typeof(options[property]) !== "boolean")
          continue;
        $('#checkbox_' + property).prop('checked', options[property]);
      }
    }
  };
  xhr.send(null);
}

function showBezel()
{
  $(".bezel").toggle();
}

function updateTile(tile) {
  var windowDiv = $('#' + tile.uuid);
  // don't use minHeight, minWitdh and zIndex from REST interface for focused window
  if (!tile.focus) {
    windowDiv.css("min-height", tile.minHeight);
    windowDiv.css("min-width", tile.minWidth);
    windowDiv.css("zIndex", tile.zIndex);
  }
  else
  {
    windowDiv.css("min-height", 0);
    windowDiv.css("min-width", 0);
    windowDiv.css("zIndex", 100);
  }
  windowDiv.css("top", tile.y);
  windowDiv.css("left", tile.x);
  windowDiv.css("height", tile.height);
  windowDiv.css("width", tile.width);
}

function updateWall() {
  enableHandles();
  fullscreen = false;
  focus = false;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "windows", true);
  xhr.onload = function () {
    if (xhr.status === 200) {
      var jsonList = JSON.parse(xhr.responseText)["windows"];

      // Check if a window has been removed on the wall
      checkForRemoved();

      var jsonUuidList = jsonList.map(function (element) {
        return element.uuid;
      });
      var windowUuidList = windowList.map(function (element) {
        return element.uuid;
      });

      // Loop through data from rest api to add new windows and modify exisiting if different
      for (var i = 0; i < jsonUuidList.length; i++) {
        if (jsonList[i].mode == modeFocus)
          focus = true;
        if (jsonList[i].mode == modeFullscreen)
          fullscreen = true;

        //Window missing. Create it.
        if (windowUuidList.indexOf(jsonUuidList[i]) === -1)
          createWindow(jsonList[i]);

        else {
          var tile = windowList[windowUuidList.indexOf(jsonUuidList[i])];
          //If window has been updated, modify its properties
          if (!checkIfEqual(jsonList[i], tile)) {

            copy(jsonList[i], tile);
            updateTile(tile);

            if (tile.mode === modeFocus)
              disableHandles();
            if (tile.mode === modeFullscreen)
              disableHandlesForFullscreen(tile);

            if (tile.selected)
              markAsSelected(tile);
            else
              markAsUnselected(tile);

            if (tile.focus)
              markAsFocused(tile);

            if (tile.fullscreen)
              markAsFullscreen(tile);
            else
              enableControls(tile);
          }
        }
      }
    }

    function checkForRemoved() {
      var json = jsonList.map(function (element) {
        return element.uuid;
      });
      var list = windowList.map(function (element) {
        return element.uuid;
      });
      for (var i = 0; i < list.length; i++) {
        if (json.indexOf(list[i]) === -1) {
          windowList.splice(windowList.findIndex(function (element) {
            return element.uuid === list[i];
          }), 1);
          $('#' + list[i]).remove();
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

  };
  xhr.send(null);
  xhr.onerror = function () {
    alertPopup("Something went wrong.", "Tide REST interface not accessible at: " + restUrl);
  }
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
      requests[i].send(JSON.stringify({"filename": (file.name),
      "x": coords["x"], "y": coords["y"]
      }));
    })(i)
  }
}
