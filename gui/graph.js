const GRAPH = {
    Node: class {
        constructor(id, options) {
            options = options || {};
            this.id = id;
            this.nodesTo = [];
            this.nodesFrom = [];
            this.edges = {};
            this.position = options.position || [];
            this.color = options.color || 0x000099;
            this.data = options.data || {};
        }
    },
    Edge: class {
        constructor(source, target, options) {
            options = options || {};
            this.source = source;
            this.target = target;
            this.color = options.color || 0xff3333;
            this.data = options.data || {};
        }
    },
    Graph: class {
        constructor() {
            this.nodeSet = {};
            this.nodes = [];
            this.edges = [];
            this.data = {};
        }
        addNode(node, options) {
            if (!(node instanceof GRAPH.Node))
                node = new GRAPH.Node(node, options);
            if (this.nodeSet[node.id] == undefined) {
                this.nodeSet[node.id] = node;
                this.nodes.push(node);
                return node;
            }
            return null;
        }
        getNode(id) {
            return this.nodeSet[id];
        }
        addEdge(source, target, options) {
            if (!(source instanceof GRAPH.Node))
                source = this.getNode(source);
            if (!(target instanceof GRAPH.Node))
                target = this.getNode(target);
            if (!source.nodesTo.includes(target)) {
                source.nodesTo.push(target);
                target.nodesFrom.push(source);
                let edge = new GRAPH.Edge(source, target, options);
                edge.id = this.edges.length;
                this.edges.push(edge);
                source.edges[target.id] = edge;
                target.edges[source.id] = edge;
                return edge;
            }
            return null;
        }
    }
};