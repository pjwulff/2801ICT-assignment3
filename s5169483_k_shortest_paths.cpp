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
	Vertex *from;
	Vertex *to;
};

struct Vertex {
	std::vector<Edge *> forwards;
	std::vector<Edge *> backwards;
	double shortest_path;
};

struct Graph {
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
};

struct QueueElement {
	Vertex *vertex;
	double priority;
	double path_cost;
	Vertex *parent;
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
	graph.vertices = std::vector<Vertex>(num_vertices, {{}, {}, INFINITY});
	graph.edges.reserve(num_edges);
	for (size_t i = 0; i < num_edges; ++i) {
		size_t from, to;
		double weight;
		file >> from;
		file >> to;
		file >> weight;
		edges[i] = {weight, &vertices[from], &vertices[to]};
		vertices[from].forwards.push_back(&edges[i]);
		vertices[to].backwards.push_back(&edges[i]);
	}
	return graph;
}

void
calculate_heuristic(Graph &graph, size_t destination)
{
	std::set<Vertex *> visited_vertices = {};
	std::priority_queue<QueueElement> queue;
	QueueElement initial_element = {&graph.vertices[destination], 0.0, 0.0, nullptr};
	queue.push(initial_element);
	graph.vertices[destination].shortest_path = 0.0;
	while (!queue.empty()) {
		auto element = queue.top();
		auto vertex = element.vertex;
		queue.pop();
		if (visited_vertices.count(vertex)) {
			continue;
		}
		double distance = element.path_cost;
		visited_vertices.insert(vertex);
		for (auto &edge : vertex->backwards) {
			auto parent = vertex;
			if (!visited_vertices.count(edge->from)) {
				vertex = edge->from;
				double path_cost = distance + edge->weight;
				if (path_cost < vertex->shortest_path) {
					vertex->shortest_path = path_cost;
				}
				QueueElement element = {
					vertex,
					vertex->shortest_path,
					vertex->shortest_path,
					parent};
				queue.push(element);
			}
		}
	}
}

void
search(Graph &graph, size_t source, size_t destination, size_t k)
{
	std::priority_queue<QueueElement> queue;
	auto &vertices = graph.vertices;
	QueueElement initial_element = {
		&vertices[source],
		vertices[source].shortest_path,
		0.0};
	queue.push(initial_element);
	while (!queue.empty()) {
		auto element = queue.top();
		auto vertex = element.vertex;
		auto path_cost = element.path_cost;
		queue.pop();
		if (vertex == &vertices[destination]) {
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
		for (auto const &edge : vertex->forwards) {
			double current_path_cost = path_cost + edge->weight;
			QueueElement element = {
				edge->to,
				current_path_cost + edge->to->shortest_path,
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
	double path_cost = 0.0;
	Vertex *vertex = graph.vertices[destination];
	//while (vertex != graph.vertices)
	search(graph, source, destination, k);
	return 0;
}
