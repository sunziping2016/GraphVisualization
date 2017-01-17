const $ = require('jquery');
const electron = require('electron');
const {shell, remote} = electron;
const {dialog} = electron.remote;
const {processor} = remote.getGlobal('shared');
const noty = (options) => {require('noty')({
    layout: 'topRight',
    theme: 'metroui',
    type: options.type || 'alert',
    animation: {
        open: {height: 'toggle'},
        close: {height: 'toggle'},
        easing: 'swing',
        speed: 200 // opening & closing animation speed
    },
    //theme: 'metroui',
    text: options.text,
    timeout: options.timeout || 1500
})};
let drawer, graph = null, busy = false;
$(() => {
    drawer = new GraphDrawer(document.getElementById('graph-canvas'));
    drawer.animate();
    layout.change();
});

function check_busy() {
    if (busy)
        noty({ type: 'error', text: "请等待其他操作完成" });
    return busy;
}

function check_graph() {
    if (!graph) {
        noty({ type: 'error', text: "图不能为空" });
        return true;
    }
    return false;
}

/* External hyperlink */
$('a.extern').click(function () {
    event.preventDefault();
    shell.openExternal($(this).attr('href'));
});

/* About dialog */
const about_dialog = $('#dialog-about');
$('#menu-about').click(e => {
    e.preventDefault();
    about_dialog.get(0).showModal();
});
$('#menu-about-close').click(() => {
    about_dialog.get(0).close();
});

/* Toggle drawer */
const menu_drawer = document.querySelector('.mdl-layout');
function toggle_drawer() {
    menu_drawer.MaterialLayout.toggleDrawer();
}

/* Toggle panel */
const info_panel = $('#info-panel');
info_panel.find('.mdl-card').each(function (index) {
    $(this).css({'z-index': 10 - index});
});
info_panel.find('.mdl-card__title-text').css({'cursor': 'pointer'}).click(function () {
    $(this).parent().find('.collapse-content').slideToggle(200);
});

const num_vertices = $('#graph-vertices-num'),
    num_edges = $('#graph-edges-num'),
    num_components = $('#graph-components-num');

function clear_information() {
    num_vertices.val('').parent().removeClass('is-dirty');
    num_edges.val('').parent().removeClass('is-dirty');
    num_components.val('').parent().removeClass('is-dirty');
}

