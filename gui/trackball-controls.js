const STATE = { NONE: - 1, ROTATE: 0, ZOOM: 1, PAN: 2, TOUCH_ROTATE: 3, TOUCH_ZOOM_PAN: 4 };
const EPSILON = 1e-3;

THREE.TrackballControls = class extends THREE.EventDispatcher {
    constructor(object, domElement, options) {
        super();

        this.object = object;
        this.domElement = domElement || document;

        options = options || {};

        this._enabled = false;
        this.screen = { left: 0, top: 0, width: 0, height: 0 };

        this.rotateSpeed = options.rotateSpeed || 1.5;
        this.zoomSpeed = options.zoomSpeed || 0.25;
        this.panSpeed = options.panSpeed || 0.4;

        this.noRotate = options.noRotate || false;
        this.noZoom = options.noZoom || false;
        this.noPan = options.noPan || false;

        this.dynamicDampingFactor = options.dynamicDampingFactor || 0.2;

        this.rotateDampingFactor = options.rotateDampingFactor || 0.9;
        this.zoomDampingFactor = options.zoomDampingFactor || 0.6;
        this.panDampingFactor = options.panDampingFactor || 0.9;

        // Target position, relative position and up direction together
        // make up the state of a camera, among which target position is not
        // stored in camera.
        this.target = options.target || new THREE.Vector3();
        this.state = STATE.NONE;

        // Some temporary state is saved for smooth animation.
        this._reset = null;
        this.mouseStart = new THREE.Vector2();
        this.rotateGoal = new THREE.Vector2();
        this.zoomGoal = 0;
        this.panGoal = new THREE.Vector2();

        this.rotateAxis = null;
        this.rotateAngle = 0;
        this.panDirection = new THREE.Vector3();
        this.zoomFactor = 0;

        // Event handlers
        let self = this;
        this.handleResize = function () {
            if (self.domElement === document) {
                self.screen.left = 0;
                self.screen.top = 0;
                self.screen.width = window.innerWidth;
                self.screen.height = window.innerHeight;
            } else {
                let box = self.domElement.getBoundingClientRect();
                // Adjustments come from similar code in the jquery offset() function
                let d = self.domElement.ownerDocument.documentElement;
                self.screen.left = box.left + window.pageXOffset - d.clientLeft;
                self.screen.top = box.top + window.pageYOffset - d.clientTop;
                self.screen.width = box.width;
                self.screen.height = box.height;
            }
        };

        this.mousedown = function (event) {
            if (self.enabled === false)
                return;
            event.preventDefault();
            event.stopPropagation();
            if (self.state === STATE.NONE)
                self.state = event.button;
            if ((self.state === STATE.ROTATE || self.state === STATE.PAN) &&
                (!self.noRotate || !self.noPan))
                self.mouseStart.copy(self.getMouseOnCircle(event.pageX, event.pageY));
            document.addEventListener('mousemove', self.mousemove, false);
            document.addEventListener('mouseup', self.mouseup, false);
        };

        this.mousemove = function (event) {
            if (self.enabled === false)
                return;
            event.preventDefault();
            event.stopPropagation();
            let newMouseStart = self.getMouseOnCircle(event.pageX, event.pageY),
                deltaMouse = newMouseStart.clone().sub(self.mouseStart);
            if (self.state === STATE.ROTATE && !self.noRotate)
                self.rotateGoal.add(deltaMouse);
            else if (self.state === STATE.PAN && !self.noPan)
                self.panGoal.add(deltaMouse);
            self.mouseStart.copy(newMouseStart);
        };

        this.mouseup = function (event) {
            if (self.enabled === false)
                return;
            event.preventDefault();
            event.stopPropagation();
            self.state = STATE.NONE;
            document.removeEventListener('mousemove', self.mousemove);
            document.removeEventListener('mouseup', self.mouseup);
        };

        this.mousewheel = function (event) {
            if (self.enabled === false)
                return;
            event.preventDefault();
            event.stopPropagation();
            self.zoomGoal += event.deltaY * 0.01;
        };

        this.contextmenu = function (event) {
            event.preventDefault();
        };

        this.enabled = options.enabled || true;
    }

    set enabled(value) {
        if (this._enabled !== value) {
            this._enabled = value;
            if (value) {
                /*
                 this.domElement.addEventListener('touchstart', this.touchstart, false);
                 this.domElement.addEventListener('touchend', this.touchend, false);
                 this.domElement.addEventListener('touchmove', this.touchmove, false);*/
                this.domElement.addEventListener('mousedown', this.mousedown, false);
                this.domElement.addEventListener('contextmenu', this.contextmenu, false);
                this.domElement.addEventListener('wheel', this.mousewheel, false);
                window.addEventListener('resize', this.handleResize);

                this.handleResize();
            } else {
                /*
                 this.domElement.removeEventListener('mousedown', this.mousedown, false);
                 this.domElement.removeEventListener('wheel', this.mousewheel, false);
                 this.domElement.removeEventListener('touchstart', this.touchstart, false);
                 this.domElement.removeEventListener('touchend', this.touchend, false);
                 this.domElement.removeEventListener('touchmove', this.touchmove, false);
                 */
                this.domElement.removeEventListener('mousedown', this.mousedown, false);
                this.domElement.removeEventListener('contextmenu', this.contextmenu, false);
                this.domElement.removeEventListener('wheel', this.mousewheel, false);
                // Possibly added by mousedown
                document.removeEventListener('mousemove', this.mousemove);
                document.removeEventListener('mouseup', this.mouseup);
                window.removeEventListener('resize', this.handleResize);
            }
        }
    }

    get enabled() {
        return this._enabled;
    }

    getMouseOnCircle(x, y) {
        return new THREE.Vector2(
            ((x - this.screen.width * 0.5 - this.screen.left) / (this.screen.width * 0.5)),
            ((this.screen.height + 2 * (this.screen.top - y)) / this.screen.width)
        );
    }

    update() {
        let state = {
            target: this.target.clone(),
            eye: this.object.position.clone().sub(this.target),
            up: this.object.up.clone(),
            changed: false
        };
        if (this._reset) {
            this.object.position.copy(this._reset);
            this.object.up = new THREE.Vector3(0, 1, 0);
            this.object.lookAt(new THREE.Vector3(0, 0, 0));
            this._reset = null;
        } else {
            if (!this.noRotate)
                this.rotateCamera(state);
            if (!this.noZoom)
                this.zoomCamera(state);
            if (!this.noPan)
                this.panCamera(state);
            if (state.changed) {
                this.target = state.target;
                this.object.position.addVectors(state.target, state.eye);
                this.object.up.copy(state.up);
                this.object.lookAt(state.target);
            }
        }
    }

    rotateCamera(state) {
        let angle = this.rotateGoal.length() * this.rotateSpeed;
        if (angle > EPSILON) {
            let axis = state.up.clone().setLength(this.rotateGoal.y).add(
                state.up.clone().cross(state.eye).setLength(this.rotateGoal.x)).cross(
                state.eye
                ).normalize(),
                quaternion = new THREE.Quaternion().setFromAxisAngle(axis, angle);
            state.eye.applyQuaternion(quaternion);
            state.up.applyQuaternion(quaternion);
            state.changed = true;
            this.rotateGoal.set(0, 0);
            this.rotateAngle = angle;
            this.rotateAxis = axis;
        } else if (this.rotateAngle > EPSILON) {
            this.rotateAngle *= this.rotateDampingFactor;
            let quaternion = new THREE.Quaternion().setFromAxisAngle(this.rotateAxis, this.rotateAngle);
            state.eye.applyQuaternion(quaternion);
            state.up.applyQuaternion(quaternion);
            state.changed = true;
        }
    }

    zoomCamera(state) {
        let factor = 1.0 + this.zoomGoal * this.zoomSpeed;
        if (Math.abs(factor - 1.0) > EPSILON && factor > EPSILON) {
            state.eye.multiplyScalar(factor);
            state.changed = true;
            this.zoomFactor = this.zoomGoal * this.zoomSpeed;
            this.zoomGoal = 0;
        } else if (Math.abs(this.zoomFactor) > EPSILON) {
            this.zoomFactor *= this.zoomDampingFactor;
            state.eye.multiplyScalar(1.0 + this.zoomFactor);
            state.changed = true;
        }
    }

    panCamera(state) {
        let pan = this.panGoal.clone().multiplyScalar(state.eye.length() * this.panSpeed);
        if (pan.length() > EPSILON) {
            this.panDirection = state.eye.clone().cross(state.up).setLength(pan.x).add(
                state.up.clone().setLength(-pan.y)
            );
            state.target.add(this.panDirection);
            state.changed = true;
            this.panGoal.set(0, 0);
        } else if (this.panDirection.length() > EPSILON){
            this.panDirection.multiplyScalar(this.panDampingFactor);
            state.target.add(this.panDirection);
            state.changed = true;
        }
    }

    reset(position) {
        this.target = new THREE.Vector3();
        this._reset = position;
    }
};

THREE.TrackballControls.Event = class {
    constructor(type) {
        this.type = type;
    }
};
