const char *sensors_js = R"==(var s = '{"ds18inner":{"name":"DS18 Inner","value":"25.00","status":1},"ds18prob":{"name":"DS18 Probe","value":"0.00","status":0},"moist":{"name":"Moisture Sensor","value":"109","status":-1},"ldr":{"name":"LDR Sensor","value":"0","status":-1},"dht_hum":{"name":"DHT Humidity","value":"0","status":-1},"dht_temp":{"name":"DHT Temperature","value":"0.00","status":-1},"waterlevel":{"name":"Water Level","value":"50","status":-1},"rain":{"name":"Rain","value":"0","status":-1},"wl0":{"name":"Water Level 0","value":"1","status":-1},"wl1":{"name":"Water Level 1","value":"0","status":-1},"wl2":{"name":"Water Level 2","value":"0","status":-1},"spiffs":{"name":"spiffs","value":"0.04","status":1}}'
var r = '{"ds18inner":{"name":"DS18 Inner","value":"25.00","status":1},"ds18prob":{"name":"DS18 Probe","value":"0.00","status":0},"moist":{"name":"Moisture Sensor","value":"109","status":-1},"ldr":{"name":"LDR Sensor","value":"0","status":1},"dht_hum":{"name":"DHT Humidity","value":"0","status":-1},"dht_temp":{"name":"DHT Temperature","value":"0.00","status":-1},"waterlevel":{"name":"Water Level","value":"50","status":-1},"rain":{"name":"Rain","value":"0","status":-1},"wl0":{"name":"Water Level 0","value":"1","status":-1},"wl1":{"name":"Water Level 1","value":"0","status":-1},"wl2":{"name":"Water Level 2","value":"0","status":-1},"spiffs":{"name":"spiffs","value":"0.98","status":1}}'
var t = '{"pumpinfo":"Last Auto Pump Run: 00:00 01/01/70. || [NEXT] : 04:00 03/01/70. || [NOW] : 00:03 01/01/70","pumpconfirm":"Pump was on for 5000ms.  [00:01]"}'

// Variable to store the previous data
var previousData = {};
var oldDataRaw;
function parsePump(params) {

    var response = JSON.parse(params);
    document.getElementById('pump-info').innerHTML = response.pumpinfo;
    document.getElementById('pump-confirm').innerHTML = response.pumpconfirm;
}

function refreshData() {
    parseData(oldDataRaw);
}

function parseData(params) {
    oldDataRaw = params;
    var response = JSON.parse(params);
    var currentTime = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false }); // Change to show time only
    var html = '<p>Last Received Data: ' + currentTime + '</p>'; // Update display text
    html += '<table>';
    html += '<tr><th>Name</th><th>Value</th><th>Status</th></tr>';
    var hide_offline = document.getElementById('hideofflinesw').checked
    var hide_unknown = document.getElementById('hideunknownsw').checked

    for (var id in response) {
        if (id === "pump-status") {
            document.getElementById('pump-status').classList.toggle('pump-on', response["pump-status"] === "Running");
            document.getElementById('pump-status').innerHTML = response["pump-status"];
        }
        else if (response.hasOwnProperty(id)) {
            var data = response[id];
            var statusText = "";
            var statusClass = "";
            if (data.status === 1) {
                statusText = "Online";
                statusClass = "online";
            } else if (data.status === 0) {
                statusText = "Offline";
                statusClass = "offline";
            } else {
                statusText = "Unknown";
                statusClass = "unknown";
            }
            // Check if the value or status has changed
            var valueChangedClass = (previousData[id] && previousData[id].value !== data.value) ? "changed" : "";
            var statusChangedClass = (previousData[id] && previousData[id].status !== data.status) ? "changed" : "";
            var value = data.value;
            if (data.unit) {
                value += " ";
                value += data.unit;
            }
            console.log(hide_offline)
            if (hide_offline && statusClass === "offline") { }
            else if (hide_unknown && statusClass === "unknown") { }
            else {
                html += '<tr class="' + '">';
                html += '<td>' + data.name + '</td>';
                html += '<td class="' + valueChangedClass + '">' + value + '</td>';
                html += '<td class="' + statusClass + ' ' + statusChangedClass + '">' + statusText + '</td>'; html += '</tr>';
            }
        }
    }
    html += '</table>';
    document.getElementById('server-data').innerHTML = html;

    // Update the previous data with the current data
    previousData = response;
}

// Fetch data every 5 seconds
setInterval(() => {requestData('/sensordata', parseData)}, 1000);
setInterval(() => {requestData('/pumpdata', parsePump)}, 10000);
window.onload = function () {
    parseData(s);
}



function triggerPump(Auto) {
    var activationTime = document.getElementById('pump-time-input').value;
    var forceStart = document.getElementById('force-start-checkbox').checked;


    var requestURL = '/pumpstart?';
    requestURL += 'timer=' + activationTime;
    if (forceStart === true)
        requestURL += '&force=' + forceStart;
    // Add activationTime as parameter if greater than 0

   if (Auto)
    requestURL = '/pumpauto';


    // Add forceStart as parameter
    // Create a new XMLHttpRequest object
    console.log('Sending request to:', requestURL);
    // Define the request
    requestData(requestURL, console.log);
}
)==";