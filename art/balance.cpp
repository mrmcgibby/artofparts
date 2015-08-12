#include <cstdio>
#include <vector>
#include <array>
#include <sstream>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>

#include <time.h>

#include "gason.h"

using namespace std;



static const size_t dimensions = 2;

template <typename T>
string attr(string name, T value) {
	ostringstream os;
	os << name << "=\"" << value << "\" ";
	return os.str();
}

template <typename T>
string json_field(string name, T value) {
	ostringstream os;
	os << "\"" << name << "\": " << value;
	return os.str();
}

string json_field(string name, string value) {
	ostringstream os;
	os << "\"" << name << "\":\"" << value << "\"";
	return os.str();
}

string json_field(string name, const char* value) {
	ostringstream os;
	os << "\"" << name << "\":\"" << value << "\"";
	return os.str();
}

class box {
	public:
		typedef array<double, dimensions> point;

		box() :
			m_size(0),
			m_dimension(0)
		{
		}
		
		box(double size, string name) :
			m_size(size),
			m_dimension(0),
			m_name(name)
		{
			int c = rand() % 0xFFFFFF;
			ostringstream os;
			os << "#" << setw(6) << setfill('0') << hex << c;
			m_color = os.str();
		}

		// calculate the box sizes for the current tree
		void map(point new_extent, point new_origin) {
			m_extent = new_extent;
			m_origin = new_origin;
			remap();
		}

		// fix the child areas to match the parent
		void remap() {
			fix_dimensions();
			resize();
			double start = m_origin[m_dimension];
			double ratio = m_extent[m_dimension] / m_size;
			for (size_t i = 0; i < m_children.size(); ++i) {
				box& b = m_children[i];
				
				// use the parent values for other dimensions
				b.m_origin = m_origin;
				b.m_extent = m_extent;

				b.m_origin[m_dimension] = start;
				double w = b.m_size * ratio;
				b.m_extent[m_dimension] = w;

				start += w;

				b.remap();
			}
		}

		void resize() {
			if (m_children.size() == 0) {
				return;
			}

			m_size = 0.0;
			for (size_t i = 0; i < m_children.size(); ++i) {
				m_children[i].resize();
				m_size += m_children[i].m_size;
			}
		}

		// the size of the box and children should be kept up to date
		double size() {
			return m_size;
		}

		void add(box b) {
			m_children.push_back(b);
			m_size += b.m_size;
		}

		void insert(int pos, box b) {
			m_children.insert(m_children.begin()+pos, b);
			m_size += b.m_size;
		}

		template <typename... Args>
		void add(box b, Args... args) {
			add(b);
			add(args...);
		}

		void fix_dimensions() {
			for (int i = 0; i < m_children.size(); ++i) {
				m_children[i].m_dimension = next_dimension();
				m_children[i].fix_dimensions();
			}
		}

		double origin() {
			return m_origin[m_dimension];
		}

		double extent() {
			return m_extent[m_dimension];
		}

		box& child(size_t index) {
			return m_children[index];
		}

		size_t children() {
			return m_children.size();
		}

		int next_dimension(int change = 1) const {
			return (m_dimension + change) % dimensions;
		}

		void clear() {
			m_children.clear();
			m_size = 0;
			m_desired.clear();
            m_name = string();
            m_color = string();
		}

        void split(size_t index) {
			box b[2];
			size_t n = 0;
			for (int i = 0; i < children(); ++i) {
				if (i == index) {
					++n;
				}
				b[n].add(child(i));
			}
			m_children.clear();
			m_size = 0;
			m_desired.clear();
			add(b[0]);
			add(b[1]);
        }
		
		void balance(bool top = false) {
			// get score for doing nothing
			double best_score = score();
			box best_box = *this;
			if (top) cerr << "initial score: " << best_score << endl;
			for (int r = 0; r < min(4ul,m_children.size()-1); ++r) {
				if (top) cerr << "Shuffle: " << r << endl;
				random_shuffle(m_children.begin(), m_children.end());
				
				remap();
				
				for (int p = 0; p < m_children.size()/3; ++p) {
					if (top) cerr << "Testing split: " << p << endl;

					if (m_children.size() < 3) {
						break;
					}
					int i = rand() % (m_children.size()-2) + 1;
				
					box b = *this;
					b.split(i);
					b.map(m_extent, m_origin);
					b.child(0).balance();
					b.child(1).balance();
					
					double s = b.score();
					if (s < best_score) {
						if (top) cerr << "new score: " << s << endl;
						best_score = s;
						best_box = b;
						if (top) best_box.write_svg("out.svg");
					}
				}
			}
			*this = best_box;
		}

