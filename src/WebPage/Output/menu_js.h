const char *menu_js = R"==(window.onload = function () {
    var time = new Date().getTime()/1000 - 3 * 60 * 60;
    requestData ('cmd?cmd=AUTOSYNCTIME '+ parseInt(time), (res) => {
        if (res.includes('AUTO'))
            alert(res);
    });
}

function requestData(http, reponseMethod)
{
    console.log(http);
    var xhr = new XMLHttpRequest();

    // Define the request
    xhr.open('GET', http, true);

    // Set up what to do when the request is complete
    xhr.onload = function () {
        if (xhr.status >= 200 && xhr.status < 300) {
            reponseMethod(xhr.responseText);
        } else {
            console.log('Request failed with status: ' + xhr.status);
        }
    };
xhr.send();
})==";