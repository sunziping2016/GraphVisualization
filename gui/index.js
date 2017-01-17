const {app, BrowserWindow} = require('electron');
const path = require('path');
const url = require('url');
const libgraph = require('./libgraph');

global.shared = {
    processor: new libgraph.GraphProcessor(),
};

require('electron-debug')({enabled: true});

let win;

app.on('ready', () => {
    win = new BrowserWindow({width: 1000, height: 750, icon: path.join(__dirname, 'favicon.ico')});
    win.loadURL(url.format({
        pathname: path.join(__dirname, 'index.html'),
        protocol: 'file:',
        slashes: true
    }));
    win.on('closed', () => {
        win = null;
    });
});

app.on('window-all-closed', () => {
    shared.processor.destroy();
    app.quit();
});