/* Import and open menu */
$('#menu-import').click(e => {
    e.preventDefault();
    toggle_drawer();
    import_file();
});
$('#menu-open').click(e => {
    e.preventDefault();
    toggle_drawer();
    open_file();
});
function import_file() {
    if (check_busy())
        return;
    const files = dialog.showOpenDialog({
        title: '导入文件 ...',
        properties: ['openFile'],
        filters: [
            {name: '边列表', extensions: ['txt']},
            {name: '图的数据', extensions: ['graph']},
            {name: '所有文件', extensions: ['*']}
        ]
    });
    if (files && files.length)
        load_file(files[0]);
}
function open_file() {
    if (check_busy())
        return;
    const files = dialog.showOpenDialog({
        title: '打开文件 ...',
        properties: ['openFile'],
        filters: [
            {name: '图的数据', extensions: ['graph']},
            {name: '边列表', extensions: ['txt']},
            {name: '所有文件', extensions: ['*']}
        ]
    });
    if (files && files.length)
        load_file(files[0]);
}
function load_file(filename) {
    if (check_busy())
        return;
    let method = 'import_edge_list';
    if (filename.endsWith('.graph'))
        method = 'load';
    NProgress.start();
    busy = true;
    processor[method](filename, (err, res) => {
        if (err) {
            noty({ type: 'error', text: err.message });
            NProgress.done();
            busy = false;
            return;
        }
        processor.get_vertex_num((err, res) => {
            if (err) {
                noty({type: 'error', text: err.message});
                NProgress.done();
                busy = false;
                return;
            }
            let g = new GRAPH.Graph();
            for (let i = 0; i < res; ++i)
                g.addNode(i).data.visible = {};
            processor.get_edge_list((err, res) => {
                if (err) {
                    noty({type: 'error', text: err.message});
                    NProgress.done();
                    busy = false;
                    return;
                }
                for (let edge of res) {
                    let e = g.addEdge(edge[0], edge[1]);
                    e.data.weight = edge[2];
                    e.data.visible = {}
                }
                processor.query_connected_components((err, res) => {
                    if (err) {
                        noty({type: 'error', text: err.message});
                        NProgress.done();
                        busy = false;
                        return;
                    }
                    graph = g;
                    graph.data.components = res;
                    graph.data.filename = filename;
                    update_components(res);
                    update_layout_options();
                    clear_algorithm();
                    num_vertices.val(g.nodes.length).parent().addClass('is-dirty');
                    num_edges.val(g.edges.length).parent().addClass('is-dirty');
                    num_components.val(res.length).parent().addClass('is-dirty');
                    drawer.reset_camera();
                    NProgress.done();
                    busy = false;
                    layout.val(layout_options_list[1]).change();
                });
            });
        });
    });
}
function save_file() {
    if (check_busy() || check_graph())
        return;
    let filename = graph.data.save_filename;
    if (!filename) {
        let default_filename = graph.data.filename;
        if (default_filename.endsWith('.graph'))
            filename = default_filename;
        else {
            default_filename = default_filename.slice(0, default_filename.lastIndexOf('.')) + '.graph';
            const file = dialog.showSaveDialog({
                title: '保存文件 ...',
                defaultPath: default_filename,
                filters: [
                    {name: '图的数据', extensions: ['graph']}
                ]
            });
            if (file)
                filename = file;
            else
                return;
        }
    }
    NProgress.start();
    busy = true;
    console.log(filename);
    processor.save(filename, (err, res) => {
        if (err) {
            noty({type: 'error', text: err.message});
            NProgress.done();
            busy = false;
            return;
        }
        graph.data.save_filename = filename;
        noty({type: 'success', text: '成功保存文件'});
        NProgress.done();
        busy = false;
    });
}
function close_file() {
    if (check_busy())
        return;
    NProgress.start();
    busy = true;
    processor.clear((err, res) => {
        if (err) {
            noty({type: 'error', text: err.message});
            NProgress.done();
            busy = false;
            return;
        }
        graph = null;
        drawer.graph = null;
        update_components([]);
        update_layout_options();
        clear_algorithm();
        clear_information();
        NProgress.done();
        busy = false;
        layout.val(layout_options_list[0]).change();
    });
}

/* Save menu, close menu and exit menu */
$('#menu-save').click(() => {
    save_file();
});
$('#menu-close').click(() => {
    close_file();
});
$('#menu-exit').click(() => {
    remote.getCurrentWindow().close();
});
$(document).keydown((evt) => {
    if (evt.ctrlKey) {
        if (evt.key == 'i')
            import_file();
        else if (evt.key == 'o')
            open_file();
        else if (evt.key == 's')
            save_file();
        else if (evt.key == 'x')
            close_file();
        else if (evt.key == 'q' )
            remote.getCurrentWindow().close();
    }
});


const layout = $('#settings-layout'),
    layout_options = $('#settings-layout-options'),
    layout_tolerance = $('#settings-layout_tolerance'),
    layout_tolerance_div = $('#settings-layout_tolerance-div'),
    layout_iterations = $('#settings-layout_iterations'),
    layout_iterations_div = $('#settings-layout_iterations-div'),
    relayout = $('#settings-relayout'),
    relayout_tooltip = $('#settings-relayout-tooltip'),
    iterate_layout = $('#settings-iterate_layout'),
    iterate_layout_tooltip = $('#settings-iterate_layout-tooltip'),
    node_color = $('#settings-node_color'),
    node_color_div = $('#settings-node_color-div'),
    node_color_options = $('#settings-node_color-options'),
    node_color_list = ['（无）', '连通分量', '紧密中心度', '介数中心度'],
    label = $('#settings-label'),
    label_div = $('#settings-label-div'),
    rotate = $('#settings-rotate'),
    rotate_div = $('#settings-rotate-div'),
    reset_viewport = $('#settings-reset_viewport'),
    layout_items = [relayout, iterate_layout, node_color_div, label_div, rotate_div, reset_viewport],
    layout_options_list = ['（无）', '3D Kamada Kawai', '2D Kamada Kawai', '2D Fruchterman Reingold'];
