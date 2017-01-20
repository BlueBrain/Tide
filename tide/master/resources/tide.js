"use strict";
var closeIcon;
var focus;
var focusIcon;
var fullscreen;
var fullscreenIcon;
var timer;
var sessionDir;
var sessionFiles = [];
var wallWidth;
var wallHeight;
var windowList = [];
var zoomScale;

window.onresize = setScale;

$(init);

function bootstrapMenus() {

  $("#addButton").click(function (e) {
    $('#sessionMenu').hide("puff", 200);
    $('#fsMenu').css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed)
  });


  $("#sessionButton").click(function (e) {
    $('#sessionMenu').css("left", e.pageX - 50 + 'px').css("top", 25).toggle("puff", showEffectSpeed);
    $('#fsMenu').hide("puff", showEffectSpeed)

  });
  $("#sessionMenu").mouseleave(function () {
    $('#sessionMenu').hide("puff", showEffectSpeed)
  });

  $("#fsMenu").mouseleave(function () {
    $('#fsMenu').hide("puff", showEffectSpeed)
  });
}

function getFileSystemContent(path) {
  requestPUT("ls", JSON.stringify({"dir": path}));
  window.setTimeout(function () {
    var xhr = new XMLHttpRequest();
    var url = (path !== "/") ? restUrl + "ls/" + encodeURI(path) : restUrl + "ls";
    var files = [];
    xhr.open("GET", url, true);
    xhr.overrideMimeType("application/json");

    xhr.onload = function () {
      var data = JSON.parse(xhr.responseText);
      var filesCount = 0;
      for (var i = 0; i < data.length; i++)
        if (data[i].file  && !(/.*.dcx/.test(data[i].name)))
          filesCount += 1;

      for (var i = 0; i < data.length; i++) {
        var file = {
          text: data[i].name,
          path: data[i].path,
          dir: data[i].dir,
          file: data[i].file,
          session: /.*.dcx/.test(data[i].name)
        };

        file.icon = data[i].dir ? "glyphicon glyphicon-folder-close" : "glyphicon glyphicon-file";
        if (file.text === "..") {
          file.icon = "glyphicon glyphicon-level-up";
          file.text = "Move up";
          file.backColor = "#d1e2ee";
        }
        if (file.text === ".") {
          file.text = filesCount > 0 ? " Open all regular files: " + filesCount : " No regular files to open here";
          file.icon = "glyphicon glyphicon-folder-open";
          file.backColor = filesCount > 0 ? "#1a6092" : "ligthgrey";
          file.currentDir = true
        }
        if (data[i].path !== "../")
          files.push(file)
      }

      $('#fsMenu').treeview({
        data: files,
        searchResultBackColor: "#014f86",
        highlightSelected: true

      });
      $('#fsMenu').on('nodeSelected', function (event, data) {
        // PARENT FOLDER
        if (data.text == "..") {
          getFileSystemContent(data.path)
        }
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
        // FOLDER BELOW - query for it content and update the view
        else if (data.dir)
          getFileSystemContent(data.path);
        // SESSION
        else if (data.session) {
          requestPUT("load", JSON.stringify({"uri": data.text}), updateWall);
          window.setTimeout(function () {
            $('#fsMenu').treeview('toggleNodeSelected', [data.nodeId, {silent: true}]);
            updateWall()
          }, sessionLoadingTimeout)
        }
        // REGULAR FILE
        else if (data.file) {
          requestPUT("open", JSON.stringify({"uri": data.path}), updateWall);
          window.setTimeout(function ()
          {
            $('#fsMenu').treeview('toggleNodeSelected', [data.nodeId, {silent: true}]);
          }, fileLoadingTimeout)
        }
      })
    };
    xhr.send(null);
  }, directoryListTimeout)
}

function getSessionFolderContent() {

  var url = restUrl + "ls/" + sessionDir;
  requestPUT("ls", JSON.stringify({"dir": sessionDir}));
  window.setTimeout(function ()
  {
    var xhr = new XMLHttpRequest();
    console.log(url);
    var data;
    sessionFiles = [];
    xhr.open("GET", encodeURI(url), true);
    xhr.overrideMimeType("application/json");
    xhr.onload = function () {
      data = JSON.parse(xhr.responseText);
      for (var i = 0; i < data.length; i++) {
        var file = {
          text: data[i].name,
          path: data[i].path,
          dir: data[i].dir,
          file: data[i].file,
          session: /.*.dcx/.test(data[i].name)
        };
        file.icon = data[i].dir ? "glyphicon glyphicon-chevron-right" : "glyphicon glyphicon-file";
        file.color = data[i].dir ? "grey" : "black";
        // Add only dcx files
        if (data[i].file && /.*.dcx$/.test(data[i].name)) {
          sessionFiles.push(file)
        }
      }

      $('#sessionTree').treeview({
        data: sessionFiles,
        searchResultBackColor: "#014f86",
        highlightSelected: true
      });
      $('#sessionTree').on('nodeSelected', function (event, data) {
        if (data.session) {
          requestPUT("load", JSON.stringify({"uri": data.text}));
          window.setTimeout(function ()
          {
            $('#sessionTree').treeview('toggleNodeSelected', [data.nodeId, {silent: true}]);
            updateWall()
          }, sessionLoadingTimeout)
        }
      })
    };
    xhr.send(null);
  }, directoryListTimeout);
}

