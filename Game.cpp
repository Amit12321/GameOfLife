#include "Game.hpp"
#include <functional>

#define NUM_OF_SPECIES 7
static const char *colors[NUM_OF_SPECIES] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {
	vector<vector<int>>& field = *curr;
	if (print_on) { 
		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != NULL)
			cout << "<------------" << header << "------------>" << endl;
		
		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height; ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}

using NeighborForeach = void (int i, int j);
static inline void for_each_neighbor(int height, int width, int i, int j, std::function<NeighborForeach> callback)
{
	for (int k = -1; k <= 1; ++k) {
		for (int m = -1; m <= 1; ++m) {
			if (i + k >= 0 && i + k < height && j + m >= 0 && j + m < width) {
				if (k == 0 && m == 0) {
					continue;
				}
				callback(i+k, j+m);
			}
		}
	}
}
static int is_alive(uint i, uint j, vector<vector<int>>& field) {
	if (i < 0 || j < 0 || i >= field.size() || j >= field[0].size()) {
		return -1;
	}
	return field[i][j] > 0 ? 1 : 0;
}

static bool is_alive_after(uint i, uint j, vector<vector<int>>& field) {
	uint field_height = field.size();
	uint field_width = field[0].size();
	int counter = 0;

	for_each_neighbor(field_height, field_width, i, j, [&] (int m, int n) {
		counter += is_alive(m, n, field);
	});

	int alive = is_alive(i, j, field);
	return (alive == 1 && (counter == 2 || counter == 3)) || (alive == 0 && counter == 3);
}

static int find_dominant_species(uint i, uint j, vector<vector<int>>& field) {
	uint field_height = field.size();
	uint field_width = field[0].size();
	vector<int> presence(NUM_OF_SPECIES + 1);

	for_each_neighbor(field_height, field_width, i, j, [&] (int m, int n) {
		int species = field[m][n];
		presence[species] += species;
	});

	auto dominant = std::max_element(presence.begin(), presence.end());
	return dominant - presence.begin();
}

static int get_new_species(uint i, uint j, vector<vector<int>>& field) {
	if (is_alive(i, j, field) == 0) {
		return 0;
	}
	uint field_height = field.size();
	uint field_width = field[0].size();
	int sum_of_species = field[i][j]; // Start with the species of the current cell
	int alive_count = 1; // 1 is for the cell itself

	for_each_neighbor(field_height, field_width, i, j, [&] (int m, int n) {
		sum_of_species += field[m][n];
		alive_count += is_alive(m, n, field);
	});

	return std::round((double)sum_of_species / alive_count);
}


int calculate_phase1(vector<vector<int>>& field, vector<vector<int>>& next_field, uint start, uint stop) {
	uint field_width = field[0].size();
	for (uint i = start; i <= stop; ++i) {
		for (uint j = 0; j < field_width; ++j) {
			if (is_alive_after(i, j, field)) {
				if (is_alive(i, j, field)) {
					next_field[i][j] = field[i][j];
					continue;
				}
				next_field[i][j] = find_dominant_species(i, j, field);
			}
			else {
				next_field[i][j] = 0;
			}
		}
	}
	return 0;
}

int calculate_phase2(vector<vector<int>>& field, vector<vector<int>>& next_field, uint start, uint stop) {
	uint field_width = field[0].size();
	for (uint i = start; i <= stop; ++i) {
		for (uint j = 0; j < field_width; ++j) {
			(next_field)[i][j] = get_new_species(i, j, field);
		}
	}
	return 0;
}

Game::Game(game_params params) {
	m_gen_num = params.n_gen;
	m_thread_num = params.n_thread;
	filename = params.filename;
	interactive_on = params.interactive_on;
	print_on = params.print_on;
}


uint Game::thread_num() const {
	return m_thread_num > field_height ? field_height : m_thread_num;
}

void Game::run() {
	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(NULL);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {
	// Create threads
	// Create game fields
	// Start the threads
	// Testing of your implementation will presume all threads are started here
	curr = new vector<vector<int>>;
	vector<vector<string>> field;
	vector<string> rows = utils::read_lines(filename);
	for (string& row : rows) {
		vector<int> next;
		vector<string> next_string = utils::split(row, ' ');
		for(string& s : next_string) {
			next.push_back(stoi(s));
		}
		(*curr).push_back(next);
	}
	field_height = (*curr).size();
	field_width = (*curr)[0].size();
	
	next = new vector<vector<int>>(*curr);

	jobs = new PCQueue<Job*>;

	b = new Barrier();

	pthread_mutex_init(&tile_hist_mutex, NULL);
	for (uint i = 0; i < thread_num(); ++i) {
		Thread* t = new LifeThread(i, jobs, b, &m_tile_hist, &tile_hist_mutex);
		m_threadpool.push_back(t);
		t->start();
	}

}

void Game::_prepare_jobs(JOB_TYPE type) {
	int start, stop;
	for (uint i = 0; i < thread_num(); ++i) {
		start = ( field_height / thread_num() ) * i;
		if (i == thread_num() - 1) {
			stop = field_height - 1;
		} else {
			stop = start + (field_height / thread_num()) - 1;
		}
		jobs->push(new Job(type, start, stop, curr, next));
		b->increase();
	} 
	b->wait();
}

void Game::_step(uint curr_gen) {
	// Push jobs to queue
	// Wait for the workers to finish calculating 
	// Swap pointers between current and next field
	_prepare_jobs(PHASE1);
	vector<vector<int>>* temp = curr;
	curr = next;
	next = temp;
	_prepare_jobs(PHASE2);
	temp = curr;
	curr = next;
	next = temp;
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// Testing of your implementation will presume all threads are joined here
	for (uint i = 0; i < thread_num(); ++i) {
		jobs->push(new Job(POISON));
		b->increase();
	}

	b->wait();
	for (Thread* t : m_threadpool) {
		t->join();
		delete t;
	}
	delete b;
	delete jobs;
	delete curr;
	delete next;
}

const vector<double> Game::tile_hist() const {
	return m_tile_hist;
}

const vector<double> Game::gen_hist() const {
	return m_gen_hist;
}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/

