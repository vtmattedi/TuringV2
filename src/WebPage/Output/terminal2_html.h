const char *terminal2_html = R"==(<!DOCTYPE html>
<html>
<head>
    <style>
        body {
            background-color: #000;
            color: #0F0;
            font-family: monospace;
            padding: 10px;
        }
        #terminalContainer {
            border: 2px solid #0F0;
            padding: 10px;
            width: 80%;
            margin: auto;
        }
        #terminal {
            white-space: pre-wrap;
            line-height: 1.5;
            font-size: 14px;
        }
        #inputLine {
            display: flex;
        }
        #cursor {
            animation: blink 1s infinite;
        }
        @keyframes blink {
            50% { visibility: hidden; }
        }
    </style>
</head>
<body>
    <div id="terminalContainer">
        <div id="terminal"></div>
        <div id="inputLine">
            <span>></span>
            <span id="input"></span>
            <span id="cursor">|</span>
        </div>
    </div>
    <script>
        const terminal = document.getElementById('terminal');
        const inputLine = document.getElementById('input');
        const cursor = document.getElementById('cursor');
        var blink_cursor_var = false;
        function blinkCursor() {
            cursor.hidden = blink_cursor_var;
            blink_cursor_var = !blink_cursor_var;
        }
        setInterval(blinkCursor,500)
        window.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                terminal.textContent += '\n< ' + inputLine.textContent;
                inputLine.textContent = '';
            } else if (e.key === 'Backspace') {
                inputLine.textContent = inputLine.textContent.slice(0, -1);
            } else if (e.key.length === 1) {
                inputLine.textContent += e.key;
            }
        });
    </script>
</body>
</html>
)==";