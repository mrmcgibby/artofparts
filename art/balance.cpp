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

#include "box.h"

using namespace std;

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

        //add_box(total, username);
	}
}

#if 0
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
#endif
