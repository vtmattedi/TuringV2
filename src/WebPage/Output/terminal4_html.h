const char *terminal4_html = R"==(<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Terminal</title>
    <style>
        #responseContainer {
            background-color: #000;
            color: #fff;
            padding: 10px;
            margin-top: 10px;
            font-family: monospace;
            white-space: pre-wrap;
            overflow-wrap: break-word;
        }
    </style>
</head>

<body>
    <h1>ESP32 Terminal</h1>
    <form id="terminalForm">
        <label for="commandInput">Enter Command:</label>
        <input type="text" id="commandInput" name="commandInput">
        <button type="submit">Submit</button>
    </form>
    <div id="responseContainer"></div>

    <script>
        document.getElementById("terminalForm").addEventListener("submit", function (event) {
            event.preventDefault();

            // Get the command input value
            var command = document.getElementById("commandInput").value.trim();

            if (command !== "") {
                // Send the command to the server via HTTP GET request
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/cmd?cmd=" + encodeURIComponent(command), true);
                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        // Display the server response
                        var responseContainer = document.getElementById("responseContainer");
                        responseContainer.innerHTML += '<div>$ ' + command + '</div>';
                        responseContainer.innerHTML += '<div>' + xhr.responseText + '</div>';
                    }
                };
                xhr.send();
            } else {
                // If command input is empty, display an error message
                alert("Please enter a command.");
            }
        });
    </script>
</body>

</html>)==";