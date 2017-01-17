/**
 * Created by sun on 1/13/17.
 */
const path = require('path');
const ref = require('ref');
const ffi = require('ffi');
const libgraph = ffi.Library(path.join(__dirname, /^win/.test(process.platform) ? 'graph' : 'libgraph'), {
    'graph_ctor':                        [ 'pointer', [] ],
    'graph_dtor':                        [ 'void',    [ 'pointer' ] ],
    'graph_load':                        [ 'pointer', [ 'pointer', 'string' ] ],
    'graph_save':                        [ 'pointer', [ 'pointer', 'string' ] ],
    'graph_clear':                       [ 'pointer', [ 'pointer' ] ],
    'graph_import_edge_list':            [ 'pointer', [ 'pointer', 'string' ] ],
    'graph_get_vertex_num':              [ 'pointer', [ 'pointer' ] ],
    'graph_get_edge_list':               [ 'pointer', [ 'pointer' ] ],
    'graph_get_vertex_property':         [ 'pointer', [ 'pointer', 'size_t' ] ],
    'graph_get_edge_property':           [ 'pointer', [ 'pointer', 'size_t', 'size_t' ] ],
    'graph_query_shortest_path':         [ 'pointer', [ 'pointer', 'size_t', 'size_t' ] ],
    'graph_query_minimum_spanning_tree': [ 'pointer', [ 'pointer' ] ],
    'graph_query_betweeness_centrality': [ 'pointer', [ 'pointer' ] ],
    'graph_query_closeness_centrality':  [ 'pointer', [ 'pointer' ] ],
    'graph_query_connected_components':  [ 'pointer', [ 'pointer' ] ],
    'graph_layout_3dkk':                 [ 'pointer', [ 'pointer', 'bool', 'bool', 'double' ] ],
    'graph_layout_2dkk':                 [ 'pointer', [ 'pointer', 'bool', 'bool', 'double' ] ],
    'graph_layout_2dfr':                 [ 'pointer', [ 'pointer', 'bool', 'bool', 'size_t'] ],
    'str_get':                           [ 'string',  [ 'pointer' ] ],
    'str_free':                          [ 'void',    [ 'pointer' ] ]
});

function json_wrapper(callback) {
    return (err, res) => {
        if (err)
            callback(err, null);
        let result = JSON.parse(libgraph.str_get(res));
        libgraph.str_free(res);
        if (result.error)
            callback(new Error(result.error), null);
        else
            callback(null, result.result);
    };
}

module.exports.GraphProcessor = class {
    constructor() {
        this.ins = libgraph.graph_ctor();
    }
    destroy() {
        libgraph.graph_dtor(this.ins);
    }
    save(filename, callback) {
        libgraph.graph_save.async(this.ins, filename, json_wrapper(callback));
    }
    load(filename, callback) {
        libgraph.graph_load.async(this.ins, filename, json_wrapper(callback));
    }
    clear(callback) {
        libgraph.graph_clear.async(this.ins, json_wrapper(callback));
    }
    import_edge_list(filename, callback) {
        libgraph.graph_import_edge_list.async(this.ins, filename, json_wrapper(callback));
    }
    get_vertex_num(callback) {
        return libgraph.graph_get_vertex_num.async(this.ins, json_wrapper(callback));
    }
    get_edge_list(callback) {
        return libgraph.graph_get_edge_list.async(this.ins, json_wrapper(callback));
    }
    get_vertex_property(id, callback) {
        return libgraph.graph_get_vertex_property.async(this.ins, id, json_wrapper(callback));
    }
    get_edge_property(id1, id2, callback) {
        return libgraph.graph_get_edge_property.async(this.ins, id1, id2, json_wrapper(callback));
    }
    query_shortest_path(id1, id2, callback) {
        return libgraph.graph_query_shortest_path.async(this.ins, id1, id2, json_wrapper(callback));
    }
    query_minimum_spanning_tree(callback) {
        return libgraph.graph_query_minimum_spanning_tree.async(this.ins, json_wrapper(callback));
    }
    query_betweeness_centrality(callback) {
        return libgraph.graph_query_betweeness_centrality.async(this.ins, json_wrapper(callback));
    }
    query_closeness_centrality(callback) {
        return libgraph.graph_query_closeness_centrality.async(this.ins, json_wrapper(callback));
    }
    query_connected_components(callback) {
        return libgraph.graph_query_connected_components.async(this.ins, json_wrapper(callback));
    }
    layout_3dkk(reinitial, recalculate, tolerance, callback) {
        return libgraph.graph_layout_3dkk.async(this.ins, reinitial, recalculate, tolerance, json_wrapper(callback));
    }
    layout_2dkk(reinitial, recalculate, tolerance, callback) {
        return libgraph.graph_layout_2dkk.async(this.ins, reinitial, recalculate, tolerance, json_wrapper(callback));
    }
    layout_2dfr(reinitial, recalculate, iterations, callback) {
        return libgraph.graph_layout_2dfr.async(this.ins, reinitial, recalculate, iterations, json_wrapper(callback));
    }
};