function saveSession() {
  var uri = $('#sessionNameInput').val();
  if (!uri.endsWith(".dcx"))
    uri=uri+".dcx";
  var params = JSON.stringify({"uri": uri});
  var exist = false;
  for (var i = 0; i < sessionFiles.length; i++) {
    if (sessionFiles[i].text == uri )
      exist = true
  }
  swal({
      type: "warning",
      title: "Are you sure?",
      text: exist ? "You intend to overwrite an existing session: " + uri : "Save as: "+ uri +"?",
      confirmButtonColor: exist ? "#DD6B55" : "#014f86",
      confirmButtonText: "Yes",
      cancelButtonText: "No",
      closeOnConfirm: false,
      closeOnCancel: true,
      showCancelButton: true
    },
    function () {
      requestPUT("save", params);
      swal({
        title: "Saved!",
        text: "Your file has been saved as: " + uri ,
        type: "success",
        confirmButtonText: "OK",
        confirmButtonColor: "#014f86"
      }, function () {
        getSessionFolderContent();
        $('#sessionNameInput').val("");
      });
    });
}

function alertPopup(title, text) {
  swal({
      title: title,
      text: text,
      confirmButtonColor: "#014f86",
      confirmButtonText: "Refresh page!",
      closeOnConfirm: false
    },
    function () {
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
      requestPUT("unfocusWindow", jsonUuidHelper(tile), function () {
        return requestPUT("close", jsonUuidHelper(tile), updateWall);
      });
    else
      requestPUT("close", jsonUuidHelper(tile), updateWall);
    $('#' + tile.uuid).remove();
  };

  closeButton.appendChild(createIcon(closeIcon));
  return closeButton;
}

function createFocusButton(tile) {
  var focusButton = createButton("focusButton", tile);
  focusButton.style.visibility = tile.selected ? "visible" : "hidden";
  focusButton.onclick = function (event) {
    event.stopImmediatePropagation();
    if (!focus)
      requestPUT("focusWindows", null, updateWall);
    else {
      tile.selected = false;
      requestPUT("unfocusWindow", jsonUuidHelper(tile), updateWall);
    }
  };

  focusButton.appendChild(createIcon(focusIcon));
  return focusButton;
}

function createIcon(type) {
  var icon = type.cloneNode(true);
  icon.setAttribute('class', 'controlIcon');
  return icon;
}

function createFullscreenButton(tile) {
  var fullscreenButton = createButton("fsButton", tile);
  fullscreenButton.style.visibility = tile.mode === modeFullscreen ? "hidden" : "visible";
  fullscreenButton.onclick = function (event) {
    requestPUT("moveWindowToFullscreen", jsonUuidHelper(tile), updateWall);
    event.stopImmediatePropagation();
  };
  fullscreenButton.appendChild(createIcon(fullscreenIcon));
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
  var thumbnail = new Image();
  thumbnail.id = "img" + tile["uuid"];
  thumbnail.className = "thumbnail";
  windowDiv.appendChild(thumbnail);
  queryThumbnail(tile);

  var controlDiv = document.createElement("div");
  controlDiv.className = "windowControls";
  controlDiv.appendChild(createCloseButton(tile));
  controlDiv.appendChild(createFullscreenButton(tile));
  controlDiv.appendChild(createFocusButton(tile));
  windowDiv.appendChild(controlDiv);

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
      requestPUT("toggleSelectWindow", jsonUuidHelper(tile),
        function () {
          return requestPUT("moveWindowToFront", jsonUuidHelper(tile), updateWall)
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

function init() {
  bootstrapMenus();
  //Work-around for zeroEQ not setting proper MIME for svg
  fullscreenIcon = getIcon(fullscreenImageUrl);
  focusIcon = getIcon(focusImageUrl);
  closeIcon = getIcon(closeImageUrl);

  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "config", true);
  xhr.overrideMimeType("application/json");
  xhr.onload = function () {
    var config = JSON.parse(xhr.responseText)["config"];
    wallWidth = config["wallSize"]["width"];
    wallHeight = config["wallSize"]["height"];
    setScale();

    var wall = $("#wall");
    wall.css("background-color", config["backgroundColor"]);
    wall.css("width", wallWidth).css("height", wallHeight);
    wall.click(function () {
      if (windowList.length > 0) {
        requestPUT("deselectWindows", null, updateWall);
      }

    });
    $("#wallOutline").css("width", wallWidth).css("height", wallHeight);

    $("#buttonContainer").append("Tide ", config["version"], " rev ",
      "<a href=\"https://github.com/BlueBrain/Tide/commit/" + config["revision"] + "\">" + config["revision"],
      " </a>", " running on ", config["hostname"], " since ", config["startTime"]);
    getFileSystemContent("/");
    sessionDir = config["sessionDir"].replace(config["contentDir"]+"/","");
    getSessionFolderContent();
    updateWall();
  };

  xhr.onerror = function () {
    alertPopup("Something went wrong.", "Tide REST interface not accessible at: " + restUrl);
  };
  xhr.send(null);
  $("#exitFullscreenButton").on("click", function () {
    requestPUT("exitFullScreen", null, updateWall);
    removeCurtain(fullscreenCurtain);
  });
  $("#closeAllButton").on("click", function () {
    requestPUT("clear", null, updateWall);
  });
}

function jsonUuidHelper(tile) {
  return JSON.stringify({"uri": tile.uuid})
}

function getIcon(url) {
  var element;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", url, false);
  xhr.overrideMimeType("image/svg+xml");
  xhr.onload = function () {
    if (xhr.status === 200)
      element = xhr.responseXML.documentElement;
    else
      element = document.createElement(null);
  };
  xhr.send(null);
  return element;
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
      requestPUT("moveWindow", params, updateWall);
    },
    disabled: false
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
      var params = JSON.stringify({"uri": tile.uuid, "w": tile.width, "h": tile.height, "centered": 0});
      requestPUT("resizeWindow", params, function () {
        return requestPUT("moveWindowToFront", jsonUuidHelper(tile), updateWall);
      });
    }
  }).on('resize', function (e) {
    e.stopPropagation()
  });

}