/* Layout options */
for (let option of layout_options_list)
    layout_options.append(`<li class="mdl-menu__item" id="layout-option-${option.replace(/\W/g, '_')}">${option}</li>`);
function update_layout_options() {
    if (graph) {
        $(`#layout-option-${layout_options_list[0].replace(/\W/g, '_')}`).hide();
        for (let option of layout_options_list.slice(1))
            $(`#layout-option-${option.replace(/\W/g, '_')}`).show();
    } else {
        $(`#layout-option-${layout_options_list[0].replace(/\W/g, '_')}`).show();
        for (let option of layout_options_list.slice(1))
            $(`#layout-option-${option.replace(/\W/g, '_')}`).hide();
    }
}
update_layout_options();
layout.change(() => {
    const choice = layout_options_list.indexOf(layout.val()) || -1;
    if (choice == 1 || choice == 2) {
        layout.parent().addClass('is-dirty');
        layout_tolerance_div.show();
        layout_iterations_div.hide();
        layout_items.forEach(i => i.show());
    } else if (choice == 3) {
        layout.parent().addClass('is-dirty');
        layout_tolerance_div.hide();
        layout_iterations_div.show();
        layout_items.forEach(i => i.show());
    } else {
        layout.val('').parent().removeClass('is-dirty');
        layout_tolerance_div.hide();
        layout_iterations_div.hide();
        layout_items.forEach(i => i.hide());
    }
    if ((choice == 2 || choice == 3) != rotate.prop('checked'))
        rotate.click();
    drawer.reset_camera();
    if (choice == 1 || choice == 2 || choice == 3)
        calculate_layout(false, false);
});

/* Relayout button */
function calculate_layout(reinitial, recalculate) {
    function load_layout(err, res) {
        if (err) {
            noty({type: 'error', text: err.message});
            return;
        }
        for (let i = 0; i < res.length; ++i)
            graph.nodes[i].position = res[i];
        if (!graph.data.has_layout) {
            drawer.graph = graph;
            graph.data.has_layout = true;
        }
    }
    if (check_busy())
        return;
    if (recalculate === undefined)
        recalculate = true;
    const choice = layout_options_list.indexOf(layout.val()) || -1;
    if (choice == 1 || choice == 2) {
        if (!layout_tolerance.val().match(/^[0-9]+(\.[0-9]+)?(e-?[0-9]+)?$/)) {
            noty({ type: 'error', text: "能量容差必须是有限实数" });
            return;
        }
        let tolerance = parseFloat(layout_tolerance.val());
        relayout.prop("disabled", true);
        iterate_layout.prop("disabled", true);
        relayout_tooltip.removeClass('is-active');
        iterate_layout_tooltip.removeClass('is-active');
        NProgress.start();
        busy = true;
        processor[choice == 1 ? 'layout_3dkk' : 'layout_2dkk'](reinitial, recalculate, tolerance, (err, res) => {
            load_layout(err, res);
            relayout.prop("disabled", false);
            iterate_layout.prop("disabled", false);
            NProgress.done();
            busy = false;
        });
    } else if (choice == 3) {
        if (!layout_iterations.val().match(/^[0-9]+$/)) {
            noty({ type: 'error', text: "迭代次数必须是整数" });
            return;
        }
        let iterations = parseInt(layout_iterations.val());
        relayout.prop("disabled", true);
        iterate_layout.prop("disabled", true);
        relayout_tooltip.removeClass('is-active');
        iterate_layout_tooltip.removeClass('is-active');
        NProgress.start();
        busy = true;
        processor.layout_2dfr(reinitial, recalculate, iterations, (err, res) => {
            load_layout(err, res);
            relayout.prop("disabled", false);
            iterate_layout.prop("disabled", false);
            NProgress.done();
            busy = false;
        });
    }
}
relayout.click(() => {
    calculate_layout(true);
    drawer.reset_camera();
});
iterate_layout.click(() => {
    calculate_layout(false);
    drawer.reset_camera();
});

