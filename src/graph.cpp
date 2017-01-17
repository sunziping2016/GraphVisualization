//
// Created by sun on 1/13/17.
//

#include <algorithm>
#include <random>
#include <limits>
#include <cmath>
#include <boost/graph/random_layout.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include "graph.h"

using namespace std;

graph_processor::graph_processor()
        : has_3dkk(false), has_2dkk(false), has_2dfr(false) {}


void graph_processor::clear()
{
    has_3dkk = has_2dkk = has_2dfr = false;
    m_graph.reset();
    m_distances.reset();
    m_subgraphs.clear();
    m_spanning_tree.reset();
    m_betweeness_centrality.reset();
    m_closeness_centrality.reset();
}

void graph_processor::import_edge_list(std::istream &in)
{
    clear();
    size_t n = 0, m = 0;
    in >> n >> m;
    m_graph.reset(new graph_type);
    for (size_t i = 0; i < n; ++i)
        boost::add_vertex(*m_graph);
    for (size_t i = 0; in && i < m; ++i) {
        double weight;
        size_t source, target;
        in >> source >> target >> weight;
        if (source > target)
            swap(source, target);
        boost::add_edge(source, target, edge_property_type(weight), *m_graph);
    }
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    do_connected_components();
}

std::size_t graph_processor::get_vertex_num() const
{
    if (!m_graph)
        throw runtime_error("Empty graph.");
    return boost::num_vertices(*m_graph);
}

std::vector<std::tuple<std::size_t, std::size_t, double> > graph_processor::get_edge_list() const
{
    if (!m_graph)
        throw runtime_error("Empty graph.");
    vector<std::tuple<std::size_t, std::size_t, double> > edges;
    for (pair<graph_type::edge_iterator, graph_type::edge_iterator> iter = boost::edges(*m_graph); iter.first != iter.second; ++iter.first)
        edges.push_back(make_tuple(boost::source(*iter.first, *m_graph), boost::target(*iter.first, *m_graph), boost::get(boost::edge_weight, *m_graph, *iter.first)));
    return edges;
}

graph_processor::vertex_property_type graph_processor::get_vertex_property(std::size_t id) const
{
    if (!m_graph)
        throw runtime_error("Empty graph.");
    return boost::get(boost::vertex_all, *m_graph, id);
}

graph_processor::edge_property_type graph_processor::get_edge_property(std::size_t id1, std::size_t id2) const
{
    if (!m_graph)
        throw runtime_error("Empty graph.");
    pair<graph_type::edge_descriptor, bool> res = boost::edge(id1, id2, *m_graph);
    if (!res.second)
        throw runtime_error("Edge doesn\'t exist.");
    return boost::get(boost::edge_all, *m_graph, res.first);
}

void graph_processor::do_connected_components()
{
    if (m_subgraphs.empty()) {
        std::unique_ptr<std::size_t []> components(new size_t[boost::num_vertices(*m_graph)]);
        size_t num = boost::connected_components(*m_graph, &components[0]);
        m_subgraphs.reserve(num);
        for (size_t i = 0; i < num; ++i)
            m_subgraphs.emplace_back(new graph_type(m_graph->create_subgraph()));
        for (std::pair<graph_type::vertex_iterator, graph_type::vertex_iterator> iter = boost::vertices(*m_graph); iter.first != iter.second; ++iter.first)
            boost::add_vertex(*iter.first, *m_subgraphs[components[*iter.first]]);
        sort(m_subgraphs.begin(), m_subgraphs.end(), [] (const std::unique_ptr<graph_type> &a, const std::unique_ptr<graph_type> &b) {
            return boost::num_vertices(*a) > boost::num_vertices(*b);
        });
    }
}

std::vector<std::vector<std::size_t> > graph_processor::query_connected_components() {
    if (!m_graph)
        throw runtime_error("Empty graph");
    vector<vector<size_t> > result;
    result.reserve(m_subgraphs.size());
    for (const std::unique_ptr<graph_type> &subgraph: m_subgraphs) {
        result.emplace_back();
        result.back().reserve(boost::num_vertices(*subgraph));
        for (std::pair<graph_type::vertex_iterator, graph_type::vertex_iterator> iter = boost::vertices(*subgraph); iter.first != iter.second; ++iter.first)
            result.back().push_back(boost::get(boost::vertex_index, *m_graph, subgraph->local_to_global(*iter.first)));
    }
    return result;
}