function queryThumbnail(tile) {
  var xhr = new XMLHttpRequest();
  var url = restUrl + "windows/" + tile["uuid"] + "/thumbnail";
  xhr.open("GET", url, true);
  xhr.overrideMimeType("data");
  xhr.onload = function () {
    var base64Img = 'data:image/png;base64,' + xhr.responseText;
    $('#img' + tile.uuid).attr("src", base64Img).mousedown(function (e) {
      e.preventDefault()
    });
  };
  xhr.send(null);
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

function removeCurtain(type) {
  $('#' + type).remove()
}

function requestPUT(command, parameters, callback) {
  var xhr = new XMLHttpRequest();
  xhr.open("PUT", restUrl + command, true);
  xhr.responseType = "json";
  xhr.send(parameters);
  xhr.onload = function () {
    if (xhr.status === 400)
      alertPopup("Something went wrong", "Issue at: " + restUrl + command);
    if (xhr.readyState === 4 && xhr.status === 200) {
      if (callback != null)
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
  if (event.detail < 0 || event.originalEvent.wheelDelta > 0) {
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

  var params = JSON.stringify({"uri": tile.uuid, "w": newWidth, "h": newHeight, "centered": 1});
  requestPUT("resizeWindow", params, updateWall);
}

function setCurtain(type) {
  if ( $('#'+type).length )
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
      requestPUT("exitFullScreen", null, updateWall);
    }
    if (focus && !fullscreen) {
      requestPUT("unfocusWindows", null, updateWall);

    }
    if (fullscreen && !focus) {
      requestPUT("exitFullScreen", null, updateWall);
      removeCurtain(fullscreenCurtain);
    }
  });
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

function updateTile(tile) {
  var windowDiv = $('#' + tile.uuid);
  windowDiv.css("min-height", tile.minHeight);
  windowDiv.css("min-width", tile.minWidth);
  windowDiv.css("top", tile.y);
  windowDiv.css("left", tile.x);
  windowDiv.css("height", tile.height);
  windowDiv.css("width", tile.width);
  windowDiv.css("zIndex", tile.z);
}

function updateWall() {
  enableHandles();
  fullscreen = false;
  focus = false;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", restUrl + "windows", true);
  xhr.overrideMimeType("application/json");
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