/* Show label checkbox */
label.change(() => {
    drawer.show_labels = label.prop('checked');
});

/* Ban rotation checkbox */
rotate.change(() => {
    drawer.controls.noRotate = rotate.prop('checked');
});

/* Reset viewport button */
reset_viewport.click(() => {
    drawer.reset_camera();
});

/* Update components */
const components_table_body = $('#components-table-body'),
    components_table_header = $('#components-table-header');
function update_components(components) {
    components_table_body.empty();
    for (let index = 0; index < components.length; ++index) {
        components_table_body.append(
            `<tr><td>
                <label class="mdl-checkbox mdl-js-checkbox mdl-data-table__select" for="components-table-row${index}">
                    <input type="checkbox" id="components-table-row${index}" class="mdl-checkbox__input" checked>
                </label>
            </td><td>${index + 1}</td><td>${components[index].length}</td></tr>`);
        let objects = [], edges = {};
        for (let i of graph.data.components[index]) {
            let node = graph.nodes[i];
            objects.push(node);
            for (let index in node.edges) {
                let edge = node.edges[index];
                edges[edge.id] = edge;
            }
        }
        for (let edge in edges)
            objects.push(edges[edge]);
        $(`#components-table-row${index}`).change(function () {
            for (let i of objects) {
                i.data.component = $(this).prop('checked');
                check_visible(i);
            }
        });
    }
}
components_table_header.change(() => {
    components_table_body.find(`input[type="checkbox"]${components_table_header.prop('checked')
        ? ':not(:checked)' : ':checked'}`).click();
});

/* Color options */
for (let option of node_color_list)
    node_color_options.append(`<li class="mdl-menu__item">${option}</li>`);
node_color.change(() => {
    const choice = node_color_list.indexOf(node_color.val());
    if (choice == 1) {
        node_color.parent().addClass('is-dirty');
        for (let i = 0; i < graph.data.components.length; ++i) {
            const color = `hsl(${i / graph.data.components.length * 240}, 100%, 30%)`;
            for (let j of graph.data.components[i])
                graph.nodes[j].color = color;
        }
    } else if (choice == 2) {
        node_color.parent().addClass('is-dirty');
        if (!graph.data.betweeness_centrality) {
            if (check_busy())
                return;
            NProgress.start();
            busy = true;
            processor.query_betweeness_centrality((err, res) => {
                if (err) {
                    noty({type: 'error', text: err.message});
                    NProgress.done();
                    busy = false;
                    return;
                }
                graph.data.betweeness_centrality = res;
                graph.data.betweeness_centrality_color = [];
                graph.data.betweeness_centrality_max = Math.max(...res);
                graph.data.betweeness_centrality_min = Math.min(...res);
                let delta = graph.data.betweeness_centrality_max - graph.data.betweeness_centrality_min || 1.0;
                for (let i of graph.data.betweeness_centrality)
                    graph.data.betweeness_centrality_color.push(new THREE.Color(
                        `hsl(${(graph.data.betweeness_centrality_max - i) / delta * 240}, 100%, 30%)`).getHex());
                for (let i = 0; i < graph.data.betweeness_centrality_color.length; ++i)
                    graph.nodes[i].color = graph.data.betweeness_centrality_color[i];
                NProgress.done();
                busy = false;
            });
        } else
            for (let i = 0; i < graph.data.betweeness_centrality_color.length; ++i)
                graph.nodes[i].color = graph.data.betweeness_centrality_color[i];
    } else if (choice == 3) {
        node_color.parent().addClass('is-dirty');
        if (!graph.data.closeness_centrality) {
            if (check_busy())
                return;
            NProgress.start();
            busy = true;
            processor.query_closeness_centrality((err, res) => {
                if (err) {
                    noty({type: 'error', text: err.message});
                    NProgress.done();
                    busy = false;
                    return;
                }
                graph.data.closeness_centrality = res;
                graph.data.closeness_centrality_color = [];
                graph.data.closeness_centrality_max = Math.max(...res);
                graph.data.closeness_centrality_min = Math.min(...res);
                let delta = graph.data.closeness_centrality_max - graph.data.closeness_centrality_min || 1.0;
                for (let i of graph.data.closeness_centrality)
                    graph.data.closeness_centrality_color.push(new THREE.Color(
                        `hsl(${(i - graph.data.closeness_centrality_min) / delta * 240}, 100%, 30%)`).getHex());
                for (let i = 0; i < graph.data.closeness_centrality_color.length; ++i)
                    graph.nodes[i].color = graph.data.closeness_centrality_color[i];
                NProgress.done();
                busy = false;
            });
        } else
            for (let i = 0; i < graph.data.closeness_centrality_color.length; ++i)
                graph.nodes[i].color = graph.data.closeness_centrality_color[i];
    } else {
        node_color.val('').parent().removeClass('is-dirty');
        for (let node of graph.nodes)
            node.color = 0x000099;
    }
});

