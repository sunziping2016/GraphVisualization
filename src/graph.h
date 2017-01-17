/*!
 * @file    graph.h
 * @brief   这个文件主要包含了`graph_processor`类，是程序后端的核心组件。前端将在程序执行的最一开始实例化这样一个对象，
 *          并借助这个对象实现对图的存取、计算等等的操作。
 *
 *          这个文件的许多部分是需要昕磊实现的。类名、函数成员名、成员变量等等都可以自行决定，甚至在必要的时候可以修改参数和返回值类型（向我说明）。
 *          对于需要昕磊实现的部分，我会注明“\<待实现\>”。对于其他未注明的部分，我会使用[`boost.graph`](http://www.boost.org/doc/libs/1_63_0/libs/graph/doc/index.html)
 *          实现。
 *
 *          <b>设计纲要</b>：请勿在代码里使用中文（注释最好不要含有中文）。请将程序写得尽可能健壮，在遇到不合法的输入应当抛出异常
 *          “`throw std::runtime_error(some_message);`”。尽量避免出现数组和指针等底层数据结构，需要用的时候可以考虑
 *          `std::vector`和`std::unique_ptr`。请尽可能使用STL组件。至于采用的语法，C++14也是可以的。
 * @author  孙子平
 * @date    2017/1/13
 */
#ifndef GRAPHVISUALIZATION_GRAPH_H
#define GRAPHVISUALIZATION_GRAPH_H

#include <iostream>
#include <exception>
#include <vector>
#include <memory>
#include <tuple>
#include <boost/graph/topology.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/multi_array.hpp>

/*!
 * @brief   这个类负责图的存取、处理。
 *
 *          具体最后，这个类可能由两部分实现。一部分是昕磊对于重要算法的实现，一部分是我需要用到的一些额外功能（主要是图的绘制）。
 *          我们会将两部分的代码合并。该类会被默认构造。
 *
 *          <b>图的存取</b>：程序将采用自己的一种文件格式，因而输入输出就有了导入（import）和载入保存（load / save）这两类。
 *          载入保存会将图及计算的中间结果存入文件（如最短路、中心度），而导入只负责载入图。载入和保存的格式可由昕磊自定义。
 *
 *          <b>中间结果</b>：程序应当惰性去求解一些算法，而一旦求解完毕，尽量保存中间结果。
 */
class graph_processor
{
public:
    /*!
     * @brief   默认构造函数。运行结果相当于执行了`clear()`。
     */
    graph_processor();

    struct vertex_3dkk_coordinates_t { typedef boost::vertex_property_tag kind; };
    struct vertex_2dkk_coordinates_t { typedef boost::vertex_property_tag kind; };
    struct vertex_2dfr_coordinates_t { typedef boost::vertex_property_tag kind; };
    /*!
     * @typedef vertex_property_type
     * @brief   节点的属性类，目前暂且留空（我会用其存储节点坐标）。可用来存储坐标。
     */
    typedef boost::property<vertex_3dkk_coordinates_t, boost::cube_topology<>::point_type,
            boost::property<vertex_2dkk_coordinates_t, boost::square_topology<>::point_type,
            boost::property<vertex_2dfr_coordinates_t, boost::square_topology<>::point_type> > > vertex_property_type;
    /*!
     * @typedef edge_property_type
     * @brief   边的属性类。可用来存储边权。
     */
    typedef boost::property<boost::edge_weight_t, double, boost::property<boost::edge_index_t, std::size_t> > edge_property_type;
    /*!
     * @typedef graph_type
     * @brief   支持创建子图的图类型。
     */
    typedef boost::subgraph<boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS,
            vertex_property_type, edge_property_type> > graph_type;