std::vector<boost::cube_topology<>::point_type> graph_processor::layout_3dkk(bool reinitial, bool recalculate, double tolerance)
{
    boost::cube_topology<> topology;
    if (!m_graph)
        throw runtime_error("Empty graph");
    if (!has_3dkk || recalculate) {
        const double margin = 0.5;
        double scale = 1.0, startx = 0.0, halfwidth = 0.5;
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> unif(-1.0, 1.0);
        boost::property_map<graph_type, vertex_3dkk_coordinates_t>::type global_position_map(
                boost::get(vertex_3dkk_coordinates_t(), *m_graph));
        if (!has_3dkk || reinitial) {
            for (size_t i = 0; i < boost::num_vertices(*m_graph); ++i) {
                double x1, x2, c1, c2;
                do {
                    x1 = unif(gen);
                    x2 = unif(gen);
                } while ((c1 = x1 * x1 + x2 * x2) >= 1);
                c2 = sqrt(1 - c1);
                global_position_map[i][0] = (2 * x1 * c2) * 100;
                global_position_map[i][1] = (2 * x2 * c2) * 100;
                global_position_map[i][2] = (1 - 2 * c1) * 100;
            }
        }
        for (size_t index = 0; index < m_subgraphs.size(); ++index) {
            graph_type &subgraph = *m_subgraphs[index];
            boost::property_map<graph_type, vertex_3dkk_coordinates_t>::type position_map(boost::get(vertex_3dkk_coordinates_t(), subgraph));
            boost::property_map<graph_type, boost::edge_weight_t>::type weight_map(boost::get(boost::edge_weight, subgraph));
            if (boost::num_vertices(subgraph) > 1)
                boost::kamada_kawai_spring_layout(subgraph, position_map, weight_map, topology, boost::edge_length(1.0), boost::layout_tolerance<double>(tolerance));
            boost::cube_topology<>::point_type min, max, middle;
            for (size_t i = 0; i < boost::num_vertices(subgraph); ++i) {
                boost::cube_topology<>::point_type &pos = position_map[i];
                if (i == 0)
                    min = max = pos;
                else {
                    for (size_t j = 0; j < pos.dimensions; ++j) {
                        if (pos[j] < min[j]) min[j] = pos[j];
                        if (pos[j] > max[j]) max[j] = pos[j];
                    }
                }
            }
            for (size_t i = 0; i < middle.dimensions; ++i)
                middle[i] = (min[i] + max[i]) / 2;
            if (index == 0) {
                scale = 2.0 / boost::cube_topology<>().distance(min, max);
                if (!isnormal(scale))
                    scale = 1.0;
                halfwidth = (middle[0] - min[0]) * scale;
            } else {
                startx += halfwidth + margin;
                halfwidth = (middle[0] - min[0]) * scale;
                startx += halfwidth;
            }
            for (size_t i = 0; i < boost::num_vertices(subgraph); ++i) {
                boost::cube_topology<>::point_type &pos = position_map[i];
                pos[0] = startx + (pos[0] - middle[0]) * scale;
                pos[1] = (pos[1] - middle[1]) * scale;
                pos[2] = (pos[2] - middle[2]) * scale;
            }
        }
        has_3dkk = true;
    }
    vector<boost::cube_topology<>::point_type> result;
    for (std::pair<graph_type::vertex_iterator, graph_type::vertex_iterator> iter = boost::vertices(*m_graph);
         iter.first != iter.second; ++iter.first)
        result.push_back(boost::get(vertex_3dkk_coordinates_t(), *m_graph, *iter.first));
    return result;
}

