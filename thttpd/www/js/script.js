window.addEventListener("load", function () {
  const myButton = document.getElementById("myButton");
  const stopRemote = document.getElementById("stop-remote");
  const startRemote = document.getElementById("start-remote");
  const remoteFirmware = document.getElementById("remote-firmware");

  myButton.addEventListener("click", function () {
    toggleLED();
  });

  stopRemote.addEventListener("click", function () {
    stopRemoteFirmware();
  });

  startRemote.addEventListener("click", function () {
    const value = remoteFirmware.value;
    startRemoteFirmware(value);
  });

  getRemoteFirmwares();
  getStats();
  getRPMsg();
  setInterval(getStats, 1000); // Actualiza cada 1 segundo
  setInterval(getRPMsg, 1000); // Actualiza cada 1 segundo
});

function getRPMsg() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      document.getElementById("rpmsg").innerHTML = xhr.responseText;
    }
  };
  xhr.open("GET", "cgi-bin/get_rpmsg.cgi", true);
  xhr.send();
}

function toggleLED() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      alert("response: " + xhr.responseText);
    }
  };
  xhr.open("GET", "cgi-bin/toggle_led.cgi", true);
  xhr.send();
}

function getStats() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      document.getElementById("stats").innerHTML = xhr.responseText;
    }
  };
  xhr.open("GET", "cgi-bin/stats.cgi", true);
  xhr.send();
}

function getRemoteFirmwares() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      document.getElementById("remote-firmware").innerHTML = xhr.responseText;
    }
  };
  xhr.open("GET", "cgi-bin/get_remote_firmwares.cgi", true);
  xhr.send();
}

function startRemoteFirmware(firmware) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      alert("response: " + xhr.responseText);
    }
  };
  xhr.open(
    "GET",
    "cgi-bin/start_remote_firmware.cgi?firmware=" + firmware,
    true,
  );
  xhr.send();
}

function stopRemoteFirmware() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status != 200) {
      alert("failed to stop remote firmware");
    }
  };
  xhr.open("GET", "cgi-bin/stop_remote_firmware.cgi", true);
  xhr.send();
}
