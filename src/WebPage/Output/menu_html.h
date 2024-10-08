const char *menu_html = R"==(<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Turing Mux/Shifter</title>
    <link rel="stylesheet" type="text/css" href="menu.css">
    <script src="common.js" defer></script>
    <script src="menu.js" defer></script>
</head>


<body class='invert'>
    <div class='wrap'>
        <h1>Turing</h1>
        <h3>Turing Gardner</h3>
        <form action='/sensors' method='get'>
            <button>Sensors</button>
        </form>
        <br/>
        <form action='/mux' method='get'>
            <button>Mux</button>
        </form>
        <br/>
        <form action='/info' method='get' >
            <button>Info</button>
        </form>
        <br/>
        <form action='/terminal' method='get'>
            <button>Terminal</button>
        </form>
        <br/>
        <hr>
        <br/>
        <form action='/update' method='get'>
            <button>Update</button>
        </form>
        <br/>
        <div class='msg S'>
            <strong>Connected</strong>
            to {SSID}<br/>
            <em>
                <small>with IP: {IP}</small>
            </em>
        </div>
        <div class='msg S'>
            <strong>Version: </strong>
            0.6.34
<br/>
            <em>
                <small>Build Time: 2024-10-04 18:17:10.951022</small>
            </em>
        </div>
    </div>
</body>
</html>)==";