let highlight_array = [];

function is_visible(object) {
    if (object.data.component !== undefined && !object.data.component)
        return false;
    if (highlight_array.length == 0)
        return true;
    for (let i of highlight_array)
        if (object.data.visible[i])
            return true;
    return false;
}

function check_visible(object) {
    if (is_visible(object)) {
        object.data.draw_object.visible = true;
        if (object.data.draw_label)
            object.data.draw_label.visible = true;
    } else {
        object.data.draw_object.visible = false;
        if (object.data.draw_label)
            object.data.draw_label.visible = false;
    }
}

function check_all() {
    for (let node of graph.nodes)
        check_visible(node);
    for (let edge of graph.edges)
        check_visible(edge)
}

function check_search(objects, search_item, checked) {
    if (checked) {
        highlight_array.push(search_item);
        if (highlight_array.length == 1) {
            check_all();
            return;
        }
    } else {
        highlight_array.splice(highlight_array.indexOf(search_item), 1);
        if (highlight_array.length == 0) {
            check_all();
            return;
        }
    }
    for (let object of objects)
        check_visible(object);
}

const shortest_path_start = $('#shortest_path-start'),
    shortest_path_end = $('#shortest_path-end'),
    shortest_path = $('#shortest_path'),
    shortest_path_result = $('#shortest_path-result');
