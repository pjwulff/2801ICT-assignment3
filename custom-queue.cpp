#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
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
	size_t queue_index;
};

struct Graph {
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
};

struct Element {
	size_t vertex;
	double priority;
	double path_length;
	bool operator<(Element const &other) {
		return priority < other.priority;
	}
};

struct Queue {
	Queue(Graph &graph) :
		vertices{graph.vertices}
	{}

	void push(Element &element)
	{
		queue.push_back(element);
		size_t index = queue.size() - 1;
		vertices[element.vertex].queue_index = index;
		sift_up(index);
	}
	Element pop()
	{
		Element result = queue[0];
		swap(0, queue.size() - 1);
		queue.pop_back();
		sift_down(0);
		return result;
	}
	void update(size_t index, double priority, double path_length)
	{
		queue[index].priority = priority;
		queue[index].path_length = path_length;
		sift_up(index);
	}
	bool empty()
	{
		return queue.empty();
	}
private:
	std::vector<Vertex> &vertices;
	std::vector<Element> queue;
	void sift_up(size_t index)
	{
		while ((index > 0) and
		       (queue[index] < queue[parent(index)])) {
			swap(index, parent(index));
			index = parent(index);
		}
	}
	void sift_down(size_t index)
	{
		while (has_children(index)) {
			size_t child_index;
			if (has_right_child(index)) {
				child_index = larger_child(index);
			} else {
				child_index = left_child(index);
			}
			if (queue[child_index] < queue[index]) {
				swap(child_index, index);
				index = child_index;
			} else {
				return;
			}
		}
	}
	size_t left_child(size_t index)
	{
		return index*2 + 1;
	}
	size_t right_child(size_t index)
	{
		return index*2 + 2;
	}
	size_t parent(size_t index)
	{
		return (index - 1) / 2;
	}
	bool has_children(size_t index)
	{
		return left_child(index) < queue.size();
	}
	bool has_right_child(size_t index)
	{
		return right_child(index) < queue.size();
	}
	size_t larger_child(size_t index)
	{
		size_t l = left_child(index);
		size_t r = right_child(index);
		if (queue[l] < queue[r]) {
			return l;
		} else {
			return r;
		}
	}
	void swap(size_t i, size_t j)
	{
		Element temp = queue[i];
		queue[i] = queue[j];
		queue[j] = temp;
		vertices[queue[i].vertex].queue_index = j;
		vertices[queue[j].vertex].queue_index = i;
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
	Vertex initial_vertex = {{}, {}, INFINITY, SIZE_MAX};
	graph.vertices = std::vector<Vertex>(num_vertices, initial_vertex);
	graph.edges.reserve(num_edges);
	for (size_t i = 0; i < num_edges; ++i) {
		bool dup = false;
		size_t from, to;
		double weight;
		file >> from;
		file >> to;
		file >> weight;
		edges[i] = {weight, from, to};
		for (auto e : vertices[to].backwards) {
			if (from == edges[e].from) {
				dup = true;
				break;
			}
		}
		if (dup) {
			continue;
		}
		vertices[from].forwards.push_back(i);
		vertices[to].backwards.push_back(i);
	}
	return graph;
}

void
calculate_heuristic(Graph &graph, size_t destination)
{
	auto &vertices = graph.vertices;
	auto &edges = graph.edges;
	auto queue = Queue(graph);
	std::set<size_t> visited_vertices = {};
	Element initial_element = {destination, 0.0, 0.0};
	queue.push(initial_element);
	vertices[destination].shortest_path = 0.0;
	while (!queue.empty()) {
		auto element = queue.pop();
		auto &vertex = vertices[element.vertex];
		if (visited_vertices.count(element.vertex)) {
			continue;
		}
		double distance = element.path_length;
		visited_vertices.insert(element.vertex);
		for (auto edge_index : vertex.backwards) {
			auto &edge = edges[edge_index];
			if (!visited_vertices.count(edge.from)) {
				auto &prev_vertex = vertices[edge.from];
				double path_length = distance + edge.weight;
				if (prev_vertex.queue_index == SIZE_MAX) {
					Element element = {
						edge.from,
						path_length,
						path_length};
					queue.push(element);
				} else {
					queue.update(
						prev_vertex.queue_index,
						path_length,
						path_length);
				}
			}
		}
	}
}

void
search(Graph &graph, size_t source, size_t destination, size_t k)
{
	auto &vertices = graph.vertices;
	auto &edges = graph.edges;
	auto queue = Queue(graph);
	Element initial_element = {
		source,
		vertices[source].shortest_path,
		0.0};
	queue.push(initial_element);
	while (!queue.empty()) {
		auto element = queue.pop();
		auto &vertex = vertices[element.vertex];
		auto path_length = element.path_length;
		if (element.vertex == destination) {
			std::cout << path_length;
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
			double current_path_length = path_length + edge.weight;
			double heuristic = vertices[edge.to].shortest_path;
			Element element = {
				edge.to,
				current_path_length + heuristic,
				current_path_length};
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
	//std::cout << std::setprecision(10);
	auto start_pre = std::chrono::steady_clock::now();
	calculate_heuristic(graph, destination);
	auto end_pre = std::chrono::steady_clock::now();
/*
	double path_cost = 0.0;
	do {
		std::cout << source << " " << path_cost << " ";
		double weight = 0.0;
		for (auto const &edge_index : graph.vertices[source].forwards) {
			auto &edge = graph.edges[edge_index];
			if (edge.to == graph.vertices[source].next) {
				weight = edge.weight;
				break;
			}
		}
		std::cout << weight << std::endl;
		path_cost += weight;
		source = graph.vertices[source].next;
	} while (source != destination);
	std::cout << source << " " << path_cost << std::endl;
	*/
	auto start_post = std::chrono::steady_clock::now();
	//search(graph, source, destination, k);
	auto end_post = std::chrono::steady_clock::now();
	std::chrono::duration<double> pre_duration = end_pre - start_pre;
	std::chrono::duration<double> post_duration = end_post - start_post;
	std::cout << "Preprocessing time: ";
	std::cout << 1000 * pre_duration.count();
	std::cout << " milliseconds."<< std::endl;
	std::cout << "Searching time: ";
	std::cout << 1000 * post_duration.count();
	std::cout << " milliseconds."<< std::endl;
	std::cout << "Total time: ";
	std::cout << 1000 * (pre_duration.count() + post_duration.count());
	std::cout << " milliseconds."<< std::endl;
	return 0;
}