std::vector<boost::square_topology<>::point_type> graph_processor::layout_2dkk(bool reinitial, bool recalculate, double tolerance) {
    boost::square_topology<> topology;
    if (!m_graph)
        throw runtime_error("Empty graph");
    if (!has_2dkk || recalculate) {
        const double margin = 0.5;
        double scale = 1.0, startx = 0.0, halfwidth = 0.5;
        for (size_t index = 0; index < m_subgraphs.size(); ++index) {
            graph_type &subgraph = *m_subgraphs[index];
            boost::property_map<graph_type, vertex_2dkk_coordinates_t>::type position_map(boost::get(vertex_2dkk_coordinates_t(), subgraph));
            boost::property_map<graph_type, boost::edge_weight_t>::type weight_map(boost::get(boost::edge_weight, subgraph));
            if (!has_2dkk || reinitial)
                boost::circle_graph_layout(subgraph, position_map, 100);
            if (boost::num_vertices(subgraph) > 1)
                boost::kamada_kawai_spring_layout(subgraph, position_map, weight_map, topology, boost::edge_length(1.0), boost::layout_tolerance<double>(tolerance));
            boost::square_topology<>::point_type min, max, middle;
            for (size_t i = 0; i < min.dimensions; ++i)
                min[i] = max[i] = 0;
            for (size_t i = 0; i < boost::num_vertices(subgraph); ++i) {
                boost::square_topology<>::point_type &pos = position_map[i];
                if (i == 0)
                    min = max = pos;
                else {
                    for (size_t j = 0; j < pos.dimensions; ++j) {
                        if (pos[j] < min[j]) min[j] = pos[j];
                        if (pos[j] > max[j]) max[j] = pos[j];
                    }
                }
            }
            for (size_t i = 0; i < middle.dimensions; ++i)
                middle[i] = (min[i] + max[i]) / 2;
            if (index == 0) {
                scale = 2.0 / boost::square_topology<>().distance(min, max);
                if (!isnormal(scale))
                    scale = 1.0;
                halfwidth = (middle[0] - min[0]) * scale;
            } else {
                startx += halfwidth + margin;
                halfwidth = (middle[0] - min[0]) * scale;
                startx += halfwidth;
            }
            for (size_t i = 0; i < boost::num_vertices(subgraph); ++i) {
                boost::square_topology<>::point_type &pos = position_map[i];
                pos[0] = startx + (pos[0] - middle[0]) * scale;
                pos[1] = (pos[1] - middle[1]) * scale;
            }
        }
        has_2dkk = true;
    }
    vector<boost::square_topology<>::point_type> result;
    for (std::pair<graph_type::vertex_iterator, graph_type::vertex_iterator> iter = boost::vertices(*m_graph);
         iter.first != iter.second; ++iter.first)
        result.push_back(boost::get(vertex_2dkk_coordinates_t(), *m_graph, *iter.first));
    return result;
}

std::vector<boost::square_topology<>::point_type> graph_processor::layout_2dfr(bool reinitial, bool recalculate, std::size_t iterations) {
    boost::square_topology<> topology;
    if (!m_graph)
        throw runtime_error("Empty graph");
    boost::property_map<graph_type, vertex_2dfr_coordinates_t>::type position_map(boost::get(vertex_2dfr_coordinates_t(), *m_graph));
    if (!has_2dfr || reinitial)
        boost::random_graph_layout(*m_graph, position_map, topology);
    if (!has_2dfr || recalculate) {
        boost::fruchterman_reingold_force_directed_layout(*m_graph, position_map, topology,
                                                          cooling(boost::linear_cooling<double>(iterations)));
        has_2dfr = true;
    }
    vector<boost::square_topology<>::point_type> result;
    for (std::pair<graph_type::vertex_iterator, graph_type::vertex_iterator> iter = boost::vertices(*m_graph);
         iter.first != iter.second; ++iter.first)
        result.push_back(position_map[*iter.first]);
    return result;
}

std::vector<std::vector<std::size_t> > graph_processor::query_shortest_path(std::size_t id1, std::size_t id2)
{
    if (!m_graph)
        throw runtime_error("Empty graph");
    do_shortest_path();
    vector<vector<size_t> > result;
    if ((*m_distances)[id1][id2] != numeric_limits<double>::max()) {
        vector<size_t> path;
        shortest_path(result, path, id1, id2);
    }
    return result;
}

void graph_processor::do_shortest_path()
{
    if (!m_distances) {
        graph_type::vertices_size_type n = boost::num_vertices(*m_graph);
        m_distances.reset(new boost::multi_array<double, 2>(boost::extents[n][n]));
        boost::floyd_warshall_all_pairs_shortest_paths(*m_graph, *m_distances);
    }
}

void graph_processor::shortest_path(std::vector<std::vector<std::size_t> > &result, std::vector<std::size_t> &path,
                                    std::size_t start, std::size_t end)
{
    path.push_back(start);
    if (start == end)
        result.push_back(path);
    else {
        for (pair<graph_type::adjacency_iterator, graph_type::adjacency_iterator> iter = boost::adjacent_vertices(start, *m_graph); iter.first != iter.second; ++iter.first)
            if (fabs((*m_distances)[start][end] - (*m_distances)[start][*iter.first] -
                     (*m_distances)[*iter.first][end]) < numeric_limits<double>::epsilon())
                shortest_path(result, path, *iter.first, end);
    }
    path.pop_back();
}