shortest_path.click(() => {
    if (check_busy() || check_graph())
        return;
    let id1 = shortest_path_start.val(), id2 = shortest_path_end.val();
    if (!id1.match(/^[0-9]+$/) || !id2.match(/^[0-9]+$/)) {
        noty({ type: 'error', text: "节点序号必须是自然数" });
        return;
    }
    id1 = parseInt(id1);
    id2 = parseInt(id2);
    if (id1 >= graph.nodes.length || id2 >= graph.nodes.length) {
        noty({ type: 'error', text: `节点序号的有效范围是 0 ~ ${graph.nodes.length - 1}` });
        return;
    }
    NProgress.start();
    busy = true;
    processor.query_shortest_path(id1, id2, (err, res) => {
        if (err) {
            noty({type: 'error', text: err.message});
            NProgress.done();
            busy = false;
            return;
        }
        let search_id = Math.random().toString().slice(2), path_length = 0;
        if (res.length) {
            for (let i = 1; i < res[0].length; ++i)
                path_length += graph.nodes[res[0][i - 1]].edges[res[0][i]].data.weight;
        }
        shortest_path_result.append(
            `<span class="mdl-chip mdl-chip--deletable" id="shortest-${search_id}-chip-div">
                <span class="mdl-chip__text">${res.length == 0 ? `不存在 ${id1} 到 ${id2} 的路径` : `${id1} 到 ${id2} 的路径长度为 ${path_length}`}</span>
                <button type="button" class="mdl-chip__action" id="shortest-${search_id}-chip"><i class="material-icons">cancel</i></button>
            </span>`);
        if (res.length) {
            shortest_path_result.append(
                `<table class="mdl-data-table mdl-data-table--selectable" id="shortest-${search_id}-table">
                    <thead><tr>
                        <th>
                            <label class="mdl-checkbox mdl-js-checkbox mdl-data-table__select">
                                <input type="checkbox" class="mdl-checkbox__input" id="shortest-${search_id}-header">
                            </label>
                        </th>
                        <th>序号</th>
                        <th>节点个数</th>
                    </tr></thead>
                    <tbody id="shortest-${search_id}-body"></tbody>
                </table>`);
        }
        const search_chip = $(`#shortest-${search_id}-chip`),
            search_chip_div = $(`#shortest-${search_id}-chip-div`),
            search_table = $(`#shortest-${search_id}-table`),
            search_header = $(`#shortest-${search_id}-header`),
            search_body = $(`#shortest-${search_id}-body`);
        search_header.change(() => {
            search_body.find(`input[type="checkbox"]${search_header.prop('checked') ? ':not(:checked)' : ':checked'}`).click();
        });
        for (let index = 0; index < res.length; ++index) {
            search_body.append(
                `<tr><td>
                    <label class="mdl-checkbox mdl-js-checkbox mdl-data-table__select" for="shortest-${search_id}-row${index}">
                        <input type="checkbox" id="shortest-${search_id}-row${index}" class="mdl-checkbox__input" checked>
                    </label>
                </td><td>${index + 1}</td><td>${res[index].length}</td></tr>`);
            const objects = [graph.nodes[res[index][0]]];
            for (let i = 1; i < res[index].length; ++i) {
                objects.push(graph.nodes[res[index][i]]);
                objects.push(graph.nodes[res[index][i - 1]].edges[res[index][i]]);
            }
            for (let i of objects)
                i.data.visible[`shortest-${search_id}-row${index}`] = true;
            $(`#shortest-${search_id}-row${index}`).change(function () {
                check_search(objects, `shortest-${search_id}-row${index}`, $(this).prop('checked'));
            }).change();
        }
        search_chip.click(() => {
            search_chip_div.remove();
            search_table.remove();
            for (let i = 0; i < res.length; ++i) {
                let index = highlight_array.indexOf(`shortest-${search_id}-row${i}`);
                if (index >= 0)
                    highlight_array.splice(index, 1);
            }
            check_all();
        });
        NProgress.done();
        busy = false;
    });
});

const spanning_tree = $('#minimum-spanning-tree');
spanning_tree.change(() => {
    if (check_busy() || check_graph())
        return;
    if (!graph.data.spanning_tree) {
        NProgress.start();
        busy = true;
        processor.query_minimum_spanning_tree((err, res) => {
            if (err) {
                noty({type: 'error', text: err.message});
                NProgress.done();
                busy = false;
                return;
            }
            graph.data.spanning_tree = res;
            graph.data.spanning_tree_objects = graph.nodes.slice();
            for (let i = 0; i < res.length; ++i)
                if (i != res[i])
                    graph.data.spanning_tree_objects.push(graph.nodes[i].edges[res[i]]);
            for (let i of graph.data.spanning_tree_objects)
                i.data.visible["spanning-tree"] = true;
            check_search(graph.data.spanning_tree_objects, "spanning-tree", spanning_tree.prop('checked'));
            NProgress.done();
            busy = false;
        });
    } else
        check_search(graph.data.spanning_tree_objects, "spanning-tree", spanning_tree.prop('checked'));
});


function clear_algorithm() {
    shortest_path_result.empty();
    spanning_tree.prop('checked', false).parent().removeClass('is-checked');
}