		void append(box new_box) {			
			box best_box;
			double best_score = 0;
			bool best = false;
			
			if (m_children.size() == 0) {
				box b = *this;
				clear();
				add(b);
			}

			for (int i = 0; i <= m_children.size(); ++i) {
				box b = *this;
				b.insert(i, new_box);

				b.remap();
				double score = b.score();
				if (!best || score < best_score) {
					best = true;
					best_box = b;
					best_score = score;
				}
			}

			if (m_children.size() > 1) {
				for (int i = 0; i < m_children.size(); ++i) {
					box b = *this;
					b.m_children[i].append(new_box);
	
					b.remap();
					double score = b.score();
					if (!best || score < best_score) {
						best = true;
						best_box = b;
						best_score = score;
					}
				}
			}
							
			*this = best_box;
		}

		box subbox(size_t start, size_t end) const {
			box rval;
			for (size_t i = start; i < end; ++i) {
				rval.add(m_children[i]);
			}
			return rval;
		}

		void add_desired(point desired) {
			m_desired.push_back(desired);
		}

		point center()
		{
			point rval;
			for (int i = 0; i < dimensions; ++i) {
				rval[i] = m_origin[i] + m_extent[i]/2.0;
			}
			return rval;
		}
		
		double distance(point a, point b) {
			double sum = 0.0;
			for (int i = 0; i < dimensions; ++i) {
				double distance = a[i] - b[i];
				sum += distance*distance;
			}
			return sqrt(sum);
		}

		double score() {
			if (m_children.size() == 0) {
                double dist = numeric_limits<double>::max();
				if (m_desired.size() > 0) {
					for (int i = 0; i < m_desired.size(); ++i) {
						dist = min(dist, distance(center(), m_desired[i]));
						if (contains(m_desired[i])) { // prefer boxes where point are actually in the middle
							dist = dist / 2.0;
						}
					}
				} else {
					dist = 0;
				}

				double ratio;
				if (m_extent[0] > m_extent[1]) {
					ratio = m_extent[0] / m_extent[1] - 1.0;
				} else {
					ratio = m_extent[1] / m_extent[0] - 1.0;
				}

				return ratio*10 + dist;
			} else {
				double sum = 0.0;
				for (int i = 0; i < m_children.size(); ++i) {
					sum += m_children[i].score();
				}
				return sum;
			}
		}
		
		bool contains(point p) {
			for (int i = 0; i < dimensions; ++i) {
				if (p[i] < m_origin[i])
					return false;
				if (p[i] > m_origin[i]+m_extent[i])
					return false;
			}
			return true;
		}

		bool contains_desired() {
			if (m_children.size() == 0) {
				for (int i = 0; i < m_desired.size(); ++i) {
					if (contains(m_desired[i])) {
						return true;
					}
				}
				return false;
			}
			for (int i = 0; i < m_children.size(); ++i) {
				if (!m_children[i].contains_desired()) {
					return false;
				}
			}
			return true;
		}

		bool leaf() {
			return m_children.size() == 0;
		}
	
		string rect_svg(point offset = point()) {
			ostringstream os;
			if (m_children.size() == 0) {
				os << "  <rect "; {
					os << attr("x", m_origin[0]+offset[0]);
					os << attr("y", m_origin[1]+offset[1]);
					os << attr("width", m_extent[0]);
					os << attr("height", m_extent[1]);
					os << attr("fill", m_color);
					os << attr("stroke", "black");
				} os << "/>\n";
			} else {
				for (int i = 0; i < m_children.size(); ++i) {
					os << m_children[i].rect_svg(offset);
				}
			}
			return os.str();
		}