std::vector<std::size_t> graph_processor::query_minimum_spanning_tree()
{
    if (!m_graph)
        throw runtime_error("Empty graph");
    if (!m_spanning_tree) {
        m_spanning_tree.reset(new size_t[boost::num_vertices(*m_graph)]);
        for (size_t index = 0; index < m_subgraphs.size(); ++index) {
            graph_type &subgraph = *m_subgraphs[index];
            unique_ptr<std::size_t[]> temp_spanning_tree(new size_t[boost::num_vertices(subgraph)]);
            boost::prim_minimum_spanning_tree(subgraph, temp_spanning_tree.get());
            for (size_t i = 0; i < boost::num_vertices(subgraph); ++i)
                m_spanning_tree[subgraph.local_to_global(i)] = subgraph.local_to_global(temp_spanning_tree[i]);
        }
    }
    return std::vector<std::size_t>(m_spanning_tree.get(), m_spanning_tree.get() + boost::num_vertices(*m_graph));
}

std::vector<std::size_t> graph_processor::query_betweeness_centrality()
{
    if (!m_graph)
        throw runtime_error("Empty graph");
    do_shortest_path();
    size_t n = boost::num_vertices(*m_graph);
    if (!m_betweeness_centrality) {
        m_betweeness_centrality.reset(new size_t[n]);
        for (size_t i = 0; i < n; ++i)
            m_betweeness_centrality[i] = 0;
        for (size_t k = 0; k < n; ++k)
            for (size_t i = 0; i < n; ++i)
                if (i != k && (*m_distances)[i][k] != numeric_limits<double>::max())
                    for (size_t j = i + 1; j < n; ++j)
                        if (j != k && (*m_distances)[k][j] != numeric_limits<double>::max() && (*m_distances)[i][j] != numeric_limits<double>::max()
                            && fabs((*m_distances)[i][j] - (*m_distances)[i][k] - (*m_distances)[k][j]) < numeric_limits<double>::epsilon())
                            ++m_betweeness_centrality[k];
    }
    return vector<size_t>(m_betweeness_centrality.get(), m_betweeness_centrality.get() + n);
}

std::vector<double> graph_processor::query_closeness_centrality() {
    if (!m_graph)
        throw runtime_error("Empty graph");
    do_shortest_path();
    size_t n = boost::num_vertices(*m_graph);
    if (!m_closeness_centrality) {
        m_closeness_centrality.reset(new double[n]);
        for (size_t i = 0; i < n; ++i)
            m_closeness_centrality[i] = 0.0;
        for (size_t i = 0; i < n; ++i)
            for (size_t j = i + 1; j < n; ++j)
                if ((*m_distances)[i][j] != numeric_limits<double>::max()) {
                    m_closeness_centrality[i] += (*m_distances)[i][j];
                    m_closeness_centrality[j] += (*m_distances)[i][j];
                }
    }
    return vector<double>(m_closeness_centrality.get(), m_closeness_centrality.get() + n);
}

