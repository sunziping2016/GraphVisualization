/**
 * Created by sun on 1/14/17.
 */
class GraphDrawer {
    constructor(dom_element) {
        this.element = $(dom_element);
        this.renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
        this.renderer.setSize(this.element.innerWidth(), this.element.innerHeight());
        this.camera = new THREE.PerspectiveCamera(40, this.element.innerWidth() / this.element.innerHeight(), 1, 100000);
        this.camera.position.z = 5000;
        $(window).resize(() => {
            this.camera.aspect = this.element.innerWidth() / this.element.innerHeight();
            this.camera.updateProjectionMatrix();
            this.renderer.setSize(this.element.innerWidth(), this.element.innerHeight());
        });
        this.scene = new THREE.Scene();
        this.element.append(this.renderer.domElement);
        this.controls = new THREE.TrackballControls(this.camera, dom_element, { enabled: false });
        this.stats = new THREE.Stats();
        this.stats.domElement.style.position = 'absolute';
        this.stats.domElement.style.top = null;
        this.stats.domElement.style.bottom = '0px';
        this.element.append(this.stats.domElement);
    }
    set graph(g) {
        if (g === this.__graph)
            return;
        if (this.__graph && this.__graph.data.has_draw_objects) {
            for (let node of this.__graph.nodes)
                this.scene.remove(node.data.draw_object);
            for (let edge of this.__graph.edges)
                this.scene.remove(edge.data.draw_object);
        }
        if (this.__graph && this.show_labels) {
            for (let node of this.__graph.nodes)
                this.scene.remove(node.data.draw_label);
        }
        this.__graph = g;
        if (g) {
            if (!g.data.has_draw_objects) {
                this.generate_objects(g);
                g.data.has_draw_objects = true;
            }
            if (!g.data.has_draw_labels) {
                this.generate_labels(g);
                g.data.has_draw_labels = true;
            }
            for (let node of g.nodes)
                this.scene.add(node.data.draw_object);
            for (let edge of g.edges)
                this.scene.add(edge.data.draw_object);
            if (this.show_labels) {
                for (let node of g.nodes)
                    this.scene.add(node.data.draw_label);
            }
            this.controls.enabled = true;
        } else
            this.controls.enabled = false;
    }
    get graph() {
        return this.__graph;
    }
    set show_labels(e) {
        if (e == this.__show_labels)
            return;
        this.__show_labels = e;
        if (this.graph) {
            if (e) {
                for (let node of this.graph.nodes)
                    this.scene.add(node.data.draw_label);
            } else {
                for (let node of this.__graph.nodes)
                    this.scene.remove(node.data.draw_label);
            }
        }
    }
    get show_labels() {
        return this.__show_labels;
    }
    animate() {
        requestAnimationFrame(()=>this.animate());
        this.controls.update();
        this.stats.update();
        if (this.graph && this.show_labels && !this.controls.noRotate)
            this.update_labels();
        this.render();
    }
    render() {
        this.renderer.render(this.scene, this.camera);
    }
    reset_camera() {
        this.controls.reset(new THREE.Vector3(0, 0, 5000));
    }
    generate_objects(g) {
        const area = 1000;
        for (let node of g.nodes) {
            const draw_object= new THREE.Mesh(new THREE.SphereGeometry(5, 10, 10), new THREE.MeshBasicMaterial({
                color: node.color,
                opacity: 0.9
            }));
            draw_object.position.set(node.position[0] * area, node.position[1] * area, node.position[2] * area);
            node.watch('color', (id, oldvalue, newvalue) => {
                draw_object.material.color = new THREE.Color(newvalue);
                return newvalue;
            });
            node.watch('position', (id, oldvalue, newvalue) => {
                draw_object.position.set(newvalue[0] * area, newvalue[1] * area, newvalue[2] * area);
                for (let index in node.edges)
                    node.edges[index].data.draw_object.geometry.verticesNeedUpdate = true;
                if(node.data.draw_label)
                    node.data.draw_label.position.copy(
                        draw_object.position.clone().add(
                            // Add a delta to let the label float over the node
                            new THREE.Vector3(0, 20, 0).applyQuaternion(
                                this.camera.quaternion)));
                return newvalue;
            });
            node.data.draw_object = draw_object;
        }
        for (let edge of g.edges) {
            const geometry = new THREE.Geometry();
            geometry.vertices.push(edge.source.data.draw_object.position, edge.target.data.draw_object.position);
            const draw_object = new THREE.Line(geometry, new THREE.LineBasicMaterial({
                color: edge.color,
                opacity: 0.9,
                linewidth: 0.1
            }));
            edge.watch('color', (id, oldvalue, newvalue) => {
                draw_object.material.color = new THREE.Color(newvalue);
                return newvalue;
            });
            edge.data.draw_object = draw_object;
        }
    }
    generate_labels(g) {
        for (let node of g.nodes) {
            node.data.draw_label = new THREE.Label(node.data.title || node.id);
        }
        this.update_labels();
    }
    update_labels() {
        for (let node of this.graph.nodes) {
            node.data.draw_label.position.copy(
                node.data.draw_object.position.clone().add(
                    // Add a delta to let the label float over the node
                    new THREE.Vector3(0, 20, 0).applyQuaternion(
                        this.camera.quaternion)));
            // Rotation
            node.data.draw_label.quaternion.copy(this.camera.quaternion);
        }
    }
}