		string point_svg(point offset = point()) {
			ostringstream os;
			if (m_children.size() == 0 && m_desired.size() > 0) {
				for (int i = 0; i < m_desired.size(); ++i) {
					//if (contains(m_desired[i])) {
						os << "  <circle ";
						os << attr("cx", m_desired[i][0]+offset[0]);
						os << attr("cy", m_desired[i][1]+offset[1]);
						os << attr("r", contains(m_desired[i]) ? 10 : 3);
						os << attr("fill", m_color);
						os << attr("stroke", "black");
						os << "/>\n";
						//}
						/*
					os << "  <line ";
					os << attr("x1", m_desired[i][0]+offset[0]);
					os << attr("y1", m_desired[i][1]+offset[1]);
					os << attr("x2", m_origin[0]+m_extent[0]/2+offset[0]);
					os << attr("y2", m_origin[1]+m_extent[1]/2+offset[1]);
					os << attr("stroke", "black");
					os << "/>";*/
					
					os << "  <circle ";
					os << attr("cx", m_origin[0]+m_extent[0]/2+offset[0]);
					os << attr("cy", m_origin[1]+m_extent[1]/2+offset[1]);
					os << attr("r", 5);
					os << attr("stroke", "black");
					os << "/>\n";
				}
			} else {
				for (int i = 0; i < m_children.size(); ++i) {
					os << m_children[i].point_svg(offset);
				}
			}
			return os.str();
		}

		string json(int level = 0) {
			ostringstream os;
			if (m_children.size() == 0 && m_name != string()) {
				for (int i = 0; i < level; ++i) {
					os << "  ";
				}
				os << "{ ";
				os << json_field("name", m_name) << ",";
				os << json_field("dimension", m_dimension) << ",";
				os << json_field("value", m_size) << " }";
			} else {
				for (int i = 0; i < level; ++i) {
					os << "  ";
				}
				os << "[\n";
				for (int i = 0; i < m_children.size(); ++i) {
					os << m_children[i].json(level+1);
					if (i < m_children.size()-1) {
						os << ",";
					}
					os << endl;
				}
				for (int i = 0; i < level; ++i) {
					os << "  ";
				}
				os << "]";
			}
			return os.str();
		}
		
		string flat_json() {
			ostringstream os;
			if (m_children.size() == 0 && m_name != string()) {
				os << "{ ";
				os << json_field("name", m_name) << ",";
				os << json_field("x", m_origin[0]) << ",";
				os << json_field("y", m_origin[1]) << ",";
				os << json_field("width", m_extent[0]) << ",";
				os << json_field("height", m_extent[1]) << ",";
				os << json_field("value", m_size) << " }";
			} else {
				for (int i = 0; i < m_children.size(); ++i) {
					os << m_children[i].flat_json();
					if (i < m_children.size()-1) {
						os << ",";
					}
					os << endl;
				}
			}
			return os.str();
		}

		void write_svg(string filename) {
			ofstream svg(filename);
			svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			svg << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
				"\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
			svg << "<svg xmlns=\"http://www.w3.org/2000/svg\">\n";
			svg << rect_svg();
			svg << point_svg();
			svg << "</svg>\n";
		}

		string name() { return m_name; }

	private:
		vector<box> m_children;
		double m_size;
		point m_origin, m_extent;
		size_t m_dimension;
		string m_color;
		string m_name;
		
		vector<point> m_desired;
};

void make_box_impl(box& rval)
{
}

template <typename... Args>
void make_box_impl(box& rval, box b, Args... args)
{
	rval.add(b);
	make_box_impl(rval, args...);
}

template <typename... Args>
box make_box(Args... args)
{
	box rval;
	make_box_impl(rval, args...);
	return rval;
}

vector<box> all;

void add_box_impl(box& b) {
	all.push_back(b);
}

template <typename... Args>
void add_box_impl(box& b, int x, int y, Args... args) {
	box::point d;
	d[0] = x;
	d[1] = y;
	b.add_desired(d);
	add_box_impl(b, args...);
}

template <typename... Args>
void add_box(int size, string name, Args... args) {
	box b(size,name);
	add_box_impl(b, args...);
}

JsonValue json_prop(const JsonValue& value, string name) {
	if (value.getTag() != JSON_OBJECT) {
		return JsonValue();
	}
	for (auto i : value) {
		if (i->key == name) {
			return i->value;
		}
	}
	return JsonValue();
}

