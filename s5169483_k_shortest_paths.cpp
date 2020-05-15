#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <vector>

struct Vertex;

/* Edges keep a record of both from which vertex they are emanating
 * and to which vertex they are going. This allows us to easily follow
 * edges backwards.
 */
struct Edge {
	double weight;
	size_t from;
	size_t to;
};

/* Similarly, vertices keep a record of both incoming and outgoing edges.
 * In the pre-processing pass we find the absolute shortest path from the
 * destination to every other node. This shortest path length is recorded
 * per vertex and is used as the heuristic in the A* search.
 */
struct Vertex {
	std::vector<size_t> outgoing;
	std::vector<size_t> incoming;
	double shortest_path;
};

/* The graph is stored as a list of vertices and edges, where each vertex also
 * maintains a list of edges, so therefore the graph is essentially an adjacency
 * list.
 */
struct Graph {
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
};

/* A custom structure is used to simplify the queue. Each element in the queue
 * keeps track of which vertex we're currently talking about, the priority,
 * and for the A*-search, the path length so far.
 */
struct QueueElement {
	size_t vertex_index;
	double priority;
	double path_length;
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
	auto &vertices = graph.vertices;
	auto &edges = graph.edges;
	file >> num_vertices;
	file >> num_edges;
	/* Vertices are initialised with a shortest path length of `INFINITY'
	 * in preparation of the Dijkstra's algofirthm about to be performed.
	 * They also have empty `forwards' and `backwards' edges.
	 */
	Vertex initial_vertex = {{}, {}, INFINITY};
	graph.vertices = std::vector<Vertex>(num_vertices, initial_vertex);
	graph.edges.reserve(num_edges);
	/* Loop over all the edges in the file. */
	for (size_t i = 0; i < num_edges; ++i) {
		size_t from, to;
		double weight;
		file >> from;
		file >> to;
		file >> weight;
		edges[i] = {weight, from, to};
		vertices[from].outgoing.push_back(i);
		vertices[to].incoming.push_back(i);
	}
	return graph;
}

/* This preprocessing stage performs Dijkstra's algorithm backwards -- that is,
 * starting at the destination and moving outwards. After this we will have
 * calculated the length of the absolute shortest path from any vertex in the
 * graph to the destination.
 */
void
calculate_heuristic(Graph &graph, size_t destination)
{
	std::set<size_t> visited_vertices;
	std::priority_queue<QueueElement> queue;
	auto &vertices = graph.vertices;
	auto &edges = graph.edges;
	/* Initially the only element in the priority queue is the destination,
	 * as we are working backwards.
	 */
	QueueElement initial_element = {destination, 0.0, 0.0};
	queue.push(initial_element);
	graph.vertices[destination].shortest_path = 0.0;
	while (!queue.empty()) {
		/* Pop the next element off the queue. */
		auto element = queue.top();
		auto &vertex = vertices[element.vertex_index];
		queue.pop();
		/* Have we already calculated the shortest path for this vertex?
		 * If so, skip.
		 */
		if (visited_vertices.count(element.vertex_index)) {
			continue;
		}
		visited_vertices.insert(element.vertex_index);
		double distance = element.path_length;
		/* For every incoming edge to the current vertex. */
		for (auto edge_index : vertex.incoming) {
			auto const &edge = edges[edge_index];
			if (!visited_vertices.count(edge.from)) {
				auto &prev_vertex = vertices[edge.from];
				double path_length = distance + edge.weight;
				if (path_length < prev_vertex.shortest_path) {
					prev_vertex.shortest_path = path_length;
					QueueElement element = {
						edge.from,
						path_length,
						path_length};
					queue.push(element);
				}
			}
		}
	}
}

/* The way we calculate the k-shortest paths is by performing an A*-search,
 * using the shortest path to the destination calculated in the previous
 * function as the heuristic. As this heuristic is not an approximation,
 * but is in fact exact, this is very fast.
 */
void
search(Graph &graph, size_t source, size_t destination, size_t k)
{
	std::priority_queue<QueueElement> queue;
	auto &vertices = graph.vertices;
	auto &edges = graph.edges;
	/* This time the first element in the priority queue is the source.
	 * The heuristic/priority is the shortest path cost we previously
	 * calculated, and the current path length is 0.
	 */
	QueueElement initial_element = {
		source,
		vertices[source].shortest_path,
		0.0};
	queue.push(initial_element);
	while (!queue.empty()) {
		/* Pop the next element off the queue. */
		auto element = queue.top();
		auto &vertex = vertices[element.vertex_index];
		auto path_length = element.path_length;
		queue.pop();
		/* Is the current vertex the destination? Great, we've found
		 * another path.
		 */
		if (element.vertex_index == destination) {
			std::cout << path_length;
			/* If we still have more paths to find, subtract 1
			 * from k and keep going. Otherwise quit early.
			 */
			if (k > 1) {
				std::cout << ", ";
				k = k - 1;
				continue;
			} else {
				std::cout << std::endl;
				return;
			}
		}
		/* For every outgoing edge from the current vertex... */
		for (auto edge_index : vertex.outgoing) {
			auto const &edge = edges[edge_index];
			double current_path_length = path_length + edge.weight;
			double heuristic = vertices[edge.to].shortest_path;
			/* Add to the priority queue. Recall that in an
			 * A*-search the priority is the current cost +
			 * the heuristic for the candidate node.
			 */
			QueueElement element = {
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

	/* Read in the graph from the file with `read_graph_from_file'. */
	auto start_build = std::chrono::steady_clock::now();
	graph = read_graph_from_file(input_file);
	auto end_build = std::chrono::steady_clock::now();

	/* Read in which vertices to use as source and destination, and `k'. */
	input_file >> source;
	input_file >> destination;
	input_file >> k;

	/* Preprocess the graph using backwards Dijkstra's to calculate the
	 * shortest path length from every vertex to the destination. This
	 * will be used as a heuristic in the next phase.
	 */
	auto start_pre = std::chrono::steady_clock::now();
	calculate_heuristic(graph, destination);
	auto end_pre = std::chrono::steady_clock::now();

	/* Search the graph using an A*-search to find paths to the destination
	 * using the heuristics previously calculated.
	 */
	auto start_post = std::chrono::steady_clock::now();
	search(graph, source, destination, k);
	auto end_post = std::chrono::steady_clock::now();

	/* Output timing information to the terminal. */
	std::chrono::duration<double> build_duration = end_build - start_build;
	std::chrono::duration<double> pre_duration = end_pre - start_pre;
	std::chrono::duration<double> post_duration = end_post - start_post;
	std::cout << "Building time: ";
	std::cout << 1000 * build_duration.count();
	std::cout << " milliseconds."<< std::endl;

	std::cout << "Preprocessing time: ";
	std::cout << 1000 * pre_duration.count();
	std::cout << " milliseconds."<< std::endl;

	std::cout << "Searching time: ";
	std::cout << 1000 * post_duration.count();
	std::cout << " milliseconds."<< std::endl;

	std::cout << "Total time: ";
	std::cout << 1000 * (
		pre_duration.count() + post_duration.count() +
		build_duration.count());
	std::cout << " milliseconds."<< std::endl;
	return 0;
}

