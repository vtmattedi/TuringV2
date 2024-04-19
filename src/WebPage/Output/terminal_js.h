const char *terminal_js = R"==(window.addEventListener('load', () => {
    const titleContainer = document.getElementById('titleContainer');
    titleContainer.style.opacity = '1';
});
const terminal = document.getElementById('terminal');
const terminalContainer = document.getElementById('terminalContainer');
const inputLine = document.getElementById('input');
const cursor = document.getElementById('cursor');
const keywords = ['boot', 'Not', 'not'];
var lastCmd = '';

let commandHistory = {
    currentIndex: 0,
    firstMove: false,
    current: '',
    commands: [],
    addCommand: function (command) {
        // Remove the command if it already exists in the array
        const index = this.commands.indexOf(command);
        if (index > -1) {
            this.commands.splice(index, 1);
        }
        // Add the command to the front of the array
        this.commands.unshift(command);
        // Limit the history 
        if (this.commands.length > 20) {
            this.commands.pop();
        }
        this.currentIndex = 0;
        this.firstMove = true;
        this.current = '';
    },
    getLastCommands: function () {
        if (this.commands[this.currentIndex])
            return this.commands[this.currentIndex];
        else
            return '';
    },

    resetIndex: function () {
        this.currentIndex = 0;
    },

    navigateIndex: function (change, currentValue) {
        if (typeof (change) !== 'number')
            return currentValue;

        if (this.commands.length === 0)
            return currentValue;

        if (this.firstMove) {
            if (change < 0)
                return currentValue;
            this.currentIndex = 0;
            this.firstMove = false;
            this.current = currentValue;
            return this.getLastCommands();
        }


        this.currentIndex += change;



        if (this.currentIndex == -1) {
            this.firstMove = true;
            this.currentIndex = 0;
            return this.current;
        }
        if (this.currentIndex >= this.commands.length)
            this.currentIndex = this.commands.length - 1;
        if (this.currentIndex < 0)
            this.currentIndex = 0;
        return this.getLastCommands();
    }


};

const scrollToBottom = (id) => {
    const element = document.getElementById(id);
    element.scrollTop = element.scrollHeight;
}
function applyHighLights(text) {
    text = highlightKeywords(text);
    text = highlightDatesTimes(text);
    text = highlightBracket(text);
    text = highlightBraces(text);
    text = highlightText(text);

    return text;
}
function highlightKeywords(text) {
    let highlightedText = text;
    keywords.forEach(keyword => {
        const regex = new RegExp(`\\b${keyword}\\b`, 'g');

        highlightedText = highlightedText.replace(regex, `<span class="highlight">${keyword}</span>`);

    });


    return highlightedText;
}

function highlightDatesTimes(text) {
    const regex = /\b(\d{1,2}\/\d{1,2}\/(\d{4}|\d{2})|\d{1,2}:\d{2}([ap]m)?)\b/gi;
    return text.replace(regex, '<span class="highlightDateTime">$1</span>');
}
function highlightBracket(text) {
    const regex = /\[(.*?)\]/g;
    return text.replace(regex, '[<span class="highlightBracket">$1</span>]');
}
function highlightText(text) {
    const regex = /\'(.*?)\'/g;
    return text.replace(regex, '<span class="highlightText">\'$1\'</span>');
}
function highlightBraces(text) {
    const regex = /\{(.*?)\}/g;
    return text.replace(regex, '{<span class="highlightBraces">$1</span>}');
}

function send() {
    var text = inputLine.value;
    if (text === '') {
        text = commandHistory.getLastCommands();
    }
    if (text.toLocaleLowerCase().includes('json'))
        requestData('/cmd?cmd=' + text, handleJson);
    else
        requestData('/cmd?cmd=' + text, handleResponse);
    const highlightedInput = applyHighLights(text);
    terminal.innerHTML += '<< ' + highlightedInput + '\n';
    inputLine.value = '';
    terminalContainer.scrollTop = terminalContainer.scrollHeight;
    commandHistory.addCommand(text);
}


window.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
        send();
    }
    else if (e.key === 'ArrowUp') {
        inputLine.value = commandHistory.navigateIndex(1, inputLine.value);
    }
    else if (e.key === 'ArrowDown') {
        inputLine.value = commandHistory.navigateIndex(-1, inputLine.value);
    }
});
function handleResponse(text) {
    const highlightedInput = applyHighLights(text);
    terminal.innerHTML += '>> ' + highlightedInput + '\n';
    terminalContainer.scrollTop = terminalContainer.scrollHeight;


}

var json;
function handleJson(json) {

    var response = JSON.parse(json);
    var text = '';

    for (var id in response) {
        text += id+ ":" + response[id] + '\n';

    }


    const highlightedInput = applyHighLights(text);
    terminal.innerHTML += '>> ' + highlightedInput + '\n';
    terminalContainer.scrollTop = terminalContainer.scrollHeight;



})==";