void graph_processor::load(std::istream &in)
{
    bool has;
    clear();
    size_t n, m;
    in >> n >> m;
    m_graph.reset(new graph_type);
    for (size_t i = 0; i < n; ++i)
        boost::add_vertex(*m_graph);
    for (size_t i = 0; in && i < m; ++i) {
        double weight;
        size_t source, target;
        in >> source >> target >> weight;
        boost::add_edge(source, target, edge_property_type(weight), *m_graph);
    }
    in >> has_3dkk >> has_2dkk >> has_2dfr;
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    if (has_3dkk || has_2dkk || has_2dfr) {
        for (size_t i = 0; i < n; ++i) {
            if (has_3dkk) {
                boost::cube_topology<>::point_type &pos = boost::get(vertex_3dkk_coordinates_t(), *m_graph)[i];
                in >> pos[0] >> pos[1] >> pos[2];
            }
            if (has_2dkk) {
                boost::square_topology<>::point_type &pos = boost::get(vertex_2dkk_coordinates_t(), *m_graph)[i];
                in >> pos[0] >> pos[1];
            }
            if (has_2dfr) {
                boost::square_topology<>::point_type &pos = boost::get(vertex_2dkk_coordinates_t(), *m_graph)[i];
                in >> pos[0] >> pos[1];
            }
        }
    }
    in >> has;
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    if (has) {
        m_distances.reset(new boost::multi_array<double, 2>(boost::extents[n][n]));
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                in >> (*m_distances)[i][j];
                if ((*m_distances)[i][j] < 0)
                    (*m_distances)[i][j] = numeric_limits<double>::max();
            }
        }
    }
    size_t num, num_v, v;
    in >> num;
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    m_subgraphs.reserve(num);
    for (size_t i = 0; i < num; ++i) {
        m_subgraphs.emplace_back(new graph_type(m_graph->create_subgraph()));
        in >> num_v;
        for (size_t j = 0; j < num_v; ++j) {
            in >> v;
            boost::add_vertex(v, *m_subgraphs.back());
        }
    }
    in >> has;
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    if (has) {
        m_spanning_tree.reset(new size_t[n]);
        for (size_t i = 0; i < n; ++i)
            in >> m_spanning_tree[i];
    }
    in >> has;
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    if (has) {
        m_betweeness_centrality.reset(new size_t[n]);
        for (size_t i = 0; i < n; ++i)
            in >> m_betweeness_centrality[i];
    }
    in >> has;
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    if (has) {
        m_closeness_centrality.reset(new double[n]);
        for (size_t i = 0; i < n; ++i)
            in >> m_closeness_centrality[i];
    }
    if (!in) {
        clear();
        throw runtime_error("Error parsing file.");
    }
    do_connected_components();
}

void graph_processor::save(std::ostream &out) const
{
    if (!m_graph)
        throw runtime_error("Empty graph");
    size_t n = boost::num_vertices(*m_graph);
    out << n << " " << boost::num_edges(*m_graph) << endl;
    for (pair<graph_type::edge_iterator, graph_type::edge_iterator> iter = boost::edges(*m_graph);
         iter.first != iter.second; ++iter.first)
        out << boost::source(*iter.first, *m_graph) << " " << boost::target(*iter.first, *m_graph) << " "
            << boost::get(boost::edge_weight, *m_graph, *iter.first) << endl;
    out << has_3dkk << " " << has_2dkk << " " << has_2dfr << endl;
    if (has_3dkk || has_2dkk || has_2dfr) {
        for (size_t i = 0; i < n; ++i) {
            if (has_3dkk) {
                boost::cube_topology<>::point_type &pos = boost::get(vertex_3dkk_coordinates_t(), *m_graph)[i];
                out << pos[0] << " " << pos[1] << " " << pos[2] << endl;
            }
            if (has_2dkk) {
                boost::square_topology<>::point_type &pos = boost::get(vertex_2dkk_coordinates_t(), *m_graph)[i];
                out << pos[0] << " " << pos[1] << endl;
            }
            if (has_2dfr) {
                boost::square_topology<>::point_type &pos = boost::get(vertex_2dkk_coordinates_t(), *m_graph)[i];
                out << pos[0] << " " << pos[1] << endl;
            }
        }
    }
    if (m_distances) {
        out << true << endl;
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if ((*m_distances)[i][j] != numeric_limits<double>::max())
                    out << (*m_distances)[i][j] << " ";
                else
                    out << -1 << " ";
            }
            out << endl;
        }
    } else
        out << false << endl;
    out << m_subgraphs.size() << endl;
    for (size_t index = 0; index < m_subgraphs.size(); ++index) {
        graph_type &subgraph = *m_subgraphs[index];
        out << boost::num_vertices(subgraph) << endl;
        for (size_t i = 0; i < boost::num_vertices(subgraph); ++i)
            out << subgraph.local_to_global(i) << " ";
        out << endl;
    }
    if (m_spanning_tree) {
        out << true << endl;
        for (size_t i = 0; i < n; ++i)
            out << m_spanning_tree[i] << " ";
        out << endl;
    } else
        out << false << endl;
    if (m_betweeness_centrality) {
        out << true << endl;
        for (size_t i = 0; i < n; ++i)
            out << m_betweeness_centrality[i] << " ";
        out << endl;
    } else
        out << false << endl;
    if (m_closeness_centrality) {
        out << true << endl;
        for (size_t i = 0; i < n; ++i)
            out << m_closeness_centrality[i] << " ";
        out << endl;
    } else
        out << false << endl;
}