
Pebble.addEventListener('showConfiguration', function(e) {
    // Show config page
    Pebble.openURL('http://localhost:5000');
});

Pebble.addEventListener('webviewclosed', function(e) {
    var configData = JSON.parse(decodeURIComponent(e.response));
    if (configData.backgroundColor) {
        Pebble.sendAppMessage({
            backgroundColor: parseInt(configData.backgroundColor, 16)
        }, function() {
            window.console.log('ok!');
        }, function() {
            window.console.log('error');
        });
    }
});
