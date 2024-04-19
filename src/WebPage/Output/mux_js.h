const char *mux_js = R"==(var test = '{"muxReadTime":0,"mux0":{"port":0,"analogValue":0,"digitalValue":false,"alias":"  MOIST_1  "},"mux1":{"port":1,"analogValue":0,"digitalValue":false,"alias":"  MOIST_2  "},"mux2":{"port":2,"analogValue":0,"digitalValue":false,"alias":"NOT_DEFINED"},"mux3":{"port":3,"analogValue":0,"digitalValue":false,"alias":"NOT_DEFINED"},"mux4":{"port":4,"analogValue":0,"digitalValue":false,"alias":"NOT_DEFINED"},"mux5":{"port":5,"analogValue":0,"digitalValue":false,"alias":"    RAIN   "},"mux6":{"port":6,"analogValue":0,"digitalValue":false,"alias":"NOT_DEFINED"},"mux7":{"port":7,"analogValue":0,"digitalValue":false,"alias":"NOT_DEFINED"},"mux8":{"port":8,"analogValue":0,"digitalValue":false,"alias":"NOT_DEFINED"},"mux9":{"port":9,"analogValue":0,"digitalValue":false,"alias":"    DHT    "},"mux10":{"port":10,"analogValue":0,"digitalValue":false,"alias":"  DS18b20  "},"mux11":{"port":11,"analogValue":0,"digitalValue":false,"alias":" WL_EXTRA  "},"mux12":{"port":12,"analogValue":0,"digitalValue":false,"alias":" WL_MIDDLE "},"mux13":{"port":13,"analogValue":0,"digitalValue":false,"alias":" WL_BOTTOM "},"mux14":{"port":14,"analogValue":0,"digitalValue":false,"alias":"    LDR    "},"mux15":{"port":15,"analogValue":0,"digitalValue":false,"alias":"NOT_DEFINED"},"sipo0":{"pin":0,"value":0,"alias":"LED_0"},"sipo1":{"pin":1,"value":0,"alias":"LED_1"},"sipo2":{"pin":2,"value":0,"alias":"LED_2"},"sipo3":{"pin":3,"value":0,"alias":"MOIST_EN0"},"sipo4":{"pin":4,"value":0,"alias":"MOIST_EN1"},"sipo5":{"pin":5,"value":1,"alias":"RELAY_PIN"},"sipo6":{"pin":6,"value":0,"alias":"NOT_DEFINED"},"sipo7":{"pin":7,"value":0,"alias":"NOT_DEFINED"}}'
// Variable to store the previous data
var previousData = {};
var oldDataRaw;

function refreshData() {
    parseData(oldDataRaw);
}

function parseData(params) {
    oldDataRaw = params;
    var response = JSON.parse(params);
    var currentTime = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false }); // Change to show time only
    var readTime = new Date((response.muxReadTime + 3600) * 1000).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false }); // Change to show time only
    var time_html = '<p>Last Received Data: ' + currentTime + '</p>'; // Update display text
    time_html += '<p>Read Time: ' + readTime + '</p>';
    var mux_html = '<label>16 CH MUX</label><table>';
    mux_html += '<tr><th>Port</th><th>Alias</th><th>Analog</th><th>Digital</th></tr>';
    var sipo_html = '<label>8 BIT SIPO Shifter</label><table>';
    sipo_html += '<tr><th>Pin</th><th>Alias</th><th>Value</th></tr>';

    document.getElementById('time-data').innerHTML = time_html;
    var hide_nd = document.getElementById('hide-not-defined').checked

    for (var id in response) {
        if (response.hasOwnProperty(id)) {
            //console.log(id.data.alias)
            if (hide_nd && response[id].alias === "NOT_DEFINED") { }
            else if (id.includes("mux") && id != 'muxReadTime') {
                var data = response[id];
                var statusClass = "";
                var digVal = "";
                var digClass = "";
                if (data.digitalValue) {
                    digVal = "HIGH";
                    digClass = "online";
                } else {
                    digVal = "LOW";
                    digClass = "offline";
                }
                // Check if the value or status has changed
                var valueChangedClass = (previousData[id] && previousData[id].value !== data.value) ? "changed" : "";
                var statusChangedClass = (previousData[id] && previousData[id].status !== data.status) ? "changed" : "";
                mux_html += '<tr class="' + '">';
                mux_html += '<td>' + data.port + '</td>';
                mux_html += '<td class="' + valueChangedClass + '">' + data.alias + '</td>';
                mux_html += '<td class="' + statusClass + ' ' + statusChangedClass + '">' + data.analogValue + '</td>';
                mux_html += '<td class="' + digClass + ' ' + statusChangedClass + '">' + digVal + '</td>';
                mux_html += '</tr>';
            }
            else if (id.includes("sipo")) {
                var data = response[id];
                var digVal = "";
                var digClass = "";
                if (data.value) {
                    digVal = "HIGH";
                    digClass = "online";
                } else {
                    digVal = "LOW";
                    digClass = "offline";
                }
                // Check if the value or status has changed
                var valueChangedClass = (previousData[id] && previousData[id].value !== data.value) ? "changed" : "";
                var statusChangedClass = (previousData[id] && previousData[id].status !== data.status) ? "changed" : "";
                sipo_html += '<tr class="' + '">';
                sipo_html += '<td>' + data.pin + '</td>';
                sipo_html += '<td class="' + valueChangedClass + ' sipo" onclick="sendSipo(' + data.pin + ',' + !data.value + ')">' + data.alias + '</td>';
                sipo_html += '<td class="' + digClass + ' ' + statusChangedClass + '">' + digVal + '</td>';
                sipo_html += '</tr>';
            }

        }
    }
    sipo_html += '</table>'
    mux_html += '</table>';
    document.getElementById('mux-data').innerHTML = mux_html;
    document.getElementById('sipo-data').innerHTML = sipo_html;

    // Update the previous data with the current data
    previousData = response;
}

function sendSipo(pin, val) {
    var requestURL = '/sipow?';
    requestURL += 'pin=' + pin;
    // Add activationTime as parameter if greater than 0
    requestURL += '&val=' + val;
    console.log(requestURL);
    requestData(requestURL,parseData);
}
// Fetch data every 5 seconds
setInterval(() => {requestData('/muxdata',parseData)}, 1000);

window.onload = function () {
    parseData(test);
}



)==";