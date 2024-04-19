const char *common_css = R"==(
:root {
    --bgColor: aqua;
    --txColor: black;

}
@media (prefers-color-scheme: dark) {
    :root {
        /* Dark mode variables go here */
        --bgColor: black;
        --txColor: #ffffff;
    }
}


/* Dark mode styles */
body.dark-mode {
    background-color: #222;
    color: #ddd;
}

.dark-mode-toggle {
    position: absolute;
    top: 10px;
    right: 10px;
    z-index: 999;
}

.dark-mode-toggle label {
    color: #fff;
}

body {
    background-color: var(--bgColor);
    color:  var(--txColor);
}

.container {
    justify-content: center;
    display: flex;
    flex-direction: column;
    text-align: center;
    /* Center the text */
    min-width: 260px;
    max-width: 500px
}

.pump-container {
    justify-content: space-evenly;
    display: flex;
    flex-direction: row;
    text-align: center;
    /* Center the text */
}

.numinput {
    text-align: center;
    width: 100px;
    height: 30px;
    font-family: Arial, sans-serif;
    /* Set the font */
    border-radius: 40px;
    font-size: large;
}

label {
    margin: 10px;
    font-family: Arial, sans-serif;
    /* Set the font */
}

button {
    margin: 10px;
    width: 160px;
    height: 40px;
    font-family: Arial, sans-serif;
    /* Set the font */
    border-radius: 40px;
    font-size: large;
}

table {
    border-collapse: collapse;
    width: 100%;
    font-family: Arial, sans-serif;
    /* Set the font */
}

table th,
table td {
    border: 1px solid #a3a3a384;
    text-align: center;
    /* Center the text */
    padding: 8px;
    color: black;
}


table tr:nth-child(even) {
    background-color: #f9f9f9;
}

table tr:nth-child(odd) {
    background-color: #ffffff;
}

/* Style for Online status */
table .online {
    color: green;
    /* Light green */
}

/* Style for Offline status */
table .offline {
    color: darkred;
    /* Light red */
}

/* Style for Unknown status */
table .unknown {
    color: black;
    /* Light gray */
}


/* Style for changed cells */
table tr:nth-child(odd) td.changed {
    animation-name: changeBackground-odd;
    animation-duration: 1s;
    animation-fill-mode: forwards;
}

table tr:nth-child(even) td.changed {
    animation-name: changeBackground-even;
    animation-duration: 1s;
    animation-fill-mode: forwards;
}

@keyframes changeBackground-even {
    from {
        background-color: #ffff99;
        /* Initial color (light yellow) */
    }

    to {
        background-color: #f9f9f9;
        /* Final color (light red) */
    }
}

@keyframes changeBackground-odd {
    from {
        background-color: #ffff99;
        /* Initial color (light yellow) */
    }

    to {
        background-color: #ffffff;
        /* Final color (light red) */
    }
}


/* The switch - the box around the slider */
.switch {
    position: relative;
    display: inline-block;
    width: 60px;
    height: 34px;
}

/* Hide default HTML checkbox */
.switch input {
    opacity: 0;
    width: 0;
    height: 0;
}

/* The slider */
.slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s;
}

.slider:before {
    position: absolute;
    content: "";
    height: 26px;
    width: 26px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    -webkit-transition: .4s;
    transition: .4s;
}

input:checked+.slider {
    background-color: #2196F3;
}

input:focus+.slider {
    box-shadow: 0 0 1px #2196F3;
}

input:checked+.slider:before {
    -webkit-transform: translateX(26px);
    -ms-transform: translateX(26px);
    transform: translateX(26px);
}

/* Rounded sliders */
.slider.round {
    border-radius: 34px;
}

.slider.round:before {
    border-radius: 50%;
}

.toggles-container {
    display: flex;
    flex-direction: row;
    justify-content: space-evenly;
}

.switch-container {
    display: flex;
    flex-direction: row;
    align-items: center;
}

.sipo {
    cursor: pointer;
}

.sipo:hover {
    text-decoration: underline;
}

/* Base styles */
.container {
    width: 90%;
    margin: auto;
}

/* Styles for screens wider than 1000px */
@media (min-width: 1000px) {
    .container {
        width: 70%;
    }
}

/* Styles for screens smaller than 600px */
@media (max-width: 600px) {
    button {
        padding: 10px 20px; /* Larger padding for easier tapping */
        font-size: 16px; /* Larger font size for readability */
    }
    .switch-container label {
        font-size: 16px; /* Larger font size for readability */
    }
    .container {
        width: 80%;
    }
})==";