    /*!
     * @brief   <b>\<待实现\></b>从输入流载入图和中间计算结果。
     * @param in 输入流，其格式自定义。
     * @exception std::runtime_error    无法解析。
     */
    void load(std::istream &in);
    /*!
     * @brief   <b>\<待实现\></b>保存图和中间计算结果至输出流。
     * @param out 输出流，其格式自定义。
     */
    void save(std::ostream &out) const;
    /*!
     * @brief   <b>\<待实现\></b>清空所有图的数据和计算的中间结果。
     */
    void clear();
    /*!
     * @brief   <b>\<待实现\></b>导入边列表。
     *
     *          调用之前应当执行`clear()`。
     * @param in    输入流，其格式与样例相同。第1行为`n m`，n是顶点数（顶点id从0到n-1），m是边数。第2到m+1为，两个关联节点id和边权。
     * @exception std::runtime_error    无法解析。
     */
    void import_edge_list(std::istream &in);
    /*!
     * @brief   获取节点属性。
     * @param id    节点id。
     * @return  节点属性。
     * @exception std::runtime_error    无法解析。
     */
    vertex_property_type get_vertex_property(std::size_t id) const;
    /*!
     * @brief   获取节点属性。
     * @param id1    第1个关联节点（id较小）。
     * @param id2    第2个关联节点（id较大）。
     * @return  节点属性。
     * @exception std::runtime_error    id不存在。
     */
    edge_property_type get_edge_property(std::size_t id1, std::size_t id2) const;
    /*!
     * @brief   获取节点个数。
     * @return  节点个数。
     */
    std::size_t get_vertex_num() const;
    /*!
     * @brief   获取边列表。
     * @return  返回一个数组，每个元素代表边，由与边关联的节点id组成（较小在前，较大在后）。
     */
    std::vector<std::tuple<std::size_t, std::size_t, double> > get_edge_list() const;
    /*!
     * @brief   <b>\<待实现\></b>获取两点之间的最短路。
     * @param id1 起始节点。
     * @param id2 结束节点。
     * @return  返回一个数组，数组的每个元素都是1条路径。每条路径包含收尾节点。
     * @exception std::runtime_error    id不存在。
     */
    std::vector<std::vector<std::size_t> > query_shortest_path(std::size_t id1, std::size_t id2);
    /*!
     * @brief   <b>\<待实现\></b>获取最小生成树。
     * @return 返回一个长度等于节点个数的数组，每个元素代表前驱。
     */
    std::vector<std::size_t> query_minimum_spanning_tree();
    /*!
     * @brief   <b>\<待实现\></b>获取介数中心度。
     * @return  返回一个长度等于节点个数的数组，数组的元素是每个节点的介数中心度。
     */
    std::vector<std::size_t> query_betweeness_centrality();
    /*!
     * @brief   <b>\<待实现\></b>获取紧密中心度。
     * @return  返回一个长度等于节点个数的数组，数组的元素是每个节点的紧密中心度。
     */
    std::vector<double> query_closeness_centrality();
    /*!
     * @brief   <b>\<待实现\></b>获取连通分量。
     * @return  返回一个长度等于连通分量个数的数组，数组的元素是每个连通分量的节点id，数组按照连通分量大小排序。
     */
    std::vector<std::vector<std::size_t> > query_connected_components();

    /*!
     * @brief   获取2D Kamada Kawai算法得到的坐标。
     * @param reinitial 是否重新初始化为随机球状分布。
     * @param recalculate   是否重新计算。
     * @param tolerance 能量容差最大阙值。
     * @return  返回一个长度等于节点个数的数组，数组的元素是每个节点的三维坐标。
     */
    std::vector<boost::cube_topology<>::point_type> layout_3dkk(bool reinitial = false, bool recalculate = false, double tolerance = 1e-3);
    /*!
     * @brief   获取2D Kamada Kawai算法得到的坐标。
     * @param reinitial 是否重新初始化为环状分布。
     * @param recalculate   是否重新计算。
     * @param tolerance 能量容差最大阙值。
     * @return  返回一个长度等于节点个数的数组，数组的元素是每个节点的二维坐标。
     */
    std::vector<boost::square_topology<>::point_type> layout_2dkk(bool reinitial = false, bool recalculate = false, double tolerance = 1e-3);
    /*!
     * @brief   获取2D Fruchterman Reingold算法得到的坐标。
     * @param reinitial 是否重新初始化为随机分布。
     * @param recalculate   是否重新计算。
     * @param tolerance 能量容差最大阙值。
     * @return  返回一个长度等于节点个数的数组，数组的元素是每个节点的二维坐标。
     */
    std::vector<boost::square_topology<>::point_type> layout_2dfr(bool reinitial = false, bool recalculate = false, std::size_t iterations = 100);

protected:
    void do_connected_components();
    void do_shortest_path();

    void shortest_path(std::vector<std::vector<std::size_t> > &result, std::vector<std::size_t> &path, std::size_t start, std::size_t end);

private:
    bool has_3dkk, has_2dkk, has_2dfr;
    std::unique_ptr<graph_type> m_graph;
    std::unique_ptr<boost::multi_array<double, 2> > m_distances;
    std::vector<std::unique_ptr<graph_type> > m_subgraphs;
    std::unique_ptr<std::size_t []> m_spanning_tree;
    std::unique_ptr<std::size_t []> m_betweeness_centrality;
    std::unique_ptr<double []> m_closeness_centrality;
};

#endif //GRAPHVISUALIZATION_GRAPH_H
