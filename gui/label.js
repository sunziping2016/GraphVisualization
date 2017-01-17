THREE.Label = class extends THREE.Mesh {
    constructor(text, options) {
        options = options || {};
        let labelCanvas = document.createElement('canvas'),
            xc = labelCanvas.getContext('2d'),
            fontSize = options.fontSize || '20pt',
            fontFamily = options.fontFamily || 'Arial',
            width, height = parseInt(fontSize) * 4/3;
        xc.font = fontSize + ' ' + fontFamily;
        width = xc.measureText(text).width;
        labelCanvas.setAttribute('width', width);
        labelCanvas.setAttribute('height', height);
        // set font size again cause it will be reset
        xc.font = fontSize + ' ' + fontFamily;
        xc.textBaseline = 'top';
        xc.fillText(text, 0, 0);
        super(
            new THREE.BoxGeometry(width, height, 0),
            new THREE.MeshBasicMaterial({
                map: new THREE.Texture(labelCanvas),
                transparent: true
            })
        );
        this.material.map.needsUpdate = true;
    }
};
