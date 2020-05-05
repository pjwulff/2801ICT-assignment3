#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <set>
#include <utility>
#include <vector>

struct Vertex;

struct Edge {
	double weight;
	size_t from;
	size_t to;
};

struct Vertex {
	std::vector<size_t> forwards;
	std::vector<size_t> backwards;
	double shortest_path;
	size_t next;
};

struct Graph {
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
};

struct QueueElement {
	size_t vertex;
	double priority;
	double path_cost;
	bool operator<(QueueElement const &other) const {
		return priority > other.priority;
	}
};

Graph
read_graph_from_file(std::fstream &file)
{
	Graph graph;
	size_t num_vertices;
	size_t num_edges;
	std::vector<Vertex> &vertices = graph.vertices;
	std::vector<Edge> &edges = graph.edges;
	file >> num_vertices;
	file >> num_edges;
	Vertex initial_vertex = {{}, {}, INFINITY, 0};
	graph.vertices = std::vector<Vertex>(num_vertices, initial_vertex);
	graph.edges.reserve(num_edges);
	for (size_t i = 0; i < num_edges; ++i) {
		size_t from, to;
		double weight;
		file >> from;
		file >> to;
		file >> weight;
		edges[i] = {weight, from, to};
		vertices[from].forwards.push_back(i);
		vertices[to].backwards.push_back(i);
	}
	return graph;
}

void
calculate_heuristic(Graph &graph, size_t destination)
{
	std::set<size_t> visited_vertices = {};
	std::priority_queue<QueueElement> queue;
	QueueElement initial_element = {destination, 0.0, 0.0};
	queue.push(initial_element);
	graph.vertices[destination].shortest_path = 0.0;
	auto &vertices = graph.vertices;
	auto &edges = graph.edges;
	while (!queue.empty()) {
		auto element = queue.top();
		auto &vertex = vertices[element.vertex];
		queue.pop();
		if (visited_vertices.count(element.vertex)) {
			continue;
		}
		double distance = element.path_cost;
		visited_vertices.insert(element.vertex);
		for (auto edge_index : vertex.backwards) {
			auto &edge = edges[edge_index];
			if (!visited_vertices.count(edge.from)) {
				auto &prev_vertex = vertices[edge.from];
				double path_cost = distance + edge.weight;
				if (path_cost < prev_vertex.shortest_path) {
					prev_vertex.shortest_path = path_cost;
					prev_vertex.next = element.vertex;
					QueueElement element = {
						edge.from,
						path_cost,
						path_cost};
					queue.push(element);
				}
			}
		}
	}
}

void
search(Graph &graph, size_t source, size_t destination, size_t k)
{
	std::priority_queue<QueueElement> queue;
	auto &vertices = graph.vertices;
	auto &edges = graph.edges;
	QueueElement initial_element = {
		source,
		vertices[source].shortest_path,
		0.0};
	queue.push(initial_element);
	while (!queue.empty()) {
		auto element = queue.top();
		auto &vertex = vertices[element.vertex];
		auto path_cost = element.path_cost;
		queue.pop();
		if (element.vertex == destination) {
			std::cout << path_cost;
			if (k > 1) {
				std::cout << ", ";
				k = k - 1;
				continue;
			} else {
				std::cout << std::endl;
				return;
			}
		}
		for (auto edge_index : vertex.forwards) {
			auto &edge = edges[edge_index];
			double current_path_cost = path_cost + edge.weight;
			double heuristic = vertices[edge.to].shortest_path;
			QueueElement element = {
				edge.to,
				current_path_cost + heuristic,
				current_path_cost};
			queue.push(element);
		}
	}
}

int
main(int argc, char *argv[])
{
	std::fstream input_file;
	std::string filename;
	size_t source, destination, k;
	Graph graph;
	if (argc != 2) {
		std::cerr << "Usage: ";
		std::cerr << argv[0] << " FILENAME" << std::endl;
		return 0;
	}
	filename = argv[1];
	input_file.open(filename);
	if (!input_file) {
		std::cerr << "could not open input file" << std::endl;
	}
	graph = read_graph_from_file(input_file);
	input_file >> source;
	input_file >> destination;
	input_file >> k;
	std::cout << std::setprecision(10);
	calculate_heuristic(graph, destination);
	/*
	double path_cost = 0.0;
	do {
		std::cout << source << " " << path_cost << std::endl;
		for (auto const &edge_index : graph.vertices[source].forwards) {
			auto &edge = graph.edges[edge_index];
			if (edge.to == graph.vertices[source].next) {
				path_cost += edge.weight;
				break;
			}
		}
		source = graph.vertices[source].next;
	} while (source != destination);
	std::cout << source << " " << path_cost << std::endl;
	*/
	search(graph, source, destination, k);
	return 0;
}