void add_from_stdin() {
	string json((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());

	char* source = strdup(json.c_str());
	char* endptr;
	JsonValue value;
	JsonAllocator allocator;
	int status = jsonParse(source, &endptr, &value, allocator);
	if (status != JSON_OK) {
		fprintf(stderr, "%s at %zd\n", jsonStrError(status), endptr - source);
		exit(EXIT_FAILURE);
	}

	JsonValue orders = json_prop(value, "orders");
	for (auto order_iter : orders) {
		JsonValue order = order_iter->value;
		
		JsonValue customer = json_prop(order, "customer");
		string first_name = json_prop(customer, "first_name").toString();
		string last_name = json_prop(customer, "last_name").toString();
		string username = first_name.substr(0,1) + last_name;

		JsonValue line_items = json_prop(order, "line_items");
		double total = 0;
		for (auto item_iter : line_items) {
			JsonValue item = item_iter->value;
			total += atof(json_prop(item, "price").toString());
		}

		cout << total << "  " << username << endl;

		add_box(total, username);
	}
}

int main() {
	add_from_stdin();
	
	//	srand(10);
	srand(time(NULL));

	box::point origin = {0,0}, extent = {1268,425};

	/*
	add_box(60, "joshg");
	add_box(110, "tmerrill",673,83,120,115,863,394);
	add_box(35, "rchrastil",1127,38,332,350,1229,388);
	add_box(260, "hollyl",340,424);
	add_box(60, "sean",334,61,323,370,802,345);
	add_box(60, "dmoore",306,369,1095,32,1006,400);
	add_box(260, "JStein");
	add_box(35, "CherryRed");
	add_box(100, "AlexanderG");
	add_box(60, "DMengel",445,146,302,168,341,365);
	add_box(35, "AubrianneN");
	add_box(260, "AAnderson");
	add_box(510, "RSmith");
	add_box(60, "JudithM",127,184,126,75,463,374);
	add_box(110, "KatieS");
	add_box(135, "JaniceW");
	add_box(110, "CHilton",346,345,650,220,846,364);
	add_box(35, "KellyA",1205,375,782,216,420,58);
	add_box(510, "JShore");
	add_box(260, "SNason");
	add_box(60, "MHackett");
	*/
	
	double	best_score;
	bool	best = false;

	int counter = 0;
	while (true) {
		if (counter % 1000 == 0) {
			cout << "Shuffles: " << counter << endl;
		}
		++counter;

		random_shuffle(all.begin(), all.end());
		
		box test = all[0];
		test.map(extent, origin);
		for (int i = 1; i < all.size(); ++i) {
			test.append(all[i]);
		}
		double score = test.score();
		if (!best || score < best_score) {
			best_score = score;
			best = true;
			cout << "Score: " << best_score << endl;
			test.write_svg("out.svg");

			ofstream flat("flat.json");
			flat << "[" << endl;
			flat << test.flat_json() << endl;
			flat << "]" << endl;
		}
	}

    /*
    all.map(extent, origin);
	all.balance(true);
	all.map(extent, origin);
	*/

	/*
    all = box(35, "lhealy");
    all.map(extent, origin);
    add_box(35, "aneubert");
    add_box(60, "mchackett");
    add_box(60, "garyl");
    add_box(60, "sean");
    add_box(60, "jkelly");
    add_box(60, "joshg");
    add_box(60, "clarkh");
    add_box(260, "shawnN");
    add_box(510, "rsmith");
    add_box(35, "rchrastil", 1127, 38, 332, 350, 1229, 388);
    add_box(60, "dmoore", 306, 369, 1095, 32, 1006, 400);
    add_box(110, "tmerrill", 673, 83, 120, 115, 863, 394);
    add_box(110, "hollyl", 340, 424, 726, 401, 454, 188);
    add_box(150, "chilton", 346, 345, 650, 220, 846, 364);
    add_box(150, "mdtommyd", 332, 372, 891, 54, 1214, 392);
    */

	/*
    all.map(extent, origin);
	all.write_svg("out.svg");

	ofstream json("out.json");
	json << all.json() << endl;

	ofstream flat("flat.json");
	flat << "[" << endl;
	flat << all.flat_json() << endl;
	flat << "]" << endl;
	*/
}
