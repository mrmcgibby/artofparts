#include <cstdio>
#include <vector>
#include <array>
#include <sstream>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <algorithm>

#include "box.h"

using namespace std;

box::box() :
    m_size(0),
    m_dimension(0)
{
}

box::box(double size, string name) :
    m_size(size),
    m_dimension(0),
    m_name(name)
{
}

// calculate the box sizes for the current tree
void box::map(point new_extent, point new_origin) {
    m_extent = new_extent;
    m_origin = new_origin;
    remap();
}

// fix the child areas to match the parent
void box::remap() {
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

void box::resize() {
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
double box::size() {
    return m_size;
}

void box::add(box b) {
    m_children.push_back(b);
    m_size += b.m_size;
}

void box::insert(int pos, box b) {
    m_children.insert(m_children.begin()+pos, b);
    m_size += b.m_size;
}

void box::fix_dimensions() {
    for (unsigned i = 0; i < m_children.size(); ++i) {
        m_children[i].m_dimension = next_dimension();
        m_children[i].fix_dimensions();
    }
}

double box::origin(int dimension) {
    if (dimension == -1) {
        return m_origin[m_dimension];
    } else {
        return m_origin[dimension];
    }
}

double box::extent(int dimension) {
    if (dimension == -1) {
        return m_extent[m_dimension];
    } else {
        return m_extent[dimension];
    }
}

box& box::child(size_t index) {
    return m_children[index];
}

size_t box::children() {
    return m_children.size();
}

int box::next_dimension(int change) const {
    return (m_dimension + change) % dimensions;
}

void box::clear() {
    m_children.clear();
    m_size = 0;
    m_desired.clear();
    m_name = string();
}

void box::split(size_t index) {
    box b[2];
    size_t n = 0;
    for (unsigned i = 0; i < children(); ++i) {
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

void box::balance(bool top) {
    // get score for doing nothing
    double best_score = score();
    box best_box = *this;
    if (top) cerr << "initial score: " << best_score << endl;
    for (unsigned r = 0; r < min(4ul,m_children.size()-1); ++r) {
        if (top) cerr << "Shuffle: " << r << endl;
        random_shuffle(m_children.begin(), m_children.end());

        remap();

        for (unsigned p = 0; p < m_children.size()/3; ++p) {
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
                //if (top) best_box.write_svg("out.svg");
            }
        }
    }
    *this = best_box;
}

void box::append(box new_box) {
    box best_box;
    double best_score = 0;
    bool best = false;

    if (m_children.size() == 0) {
        box b = *this;
        clear();
        add(b);
    }

    for (unsigned i = 0; i <= m_children.size(); ++i) {
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
        for (unsigned i = 0; i < m_children.size(); ++i) {
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

box box::subbox(size_t start, size_t end) const {
    box rval;
    for (size_t i = start; i < end; ++i) {
        rval.add(m_children[i]);
    }
    return rval;
}

void box::add_desired(point desired) {
    m_desired.push_back(desired);
}

box::point box::center()
{
    point rval;
    for (unsigned i = 0; i < dimensions; ++i) {
        rval[i] = m_origin[i] + m_extent[i]/2.0;
    }
    return rval;
}

double box::distance(point a, point b) {
    double sum = 0.0;
    for (unsigned i = 0; i < dimensions; ++i) {
        double distance = a[i] - b[i];
        sum += distance*distance;
    }
    return sqrt(sum);
}

double box::score() {
    if (m_children.size() == 0) {
        double dist = numeric_limits<double>::max();
        if (m_desired.size() > 0) {
            for (unsigned i = 0; i < m_desired.size(); ++i) {
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
        for (unsigned i = 0; i < m_children.size(); ++i) {
            sum += m_children[i].score();
        }
        return sum;
    }
}

bool box::contains(point p) const {
    for (unsigned i = 0; i < dimensions; ++i) {
        if (p[i] < m_origin[i])
            return false;
        if (p[i] > m_origin[i]+m_extent[i])
            return false;
    }
    return true;
}

box::point box::desired() const
{
    for (unsigned i = 0; i < m_desired.size(); ++i) {
        if (contains(m_desired[i])) {
            return m_desired[i];
        }
    }
    return box::point();
}

bool box::contains_desired() {
    if (m_children.size() == 0) {
        for (unsigned i = 0; i < m_desired.size(); ++i) {
            if (contains(m_desired[i])) {
                return true;
            }
        }
        return false;
    }
    for (unsigned i = 0; i < m_children.size(); ++i) {
        if (!m_children[i].contains_desired()) {
            return false;
        }
    }
    return true;
}

bool box::leaf() {
    return m_children.size() == 0;
}

string box::name() const {
    return m_name;
}

vector<box> box::flat()
{
    vector<box> rval;
    if (leaf() && m_name != string()) {
        rval.push_back(*this);
    } else {
        for (unsigned i = 0; i < m_children.size(); ++i) {
            vector<box> f = m_children[i].flat();
            rval.insert(rval.end(), f.begin(), f.end());
        }
    }
    return rval;
}
